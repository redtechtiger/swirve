using System;
using System.Text;
using System.Linq;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace McClientHandler {
    public class mcClientHandler {

        // Declares all the variables 
        public string Name { get; private set; }
        public int Tps { get; private set; }
        public int PlayersOnline { get; private set; }
        public string[] Players { get; private set; }
        public string[] PlayersWhitelisted { get; private set; }
        public int RamUsing { get; private set; }
        public int RamAllocated { get; private set; }
        public int Status { get; private set; }
        public bool Online { get; private set; }
        public bool Crashed { get; private set; }
        public string ConsoleOutput { get; private set; }
        public string ErrorOutput { get; private set; }


        private string startArgs { get; set; }
        private string shellPath { get; set; }
        private string jarPath { get; set; }
        private string workingDirectory { get; set; }
        private int terminatingClient { get; set; }
        private int internalError { get; set; }
        private int pollingRate { get; set; }

        private StringBuilder outputBuffer = new StringBuilder();

        // Processes that will be running asynchronously
        private Process client = new Process();
        private Task statusClient;

        public void Init(string _name = "{Default}", int _ram = 2, string _javapath = "java", string _jarfile="") {

            // Assign all variables
            this.internalError = 0;
            this.Status = 0;

            if(String.IsNullOrEmpty(_jarfile)) {
                internalError = 1;
                return; // Return if no client file is provided
            }

            // Assign relevant variables
            this.Name = _name;
            this.RamAllocated = _ram;
            this.shellPath = _javapath;
            this.jarPath = _jarfile;
            this.startArgs = $"-Xms{this.RamAllocated}G -Xmx{this.RamAllocated}G -jar \"{this.jarPath}\"";

            // Get working directory from jar path
            string[] _buffer = _jarfile.Split(new char[] {'/','\\'}); // Split jar path into list, with "\" as the separator
            _buffer = _buffer.SkipLast(1).ToArray(); // Get rid of filename
            this.workingDirectory = String.Join(@"\", _buffer);

            // Assign task lambda
            statusClient = new Task(() => pollClient());

            // Assign the rest of the variables     
            this.Tps = 0;
            this.PlayersOnline = 0;
            //this.PlayersWhitelisted = []; // Read config file
            this.RamUsing = 0;
            this.Online = false;
            this.Crashed = false;
            this.ConsoleOutput = null;
            this.ErrorOutput = null;
            this.Status = 1;

        }

        public void Start() {

            // Prepare the minecraft java process for starting
            this.client.StartInfo.WorkingDirectory = this.workingDirectory;
            this.client.StartInfo.FileName = this.shellPath;
            this.client.StartInfo.Arguments = this.startArgs;
            this.client.StartInfo.RedirectStandardOutput = true;
            this.client.StartInfo.RedirectStandardError = true;
            this.client.StartInfo.UseShellExecute = false;

            // Start the Minecraft server along with starting asynchronous output reading
            this.client.Start();
            startListener();

            //Starts process that asynchronously reads Minecaft server output to set client status, players online, etc.
            statusClient.Start();

            this.Status = 2;

        }

        public void Stop() {

            // Request polling task termination
            this.terminatingClient = 1;

            System.Threading.Thread.Sleep(5000);

            if(statusClient.IsCompleted.Equals(false)) {
                this.internalError = 1;
                return;
            }

            // Kill minecraft java client
            this.client.CancelOutputRead();
            this.client.CancelErrorRead();
            this.client.Kill();
        }

        public void Send(string _input) {

        }

        private void startListener() {
            // Starts asynchronous listening
            this.client.BeginOutputReadLine();

            // Add functions to the EventHandler
            this.client.OutputDataReceived += (sender, args) => { // args.data is all new text read from buffer since last event
                if(sender == null || args == null) return;
                string _buffer = args.Data.Replace($"{(char)27}[32m","\n"); // Fix first warning newline formatting
                _buffer = args.Data.Replace($"{(char)27}[m{(char)27}[32m","\n"); // Fix all newline formatting
                this.ConsoleOutput += _buffer; // Append "new" lines to the console output
            };

            this.client.ErrorDataReceived += (sender, args) => { // --||--
                if(sender == null || args == null) return;
                string _buffer = args.Data.Replace($"{(char)27}[32m","\n");
                _buffer = args.Data.Replace($"{(char)27}[m{(char)27}[32m","\n");
                this.ErrorOutput += _buffer;
            };
        }

        private void pollClient() {
            while(terminatingClient!=1) {
                switch(this.Status) {

                    case 0: // Not even initiated (Shouldn't be possible)
                        break;

                    case 1: // Initiated, but not started (Shouldn't be possible)
                        break;

                    case 2: // Starting (listen for startup finished)
                        if(this.ConsoleOutput.Contains("Done!")) {
                            this.Online = true;
                            this.Status = 3;
                        }
                        break;

                    case 3: // Running (poll for all relevant stats)
                        this.PlayersOnline = Regex.Matches(this.ConsoleOutput, "joined the game").Count - Regex.Matches(this.ConsoleOutput, "left the game").Count; // No. of joins - No. of leaves
                        break;

                }
            }
        }
    }
}