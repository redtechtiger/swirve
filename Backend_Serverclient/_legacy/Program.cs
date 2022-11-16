using System;
using System.IO;
using System.Diagnostics;

class upCraft_main {
	static void Main(string[] args) {
		//---------------------------------------------------------
		//Comment out when build is pushed to master!!
		Console.WriteLine("[SRVCLIENT_DEV] =====DEVBUILD====");
		Console.WriteLine("[SRVCLIENT_DEV] DO NOT USE IN PRODUCTION!");
		Console.WriteLine("[SRVCLIENT_DEV] =================");
		//---------------------------------------------------------

		Console.WriteLine("[SRVCLIENT_DEV] Initiating server client...");



		Console.WriteLine("[SRVCLIENT_DEV] Preparing minecraft client...");
		bool _online = false;
		int _memAllocated = 8;
		string _javaExecutable = "java";
		string _workingDirectory = @"C:\Users\Jacob\Desktop\Example_Server";
		string _jarPath = @"C:\Users\Jacob\Desktop\Example_Server\forge-1.16.5-36.2.34.jar";
		string _startArguments = $"-Xms{_memAllocated}G -Xmx{_memAllocated}G -jar {_jarPath}";
		Process mcClient = new Process();
		mcClient.StartInfo.WorkingDirectory = _workingDirectory;
		mcClient.StartInfo.FileName = _javaExecutable;
		mcClient.StartInfo.Arguments = _startArguments;
		mcClient.StartInfo.RedirectStandardOutput = true;
		mcClient.StartInfo.RedirectStandardError = true;
		mcClient.StartInfo.UseShellExecute = false;
		mcClient.Start();


		Console.WriteLine("[SRVCLIENT_DEV] Beginning console output piping...");
		//StreamReader _clientPipe = mcClient.StandardOutput;
		mcClient.BeginOutputReadLine();

        while (!_online) {
			Console.ForegroundColor = ConsoleColor.Yellow;

			mcClient.OutputDataReceived += (sender, args) => {
				Console.WriteLine($"{args.Data}");
			};

			mcClient.ErrorDataReceived += (sender, args) =>
			{
				Console.WriteLine($"{args.Data}");
			};

			Console.ForegroundColor = ConsoleColor.White;
			System.Threading.Thread.Sleep(2000);
        }
		Console.WriteLine("[SRVCLIENT_DEV] SERVER STARTED");
	}
}