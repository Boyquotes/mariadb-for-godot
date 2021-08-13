#ifndef MARIADB_H
#define MARIADB_H

/*
MariaDB Database Module
*/

#include "core/reference.h"
#include "core/variant.h"

#include <mariadb/mysql.h>


//Classes
//=================================================================================
class MariaDB : public Reference
{
    OBJ_TYPE(MariaDB, Reference);
    
    MYSQL *db;
    bool has_stored_err_msg;
    String err_msg;
    
protected:
    static void _bind_methods();
    
public:
    MariaDB();
    ~MariaDB();
    bool connect(String host, String user, String pswd);
    Variant compile_sql(String sql);
    bool execute_sql(String sql);
    String get_last_error();
};

#endif