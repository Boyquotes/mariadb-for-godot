#ifndef MARIADB_STATEMENT_H
#define MARIADB_STATEMENT_H

/*
MariaDB Database Statement
*/

#include "core/reference.h"
#include "core/variant.h"

#include <mariadb/mysql.h>


//Classes
//=================================================================================
class MariaDBStatement : public Reference
{
    OBJ_TYPE(MariaDBStatement, Reference);
    
    MYSQL_STMT *stmt;
    MYSQL_RES *res_metadata;
    MYSQL_BIND *params;
    MYSQL_BIND *results;
    
protected:
    static void _bind_methods();
    bool bind_params(Array &params);
    void unbind_params();
    bool init_result_buf();
    void fini_result_buf();
    
public:
    MariaDBStatement();
    ~MariaDBStatement();
    bool compile(MYSQL *db, const char *sql);
    bool execute(Array params);
    Variant next_row();
};

#endif