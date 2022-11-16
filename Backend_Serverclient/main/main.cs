using System;
using System.Text;
using McClientHandler;

 public class serverclientMain {

     static void Main(string[] args) {

        // Loads all classes, functions & objects from other scripts (see "using * " above)
        Console.WriteLine("Loading scripts...");
        mcClientHandler mcClient = new mcClientHandler();

        // Prepare server for start by assigning all needed values
        Console.WriteLine("Preparing server for start...");
        mcClient.Init(
            _name:"Test",
            _ram:4,
            _javapath:"java",
            _jarfile:@"C:\Users\Jacob\Desktop\Github\Delta-V Userclient\_server\forge-1.16.1-32.0.108.jar"
            );

        // Start the minecraft server
        Console.WriteLine("Starting server...");
        mcClient.Start();

        //Wait for Minecraft server to finish starting
        Console.WriteLine("Waiting for server to go online...");
        while(true) {
            System.Threading.Thread.Sleep(500);
            if(mcClient.Online) break;
        }
        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine("Client Online");
        Console.ForegroundColor = ConsoleColor.White;

        // Infinite loop, listing the different players online
        while(true) {
            Console.WriteLine($"Players online: {mcClient.PlayersOnline}");
            System.Threading.Thread.Sleep(2000);
        }

        // Unreachable code - shuts down the Minecraft client
        Console.WriteLine("Stopping client...");
        mcClient.Stop();
        Console.WriteLine("Client stopped.");
        Console.ReadLine();
        return;
    }
}