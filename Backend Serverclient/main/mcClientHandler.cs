using System;
using System.Text;
using System.Linq;
using System.Diagnostics;
using System.Collections.Generic;

namespace McClientHandler {
    public class mcClientHandler {

        // <summary>
        // Declares all the variables 

        public string Name { get; private set; }
        public string Tps { get; private set; }
        public string[] PlayersOnline { get; private set; }
        public string[] PlayersWhitelisted { get; private set; }
        public int RamUsing { get; private set; }
        public int RamAllocated { get; private set; }
        public int Status { get; private set; }

        public bool Online { get; private set; }

        public string ConsoleOutput { get; private set; }
        public string ErrorOutput { get; private set; }


        private string startArgs { get; set; }
        private string shellPath { get; set; }
        private string jarPath { get; set; }
        private string workingDirectory { get; set; }

        private StringBuilder outputBuffer = new StringBuilder();

        private Process client = new Process();


        public void Init(string _name = "{Default}", int _ram = 2, string _javapath = "java", string _jarfile="") {

            // <summary>
            // Assign all variables
            // </summary>

            if(String.IsNullOrEmpty(_jarfile)) return; //Return if no client file is provided

            this.Name = _name;
            this.RamAllocated = _ram;
            this.shellPath = _javapath;
            this.jarPath = _jarfile;
            this.startArgs = $"-Xms{this.RamAllocated}G -Xmx{this.RamAllocated}G -jar \"{this.jarPath}\"";

            //Prepare working directory from jar path
            string[] _buffer = _jarfile.Split(new char[] {'/','\\'}); //Split jar path into list, with "\" as the separator
            _buffer = _buffer.SkipLast(1).ToArray(); //Get rid of filename
            this.workingDirectory = String.Join(@"\", _buffer);
            Console.WriteLine($"[DEBUG] Working directory ->{this.workingDirectory}");
        }

        public void Start() {

            // <summary>
            // Prepare the process for starting
            // </summary>

            this.client.StartInfo.WorkingDirectory = this.workingDirectory;
            this.client.StartInfo.FileName = this.shellPath;
            this.client.StartInfo.Arguments = this.startArgs;
            this.client.StartInfo.RedirectStandardOutput = true;
            this.client.StartInfo.RedirectStandardError = true;
            this.client.StartInfo.UseShellExecute = false;

            Console.WriteLine($"[DEBUG] Process startinfo: {this.client.StartInfo.WorkingDirectory},\n {this.client.StartInfo.FileName},\n {this.client.StartInfo.Arguments},\n {this.client.StartInfo.RedirectStandardOutput},\n {this.client.StartInfo.RedirectStandardError},\n {this.client.StartInfo.UseShellExecute}");

            // Start the Minecraft server
            this.client.Start();
        }

        public void Stop() {
            this.client.Kill();
        }

        public void StartListener() {
            // Starts asynchronous listening
            this.client.BeginOutputReadLine();

            //Add functions to the EventHandler
            this.client.OutputDataReceived += (sender, args) => { //args.data is all new text read from buffer since last event
                if(sender == null || args == null) return;
                string _buffer = args.Data.Replace($"{(char)27}[32m","\n"); //Fix first warning newline formatting
                _buffer = args.Data.Replace($"{(char)27}[m{(char)27}[32m","\n"); //Fix all newline formatting
                this.ConsoleOutput += _buffer; //Append new lines to the console output
            };

            this.client.ErrorDataReceived += (sender, args) => {
                if(sender == null || args == null) return;
                string _buffer = args.Data.Replace($"{(char)27}[32m","\n");
                _buffer = args.Data.Replace($"{(char)27}[m{(char)27}[32m","\n");
                this.ErrorOutput += _buffer;
            };
        }

        //public string FetchOutput() {
        //    return this.ConsoleOutput;
        //}

        //public string FetchError() {
        //    return this.ErrorOutput;
        //}
    }
}