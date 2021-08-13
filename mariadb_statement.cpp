/*
MariaDB Database Statement
*/

#include "mariadb_statement.h"


//Functions
//=================================================================================
void MariaDBStatement::_bind_methods()
{
    ObjectTypeDB::bind_method(_MD("execute", "params"), &MariaDBStatement::execute);
    ObjectTypeDB::bind_method(_MD("next_row"), &MariaDBStatement::next_row);
}


bool MariaDBStatement::bind_params(Array &params)
{
    //Make sure the number of params is correct
    if(params.size() < mysql_stmt_param_count(stmt))
    {
        return false;
    }
    
    //Prepare each param
    Variant param;
    
    for(int i = 0; i < params.size(); i++)
    {
        //Fetch next param
        param = params[i];
        
        //Choose type
        switch(param.get_type())
        {
            //Integer?
        case Variant::INT:
        {
            //Bind integer param
            this->params[i].buffer_type = MYSQL_TYPE_LONG;
            this->params[i].buffer = memalloc(sizeof(int));
            this->params[i].buffer_length = sizeof(int);
            
            if(!this->params[i].buffer)
            {
                return false;
            }
            
            *((int *)this->params[i].buffer) = (int)param;
            break;
        }
        
            //Float?
        case Variant::REAL:
        {
            //Bind float param
            this->params[i].buffer_type = MYSQL_TYPE_DOUBLE;
            this->params[i].buffer = memalloc(sizeof(double));
            this->params[i].buffer_length = sizeof(double);
            
            if(!this->params[i].buffer)
            {
                return false;
            }
            
            *((double *)this->params[i].buffer) = (double)param;
            break;
        }
        
            //Null?
        case Variant::NIL:
        {
            //Bind null param
            this->params[i].buffer_type = MYSQL_TYPE_NULL;
            this->params[i].buffer = nullptr;
            this->params[i].buffer_length = 0;
            break;
        }
        
            //String?
        case Variant::STRING:
        {
            //Bind string param
            CharString str = ((String)param).utf8();
            this->params[i].buffer_type = MYSQL_TYPE_STRING;
            this->params[i].buffer = memalloc(sizeof(char) * str.length());
            this->params[i].buffer_length = sizeof(char) * str.length();
            
            if(!this->params[i].buffer)
            {
                return false;
            }
            
            strcpy((char *)this->params[i].buffer, str.get_data());
            break;
        }
        
            //Unknown?
        default:
            return false;
        }
    }
    
    //Bind all params
    return mysql_stmt_bind_param(stmt, this->params) == 0;
}


void MariaDBStatement::unbind_params()
{
    //Free existing param buffers
    for(int i = 0; i < mysql_stmt_param_count(stmt); i++)
    {
        if(params[i].buffer)
        {
            memfree(params[i].buffer);
            params[i].buffer = nullptr;
        }
    }
}


bool MariaDBStatement::init_result_buf()
{
    //Populate result types and create result buffers
    MYSQL_FIELD *fields = mysql_fetch_fields(res_metadata);
    
    for(int i = 0; i < mysql_num_fields(res_metadata); i++)
    {
        //Select field type
        switch(fields[i].type)
        {
            //Integer?
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
        {
            results[i].buffer_type = MYSQL_TYPE_LONG;
            results[i].buffer = memalloc(sizeof(long));
            results[i].buffer_length = sizeof(long);
            
            if(!results[i].buffer)
            {
                return false;
            }
            
            break;
        }
        
            //Float?
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        {
            results[i].buffer_type = MYSQL_TYPE_DOUBLE;
            results[i].buffer = memalloc(sizeof(double));
            results[i].buffer_length = sizeof(double);
            
            if(!results[i].buffer)
            {
                return false;
            }
            
            break;
        }
        
            //String?
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_STRING:
        {
            results[i].buffer_type = MYSQL_TYPE_STRING;
            results[i].buffer = memalloc(sizeof(char) * fields[i].length);
            results[i].buffer_length = sizeof(char) * fields[i].length;
            
            if(!results[i].buffer)
            {
                return false;
            }
            break;
        }
        
            //Unknown?
        default:
            return false;
        }
    }
    
    return true;
}


void MariaDBStatement::fini_result_buf()
{
    //Free result buffers
    for(int i = 0; i < mysql_num_fields(res_metadata); i++)
    {
        if(results[i].buffer)
        {
            memfree(results[i].buffer);
        }
    }
}


MariaDBStatement::MariaDBStatement()
{
    stmt = nullptr;
    res_metadata = nullptr;
    params = nullptr;
    results = nullptr;
}


MariaDBStatement::~MariaDBStatement()
{
    //Free the param array if it exists
    if(params)
    {
        unbind_params();
        memfree(params);
    }
    
    //Free the result array if it exists
    if(results)
    {
        fini_result_buf();
        memfree(results);
    }
    
    //Free the result metadata if it exists
    if(res_metadata)
    {
        mysql_free_result(res_metadata);
    }
    
    //Free the compiled statement if it exists
    if(stmt)
    {
        mysql_stmt_close(stmt);
    }
}


bool MariaDBStatement::compile(MYSQL *db, const char *sql)
{
    //Initialize statement
    stmt = mysql_stmt_init(db);
    
    if(!stmt)
    {
        return false;
    }
    
    //Prepare statement
    if(mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        return false;
    }
    
    //Fetch result metadata
    res_metadata = mysql_stmt_result_metadata(stmt);
    
    //Allocate param array if needed
    unsigned long param_cnt = mysql_stmt_param_count(stmt);
    
    if(param_cnt)
    {
        unsigned long param_cnt = mysql_stmt_param_count(stmt);
        params = (MYSQL_BIND *)memalloc(sizeof(MYSQL_BIND) * param_cnt);

        if(!params)
        {
            return false;
        }
        
        memset(params, 0, sizeof(MYSQL_BIND) * param_cnt);
    }
    
    //Allocate result array if needed
    if(res_metadata)
    {
        unsigned long field_cnt = mysql_num_fields(res_metadata);
        results = (MYSQL_BIND *)memalloc(sizeof(MYSQL_BIND) * field_cnt);
        
        if(!results)
        {
            return false;
        }
        
        memset(results, 0, sizeof(MYSQL_BIND) * field_cnt);
        
        if(!init_result_buf())
        {
            return false;
        }
    }

    return true;
}


bool MariaDBStatement::execute(Array params)
{
    //Return if there is no compiled statement
    if(!stmt)
    {
        return false;
    }
    
    //Reset the statement, bind the parameters, bind the results (if needed), and 
    //execute it until the first row of results.
    mysql_stmt_free_result(stmt);
    mysql_stmt_reset(stmt);
    
    if(!bind_params(params))
    {
        return false;
    }
    
    if(results && mysql_stmt_bind_result(stmt, results))
    {
        return false;
    }
    
    if(mysql_stmt_execute(stmt))
    {
        unbind_params();
        return false;
    }
    
    //Store the result
    if(mysql_stmt_store_result(stmt))
    {
        unbind_params();
        return false;
    }
    
    //Unbind params
    unbind_params();
    return true;
}


Variant MariaDBStatement::next_row()
{
    //Return if there is no statement or no result set
    if(!stmt || !results)
    {
        return Variant();
    }
    
    //Fetch next row
    if(mysql_stmt_fetch(stmt))
    {
        return Variant();
    }
    
    //Create result set dictionary
    Dictionary results;
    
    //Populate result set dictionary
    Variant key;
    Variant value;
    
    MYSQL_FIELD *fields = mysql_fetch_fields(res_metadata);
    
    for(int i = 0; i < mysql_num_fields(res_metadata); i++)
    {
        //Fetch the column name
        const char *col_name = fields[i].name;
        
        if(!col_name)
        {
            return Variant();
        }
        
        key = Variant(col_name);
        
        //Choose type
        switch(fields[i].type)
        {
            //Integer?
        case MYSQL_TYPE_LONG:
        {
            //Build int result value
            value = Variant(*(int *)this->results[i].buffer);
            break;
        }
        
            //Float?
        case MYSQL_TYPE_DOUBLE:
        {
            //Build float result value
            value = Variant(*(double *)this->results[i].buffer);
            break;
        }
        
            //Blob?
        case MYSQL_TYPE_BLOB:
        {
            //Build blob result value
            value = Variant(); //not implemented yet!
            break;
        }
        
            //Null?
        case MYSQL_TYPE_NULL:
        {
            //Build null result value
            value = Variant();
            break;
        }
        
            //Text or unknown?
        case MYSQL_TYPE_STRING:
        default:
        {
            //Build text result value
            value = Variant((const char *)this->results[i].buffer);
            break;
        }
        }
        
        //Add value to result set dictionary
        results[key] = value;
    }
    
    //Return result set dictionary
    return Variant(results);
}