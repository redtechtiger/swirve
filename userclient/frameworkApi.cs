using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Security.RightsManagement;
using System.Text.RegularExpressions;
using System.Collections.Specialized;
using System.IO.Packaging;
using System.Windows;
using System.Windows.Documents;
using System.Dynamic;

namespace Swirve_Userclient
{
    public class FrameworkApi
    {
        public struct ServerArchive
        {
            public ulong ID;
            public string Name;
            public int RamAllocated;
            public string LaunchPath;
            public int AssignedPort;
            public int JavaVersion;
        }

        public struct ServerModule
        {
            public ulong ID;
            public string Name;
            public int EPWRState;
        }

        public struct DataResponse_Swiss
        {
            public int ServerPower;
            public string ServerLog;
            public int ServerCPU;
            public int ServerVmRSS;
            public int ServerVmSize;

            public int SystemTotalMemory;
            public int SystemMemory;
            public int SystemTotalCores;
            public int SystemCPU;
        }

        public struct DataResponse_CPU
        {

        }

        public struct DataResponse_MEM
        {

        }

        private TcpClient tcpClient;
        private Stream TcpClientStream;
        private string logCache;
        private object connectionLock = new object();
        public bool connected = false;

        private string getStrFromChar(byte[] input)
        {
            return Encoding.UTF8.GetString(input).TrimEnd('\0');
        }

        public void Connect(string ip, int port)
        {
            connected = false;
            Console.WriteLine($"[{DateTime.Now.ToString("HH:mm:ss")}] API: Connect: Initializing TCPClient...");
            tcpClient = new TcpClient();
            Console.WriteLine($"[{DateTime.Now.ToString("HH:mm:ss")}] API: Connect: Connecting to TCPClient...");
            try
            {
                tcpClient.Connect(ip, port);
                connected = true;
            }
            catch { return; };
            tcpClient.SendTimeout = 5000;
            tcpClient.ReceiveTimeout = 5000;
            TcpClientStream = tcpClient.GetStream();
            Console.WriteLine($"[{DateTime.Now.ToString("HH:mm:ss")}] API: Connect: Connected");
        }

        public void Disconnect() {
            lock(connectionLock) {
                tcpClient.Close();
                connected = false;
            }
        }

        public string Request(string data)
        {
            lock(connectionLock) {
                if (!connected) return "";
                ASCIIEncoding asciiEncoding = new ASCIIEncoding();
                byte[] bytes = asciiEncoding.GetBytes(data);
                try
                {
                    TcpClientStream.Write(bytes, 0, bytes.Length);
                } catch
                {
                    Thread.Sleep(1000);
                    connected = false;
                    return "-1";
                }
                byte[] bytesReturn = new byte[4000];
                Array.Clear(bytesReturn, 0, bytesReturn.Length);
                StringBuilder stringOut = new StringBuilder();
                long totalRead = 0;
                int bytesRead = 0;
                try
                {
                    bytesRead = TcpClientStream.Read(bytesReturn, 0, bytesReturn.Length);
                } catch
                {
                    Thread.Sleep(1000);
                    connected = false;
                    return "-1";
                }
                totalRead += bytesRead;
                stringOut.Append(getStrFromChar(bytesReturn));
                while (!stringOut.ToString().Contains("\\\\_!end_!\\\\"))
                {
                    Array.Clear(bytesReturn, 0, bytesReturn.Length);
                    try
                    {
                        bytesRead = TcpClientStream.Read(bytesReturn, 0, bytesReturn.Length);
                    }
                    catch
                    {
                        Thread.Sleep(1000);
                        connected = false;
                        return "-1";
                    }
                    totalRead += bytesRead;
                    stringOut.Append(getStrFromChar(bytesReturn));
                }
                stringOut = stringOut.Replace("\\\\_!end_!\\\\","");
                return stringOut.ToString();
            }
        }

        public void StartServer()
        {
            Console.WriteLine("FrameworkAPI: Starting remote server");
            Request("setval|pwr|0");
        }

        public void StopServer()
        {
            Console.WriteLine("FrameworkAPI: Stopping remote server");
            Request("setval|pwr|1");
        }

        public void RestartServer()
        {
            Console.WriteLine("FrameworkAPI: Restarting remote server");
            Request("setval|pwr|3");
        }

        public void KillServer()
        {
            Console.WriteLine("FrameworkAPI: Killing remote server");
            Request("setval|pwr|2");
        }

        public string GetLog()
        {
            Console.WriteLine("FrameworkAPI: Fetching log of remote server");
            logCache = Request("getval|log|0");
            return logCache;
        }

        public int GetPower()
        {
            Console.WriteLine("FrameworkAPI: Fetching status of remote server");
            string outs = Request("getval|pwr|0");
            try {
                return int.Parse(outs);
            } catch
            {
                Console.WriteLine("WARNING!!!! RETURNING FAULTY POWERSTATE (RETURN="+outs+")");
                return -1;
            }
        }
        public void SetLog(string data)
        {
            Console.WriteLine("FrameworkAPI: Writing to input of remote server");
            Request("setval|log|" + data);
        }

        public List<ServerModule> GetModules()
        {
            Console.WriteLine("FrameworkAPI: Getting modules of remote server");
            try
            {
                string buffer = Request("getval|mod|0");
                List<string> moduletokens = new List<string>(buffer.Split(new string[] { @"||" }, StringSplitOptions.RemoveEmptyEntries));
                List<ServerModule> modules = new List<ServerModule>();
                foreach (string moduledata in moduletokens)
                {
                    List<String> fields = new List<String>(moduledata.Split(new string[] { @"|" }, StringSplitOptions.None));
                    ServerModule module = new ServerModule
                    {
                        ID = ulong.Parse(fields[0]),
                        Name = fields[1],
                        EPWRState = int.Parse(fields[2])
                    };
                    modules.Add(module);
                }
                return modules;
            } catch
            {
                return new List<ServerModule> { };
            }
        }

        public int SetModule(ulong moduleId)
        {
            Console.WriteLine("FrameworkAPI: Setting assigned module of remote server");
            try
            {
                string buffer = Request("setval|mod|"+moduleId);
                if(int.Parse(buffer.Split('|')[0])!=0)
                    Console.WriteLine($"Failed to set module: {buffer.Split('|')[1]}");
                return int.Parse(buffer.Split('|')[0]);
            } catch
            {
                return -2;
            }
        }

        public ServerArchive GetArchive(ulong moduleId)
        {
            List<string> buffer = Request("dynamicmod|get|" + moduleId).Split('|').ToList();
            try
            {
                return new ServerArchive()
                {
                    ID = moduleId,
                    Name = buffer[0],
                    RamAllocated = int.Parse(buffer[1]),
                    AssignedPort = int.Parse(buffer[2]),
                    LaunchPath = buffer[3],
                    JavaVersion = int.Parse(buffer[4]),

                };
            } catch
            {
                return new ServerArchive() { ID = 0 };
            }
        }

        public int GetCachedPlayerCount()
        {
            return Regex.Matches(logCache, @"\[Server thread/INFO\] \[minecraft/DedicatedServer\]: (.*) joined the game").Count - Regex.Matches(logCache, @"\[Server thread/INFO\] \[minecraft/DedicatedServer\]: (.*) left the game").Count;
        }

        public List<String> GetCachedPlayers()
        {
            List<string> players = new List<string>();
            MatchCollection joinmatches = Regex.Matches(logCache, @"\[Server thread/INFO\] \[minecraft/DedicatedServer\]: (.*) joined the game");
            MatchCollection leavematches = Regex.Matches(logCache, @"\[Server thread/INFO\] \[minecraft/DedicatedServer\]: (.*) left the game");
            foreach (Match match in joinmatches)
            {
                players.Add(match.Groups[1].ToString());
            }
            foreach (Match match in leavematches)
            {
                players.Remove(match.Groups[1].ToString());
            }
            return players;
        }

        public ulong CreateServer(string name, int ram, string launchpath = "placeholder", int javaversion = 8)
        {
            try
            {
                return ulong.Parse(Request("dynamicmod|add|" + name + "|" + ram + "|" + launchpath + "|" + javaversion).Split('|')[0]);
            } catch { return 0; }
        }

        public List<string> GetCredentials()
        {
                return Request("getval|cred|0").Split('|').ToList();
        }

        public bool OverwriteServer(ulong id, string name, int ram, string launchpath, int javaversion)
        {
            string buffer = Request("dynamicmod|overwrite|"+id+"|"+name+"|"+ram+"|"+launchpath+"|"+javaversion);
            return buffer.Split('|')[0] == "0";
        }

        public bool PortPrepare()
        {
            string buffer = Request("dynamicmod|portprep|0");
            return buffer.Split('|')[0] == "0";
        }

        public bool DeleteServer(ulong id)
        {
            string buffer = Request("dynamicmod|del|"+id);
            return buffer.Split('|')[0] == "0";
        }

        public bool Authenticate(string key)
        {
            string buffer = Request("elevatereq|authorise|"+key);
            return buffer.Split('|')[0] == "0";
        }

        public bool RestartFramework()
        {
            string buffer = Request("elevatereq|frameworkrestart|");
            return buffer.Split('|')[0] == "0";
        }

        public bool ShutdownFramework()
        {
            string buffer = Request("elevatereq|frameworkshutdown|");
            return buffer.Split('|')[0] == "0";
        }

        public bool PowerdownSystem()
        {
            string buffer = Request("elevatereq|frameworkpowerdown|");
            return buffer.Split('|')[0] == "0";
        }

        public bool RebootSystem()
        {
            string buffer = Request("elevatereq|frameworkreboot|");
            return buffer.Split('|')[0] == "0";
        }

        public bool KillAllConnections()
        {
            string buffer = Request("elevatereq|frameworkkillall|");
            return buffer.Split('|')[0] == "0";
        }

        public FrameworkApi.DataResponse_Swiss GetSwiss()
        {
            try
            {
                // Get buffer
                string buffer = Request("getval|sws|0");

                // Tokenize and get values
                List<string> tokencollection = new List<string>(buffer.Split(new string[] { "||" }, StringSplitOptions.None));
                List<string> powertokens = new List<string>(tokencollection[0].Split('|'));
                List<string> cputokens = new List<string>(tokencollection[1].Split('|'));
                List<string> memtokens = new List<string>(tokencollection[2].Split('|'));
                string log = tokencollection[3];

                logCache = log;

                FrameworkApi.DataResponse_Swiss dataresponse = new DataResponse_Swiss()
                {
                    ServerPower = int.Parse(powertokens[0]),
                    SystemTotalCores = int.Parse(cputokens[0]),
                    SystemCPU = int.Parse(cputokens[1]),
                    ServerCPU = int.Parse(cputokens[2]),
                    SystemTotalMemory = int.Parse(memtokens[0]),
                    SystemMemory = int.Parse(memtokens[0]) - int.Parse(memtokens[1]), // memotokens[1] is free memory, subtract from total to get usage
                    ServerVmSize = int.Parse(memtokens[2]),
                    ServerVmRSS = int.Parse(memtokens[3]),
                    ServerLog = log,
                };

                return dataresponse;
            }
            catch
            {
                FrameworkApi.DataResponse_Swiss dataresponse = new DataResponse_Swiss()
                {
                    ServerPower = 6,
                    SystemTotalCores = 0,
                    SystemCPU =  0,
                    ServerCPU = 0,
                    SystemTotalMemory = 0,
                    SystemMemory = 0,
                    ServerVmSize = 0,
                    ServerVmRSS = 0,
                    ServerLog = "Internal FrameworkAPI Error",
                };

                return dataresponse;
            }
        }
    }
}
