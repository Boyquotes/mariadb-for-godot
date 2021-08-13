#include "register_types.h"

#include "core/object_type_db.h"
#include "mariadb.h"
#include "mariadb_statement.h"


//Functions
//=================================================================================
void register_mariadb_types()
{
    ObjectTypeDB::register_type<MariaDB>();
    ObjectTypeDB::register_type<MariaDBStatement>();
}


void unregister_mariadb_types()
{
    //Not needed for this example
}