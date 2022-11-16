using System;
using MySql.Data.MySqlClient;

namespace DatabaseHandler {
    public class Database {
    
        public bool IsConnected { get; private set; }
        
        private var _connection = MySql.Data.MySqlClient; 
        private var _connectionInfo = @"
            server={undefined};
            userid={undefined};
            password={undefined};
            database={undefined}
            ";
    
        public void Init(string _server = "{undefined}", string _userid = "{undefined}", string _password = "{undefined}", string _database = "{undefined}") {
            IsConnected = false;
            
            _connectionInfo = $@"
            server={_server};
            userid={_userid};
            password={_password};
            database={_database}
            ";
        }

        public void Connect() {
            _connection = new MySql.Data.MySqlClient(_connectionInfo);
            _connection.Open();

            //Check if connection is running properly
            var _statement = "SELECT VERSION()";
            var _command = new MySqlCommand(_statement, _connection);

            var _version = cmd.ExecuteScalar().ToString();
            Console.WriteLine($"[DEBUG] MYSQL SERVER VERSION -> {_version}");
        }
    
    }

}