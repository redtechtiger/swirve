using System;

namespace DatabaseHandler {
    public class Database {
    
        public bool IsConnected { get; private set; }
        
        private MySql.Data.MySqlClient.MySqlConnection _connection; 
        private string _connectionInfo = "" + 
            "server={undefined};" +
            "port={undefined};" +
            "userid={undefined};" +
            "password={undefined};" +
            "database={undefined};";
    
        public void Init(string _server = "{undefined}", string _port = "{undefined}", string _userid = "{undefined}", string _password = "{undefined}", string _database = "{undefined}") {
            IsConnected = false;
            
            _connectionInfo = $@"" + 
            $@"server={_server};" +
            $@"port={_port};" +
            $@"userid={_userid};" +
            $@"password={_password};" +
            $@"database={_database};";

            Console.WriteLine($"{_connectionInfo}");
        }

        public void Connect() {
            _connection = new MySql.Data.MySqlClient.MySqlConnection(_connectionInfo);
            _connection.Open();

            //Check if connection is running properly
            var _statement = "SELECT VERSION()";
            var _command = new MySql.Data.MySqlClient.MySqlCommand(_statement, _connection);

            var _version = _command.ExecuteScalar().ToString();
            Console.WriteLine($"[DEBUG] MYSQL SERVER VERSION -> {_version}");
        }
    
    }

}