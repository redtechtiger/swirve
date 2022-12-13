﻿using System;
using DatabaseHandler;

namespace Swirve_Backend {

    class McClient {

        string name = "";
        int players = 0;
        int tps = 0;
        int status = 0;

    }

    class Backend {

        static void Main(string[] args) {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("[WARN] Running from main isn't supported - Entering dev mode");
            Console.ForegroundColor = ConsoleColor.White;

            Console.WriteLine("[DEBUG] Establishing connection with MySQL Database...");
            var database = new Database();
            database.Init("192.168.0.213","3306", "test","testpass","dev");
            database.Connect();
            Console.ReadLine();
        }
    }
}