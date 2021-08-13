# mariadb-for-godot
A Godot module that provides a convenient object-orientated wrapper for MariaDB.


## Important Licensing Note
Since the MariaDB C connector is licensed under the LGPL license (as of the last
time this file was updated), you cannot statically link to the MariaDB C connector
library without making each project that statically links to it LGPL or GPL. As a
result, I highly recommend that you always dynamically link to the MariaDB C
connector library and distribute it alongside your projects that use it per its
licensing terms.


## Instructions
1. clone the Godot repo
2. clone the branch of this repo that matches your Godot version into the "modules" 
folder inside the Godot repo
3. rename the "mariadb-for-godot" folder to "mariadb"
4. obtain the MariaDB C connector library in binary or source form (source form
requires that you built the connector library yourself)
5. update the SCsub file so that the include path points to the folder containing
the "mariadb" headers folder
6. update the SCsub file so that the library path points to the connector library
7. build Godot


## Usage
Example Program:
```gdscript
func test_mariadb():
	#Test MariaDB driver
	print("\nMariaDB Driver Test")
	print("===================")
	print("Starting MariaDB driver...")
	var sql = MariaDB.new()
	
	#Test MariaDB database connection
	print("Connecting to MariaDB database...")
	
	if not sql.connect("localhost", "user", "password"):
		print(sql.get_last_error())
		
	#Try to select database
	print("Selecting database...")
	
	if not sql.execute_sql("USE test;"):
		print(sql.get_last_error())
		
	#Try to drop existing users table
	print("Dropping users table...")
	
	if not sql.execute_sql("DROP TABLE IF EXISTS users;"):
		print(sql.get_last_error())
		
	#Try to create new users table
	print("Creating users table...")
	
	if not sql.execute_sql("CREATE TABLE users(id INTEGER PRIMARY KEY AUTO_INCREMENT, name VARCHAR(32), score INT, hp REAL);"):
		print(sql.get_last_error())
		
	#Try to insert into users table
	print("Inserting records into users table...")
	var stmt = sql.compile_sql("INSERT INTO users(name, score, hp) VALUES (?, ?, ?);")
	print(sql.get_last_error())
	var users = [["Dylan", 10000, 500.8], ["Emmi", 5000, 800.37], ["Fiona", 2500, 568.975]]
	
	for user in users:
		stmt.execute(user)
		
	#Try to select data from the users table
	print("Selecting data from users table...")
	stmt = sql.compile_sql("SELECT * FROM users;")
	
	if not stmt.execute([]):
		print(sql.get_last_error())
		
	var row = stmt.next_row()
	
	while row != null:
		print(row)
		row = stmt.next_row()
		
	#Try an SQL statement that should fail
	stmt = sql.compile_sql("SELECT * FROM log;")
	
	if stmt != null:
		print("The preceding statement should have failed to compile!")
		
	print("Error message is '" + sql.get_last_error() + "'")
```


## Class Reference
### MariaDB
MariaDB driver class (manages exactly one database connection, can have multiple 
instances)

### MariaDBStatement
MariaDB compiled SQL statement class (do not create instances directly)
                  
                  
## Method Reference
### bool MariaDB.connect(String filename)
connect to an sqlite database

### MariaDBStatement MariaDB.compile_sql(String sql)
compile an SQL statement, returns null on failure

### bool MariaDB.execute_sql(String sql)
execute an SQL statement that takes no parameters and returns no results

### String MariaDB.get_last_error()
get the last error message, useful for debugging when a function call failed
                                                 
### bool SQLStatment.execute(Array params)
execute an SQL statement up until the first row of results, "params" should contain 
one value per "?" placeholder code in the SQL statement, pass an empty array if 
there are no placeholder codes

### Dictionary SQLStatement.next_row()
returns a dictionary containing the contents of the current row in the result set 
and executes the statement up until the next row of results