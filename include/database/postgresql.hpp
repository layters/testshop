// filename: db_postgresql.hpp
#ifndef DATABASE_POSTGRESQL_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define DATABASE_POSTGRESQL_HPP_NEROSHOP
#define POSTGRESQL_TAG "\033[1;94m[postgresql]:\033[0m "
#define POSTGRESQL_TAG_ERROR "\033[1;94m[postgresql]:\033[0;91m "
#include <iostream>
// neroshop
#include "debug.hpp"
// dokun-ui
#include <string.hpp>
// libpq (C interface to PostgreSQL) - Debian/Ubuntu
////#include <postgresql/libpq-fe.h>
////#include <postgresql/libpq/libpq-fs.h> // for lo-interfaces (large object interfaces)
// libpq (C interface to PostgreSQL) - Arch
////#include <libpq-fe.h>
////#include <postgresql/server/libpq/libpq-fs.h>
// libpq (C interface to PostgreSQL) - custom (requires compiler flag: -I'/usr/include/postgresql/' on Debian/Ubuntu)
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>
#include <memory> // std::unique_ptr 

namespace neroshop {
namespace DB {
class Postgres { // DB is intended for client-server database engine. SQLite does not use a client-server architecture, so unfortunately we cannot solely use SQLite for this as its used for embedding into applications and becomes an integral part of the program
public:
    Postgres();
    Postgres(const std::string& file_url);
    ~Postgres();
    bool connect(const std::string& conninfo); // string can be empty to use all default parameters
    void finish(); // Closes the connection to the server. Also frees memory used by the PGconn object.
    void execute(const std::string& command, ExecStatusType status_type = PGRES_COMMAND_OK); // PGRES_COMMAND_OK returns no data; PGRES_TUPLES_OK returns data (such as a SELECT or SHOW)
    void execute_params(const std::string& command, const std::vector<std::string>& args, ExecStatusType status_type = PGRES_COMMAND_OK);
    void print_database_info(void);
    void create_table(const std::string& table_name); // https://www.postgresql.org/docs/14/sql-createtable.html
    void rename_table(const std::string& table_name, const std::string& new_name);    
    void alter_table(const std::string& table_name, const std::string& action, const std::string& action_arg, std::string extra_arg = ""); // adds a new column to a table (think of column as a variable) // https://www.postgresql.org/docs/current/sql-altertable.html
    void add_column(const std::string& table_name, const std::string& column_names, std::string column_type = "text DEFAULT NULL");
    void insert_into(const std::string& table_name, const std::string& column_names, const std::string& values); // inserts a new data entry into a table (like assigning a value to a variable)
    void update(const std::string& table_name, const std::string& column_name, const std::string& value, std::string condition = "");
    void update_replace(const std::string& table_name, const std::string& column_name, const std::string& old_text, const std::string& new_text, std::string condition = ""); // replaces all occurences of a character or string//void update_replace_regex(); // replaces regex    
    void create_index(const std::string& index_name, const std::string& table_name, const std::string& indexed_columns); // indexes enhance database performance. An index allows the database server to find and retrieve specific rows much faster than it could do without an index.
    void drop_index(const std::string& index_name);
    void truncate(const std::string& table_name); // deletes all rows from a table 
    void delete_from(const std::string& table_name, const std::string& condition);    
    void drop_column(const std::string& table_name, const std::string& column_name);
    void drop_table(const std::string& table_name);
    void drop_database(const std::string& database_name);
    void vacuum(std::string opt_arg = ""); // garbage collector //VACUUM frees up space within each table but does not return the space to the operating system, therefore, the size of the database file would not be reduced// VACUUM FULL; reduces database size by reclaiming storage occupied by dead tuples
    // conversion
    static std::string to_psql_string(const std::string& str);    
    // setters
    // getters
    static int get_lib_version();
    int get_server_version() const;
    PGconn * get_handle() const;
    int get_integer(const std::string& select_stmt) const; // bool get_boolean(); // atoi("") // string to int
    int get_integer_params(const std::string& select_stmt, const std::vector<std::string>& values) const;
    float get_real(const std::string& select_stmt) const;
    float get_real_params(const std::string& select_stmt, const std::vector<std::string>& values) const;
    float get_float(const std::string& select_stmt) const;
    float get_float_params(const std::string& select_stmt, const std::vector<std::string>& values) const;
    double get_double(const std::string& select_stmt) const; // atof (const char* str);
    double get_double_params(const std::string& select_stmt, const std::vector<std::string>& values) const;
    std::string get_text(const std::string& select_stmt) const;
    std::string get_text_params(const std::string& select_stmt, const std::vector<std::string>& values) const;
    unsigned int get_row_count(const std::string& table_name) const; // returns the number of rows in a table
    int get_last_id(const std::string& table_name) const; // returns the id of the latest or most recent record pushed into a table
    int get_connections_count() const;// returns current number of connections in database (or number of entities connected to the database)
    // date and time
    std::string localtimestamp_to_utc(const std::string& localtimestamp) const; // returns both date and time (arg must be timestamptz - with a timezone)
    std::string localtime_to_utc(const std::string& localtime) const; // returns only time (arg must be timetz or timestamptz - with a timezone)
    // singleton
    static DB::Postgres * get_singleton();
    // boolean
    bool table_exists(const std::string& table_name) const;
    bool column_exists(const std::string& table_name, const std::string& column_name) const;// get_integer("SELECT COUNT(column_name) FROM table_name;");
private:
    PGconn * conn;
    static std::unique_ptr<DB::Postgres> db_obj; // singleton obj (to make connection easier for all classes to access and to keep track of a single connection)
};
}
}
// https://zetcode.com/db/postgresqlc/
// http://www.jancarloviray.com/blog/postgres-quick-start-and-best-practices/
// JSONB actually turns PostgreSQL into a NoSQL database :D
// NOTES: I just learned that connections can be opened for the entirety of the client application's life
// edit: some are also saying closing the database as soon as I'm done with it is much better for scalability
// by keeping the connection opened for the entire duration of the client application's life, it will not scale as well because others may want to connect to the database if the max_connections has been reached
// conclusion - so its better to close the database when its not needed until it is
// If I ever plan on keeping the connection opened for long periods of time, then I must consider pooling!
// sources: https://stackoverflow.com/questions/312702/is-it-safe-to-keep-database-connections-open-for-long-time
//          https://stackoverflow.com/questions/18962852/should-a-database-connection-stay-open-all-the-time-or-only-be-opened-when-neede
//          https://stackoverflow.com/questions/4439409/open-close-sqlconnection-or-keep-open
//          https://stackoverflow.com/questions/11517342/when-to-open-close-connection-to-database

// DROP DATABASE neroshoptest;
// CREATE DATABASE neroshoptest;
// ALTER USER sid PASSWORD 'postgres'

// postgresql binaries:
// installer - https://www.enterprisedb.com/downloads/postgres-postgresql-downloads
// binaries (zip archive) - https://www.enterprisedb.com/download-postgresql-binaries

/*
pgbouncer is a connection pooler for PostgreSQL.

Usage:
  pgbouncer [OPTION]... CONFIG_FILE

Options:
  -d, --daemon         run in background (as a daemon)
  -q, --quiet          run quietly
  -R, --reboot         do an online reboot
  -u, --user=USERNAME  assume identity of USERNAME
  -v, --verbose        increase verbosity
  -V, --version        show version, then exit
  -h, --help           show this help, then exit

Report bugs to <https://github.com/pgbouncer/pgbouncer/issues>.
PgBouncer home page: <https://www.pgbouncer.org/>
*/
// pgbouncer -u=postgres -v -d -q /etc/postgresql/14/main/pg_hba.conf
/*
    # postgresql datatypes:
    all - https://www.postgresql.org/docs/14/datatype.html
    numerals - https://www.postgresql.org/docs/14/datatype-numeric.html
    strings  - https://www.postgresql.org/docs/14/datatype-character.html
    datetime - https://www.postgresql.org/docs/14/datatype-datetime.html
    // text, character varying(n) ( varchar(n) ), money, integer, real (float4), double precision (float8), boolean (bool), date, time, json, xml, etc.

    # open "/etc/postgresql/14/main/pg_hba.conf"
    sudo gedit /etc/postgresql/14/main/pg_hba.conf
    # edit the pg_hba.conf file (change authentication methods from "peer" to "trust"):
    "local   all             postgres                                trust"
    # restart the server (for the changes in "pg_hba.conf" to take effect): 
    sudo service postgresql restart
    # Finally you can login without need of a password as shown in the figure:
    psql user=postgres
    # change the password of postgres or remove superuser (within psql shell)
    sudo -u postgres -c "DROP ROLE superuser;" # ALTER USER postgres PASSWORD 'postgres';
    # leave the psql shell
    \q
    # restore value from trust to md5 or peer then restart again
    
    # where postgresql stores database files:
    sudo -u postgres psql -c "show data_directory;"
    nautilus /var/lib/postgresql/14/main

    # to list all databases on your device (localhost):
    psql -l
    
    # to change limits and other settings: 
    gedit '/etc/postgresql/14/main/postgresql.conf'

# steps to secure database
-- encryption
As postgresql clients sends queries in plain-text and data is also sent unencrypted, it is vulnerable to network spoofing.
You can enable SSL by setting the ssl parameter to on in postgresql.conf.


$ sudo -u postgres createuser -s $USER
$ createdb neroshoptest
$ psql -d neroshoptest

OR

sudo -u postgres createuser -s $USER
createdb
psql

# And, then in the psql shell:
CREATE ROLE myuser LOGIN PASSWORD 'mypass';
CREATE DATABASE mydatabase WITH OWNER = myuser;

# login
psql -h localhost -d mydatabase -U myuser -p <port>
psql -h localhost -d neroshoptest -U postgres -p 5432

# if we don't know the port:
SHOW port;

    // sudo service postgresql start
    // sudo service postgresql status
    
    sudo -u postgres -c "DROP ROLE superuser;"
    
    
changing password (within psql shell):    
    ALTER USER postgres PASSWORD 'postgres';
changing password (normally):
    sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'postgres';"

Example usage(s):

    DB db2("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    db2.print_database_info();
    // checking if table exists
    std::cout << db2.table_exists("users") << " (table_exists)" << std::endl;
    // creating table with columns
    db2.create_table("users");
    db2.add_column("users", "name", "text");
    db2.alter_table("users", "ADD", "pw_hash", "text");
    db2.add_column("users", "opt_email"); // will be NULL since datatype is not specified
    // table rows
    std::cout << db2.get_row_count("users") << " (row count)" << std::endl;
    std::cout << db2.get_last_id("users") << " (lastest row)" << std::endl;
    // replacing all occurences of a character or string from column at specific row(s)
    db2.update_replace("users", "pw_hash", "k", "co", "id = 2");
    // replacing all occurences of character or string from ALL column x
    db2.update_replace("users", "pw_hash", "k", "co");
    // deleting a specific row from table
    db2.delete_from("users", "opt_email='null'");
    db2.delete_from("users", "id=1");
    // deleting an entire column from table
    db2.drop_column("users", "pw_hash");
*/
#endif
