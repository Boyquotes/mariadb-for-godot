/*
MariaDB Database Module
*/

#include "mariadb.h"
#include "mariadb_statement.h"


//Functions
//=================================================================================
void MariaDB::_bind_methods()
{
    ObjectTypeDB::bind_method(_MD("connect", "filename"), &MariaDB::connect);
    ObjectTypeDB::bind_method(_MD("compile_sql", "sql"), &MariaDB::compile_sql);
    ObjectTypeDB::bind_method(_MD("execute_sql", "sql"), &MariaDB::execute_sql);
    ObjectTypeDB::bind_method(_MD("get_last_error"), &MariaDB::get_last_error);
}


MariaDB::MariaDB()
{
    db = nullptr;
    has_stored_err_msg = false;
}


MariaDB::~MariaDB()
{
    //Close existing database connection if there is one
    if(db)
    {
        mysql_close(db);
    }
}


bool MariaDB::connect(String host, String user, String pswd)
{
    //Close existing database connection if there is one, otherwise initialize
    if(db)
    {
        mysql_close(db);
    }
    else
    {
        //Initialize database driver
        db = mysql_init(nullptr);
        
        if(!db)
        {
            return false;
        }
    }
    
    //Open a database connection
    return mysql_real_connect(db, host.utf8().get_data(), user.utf8().get_data(), 
        pswd.utf8().get_data(), nullptr, 0, nullptr, 0) != nullptr;
}


Variant MariaDB::compile_sql(String sql)
{
    //Create new statement and compile it
    MariaDBStatement *stmt = memnew(MariaDBStatement());
    
    if(!stmt)
    {
        return Variant();
    }
    
    if(!stmt->compile(db, sql.utf8().get_data()))
    {
        //Preserve last error
        has_stored_err_msg = true;
        err_msg = get_last_error();
        
        //Free statement
        memdelete(stmt);
        return Variant();
    }
    
    return Variant((Object*)stmt);
}


bool MariaDB::execute_sql(String sql)
{
    return mysql_query(db, sql.utf8().get_data()) == 0;
}


String MariaDB::get_last_error()
{
    //Return stored error message or last error message
    if(has_stored_err_msg)
    {
        has_stored_err_msg = false;
        return err_msg;
    }
    else
    {
        return String(mysql_error(db));
    }
}