using System;
using System.Text;
using McClientHandler;

 public class serverclientMain {

     static void Main(string[] args) {
        // <summary>
        // Loads all classes, functions & objects from other scripts (see "using * " above)
        // </summary>
        Console.WriteLine("Loading scripts...");
        mcClientHandler mcClient = new mcClientHandler();

        Console.WriteLine("Preparing server for start...");
        mcClient.Init("Test", 4, "java", @"C:\Users\Jacob\Desktop\Github\Delta-V Userclient\_server\forge-1.16.1-32.0.108.jar");

        Console.WriteLine("Starting server...");
        mcClient.Start();

        
        mcClient.Stop();


        
        return;
    }
}