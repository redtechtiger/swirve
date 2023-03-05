using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.IO;
using System.Threading;

namespace Swirve_Userclient
{
    class FrameworkApi
    {
        private TcpClient tcpClient;
        private Stream TcpClientStream;
        private object connectionLock = new object();
        public bool connected = false;

        private string getStrFromChar(byte[] input)
        {
            string outStr = "";
            for (int i = 0; i < input.Length; i++)
            {
                if(input[i]!=0) outStr += Convert.ToChar(input[i]);
            }
            return outStr;
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
            TcpClientStream = tcpClient.GetStream();
            Console.WriteLine($"[{DateTime.Now.ToString("HH:mm:ss")}] API: Connect: Connected");
        }

        public string Request(string data)
        {
            Console.WriteLine($"[{DateTime.Now.ToString("HH:mm:ss")}] API: Request: Waiting to aquire lock");
            lock(connectionLock) {
                Console.WriteLine($"[{DateTime.Now.ToString("HH:mm:ss")}] API: Request: Lock aquired, requesting data {data}...");
                if (!connected) return "";
                ASCIIEncoding asciiEncoding = new ASCIIEncoding();
                byte[] bytes = asciiEncoding.GetBytes(data);
                TcpClientStream.Write(bytes, 0, bytes.Length);
                byte[] bytesReturn = new byte[40000];
                Array.Clear(bytesReturn, 0, bytesReturn.Length);
                string stringOut = "";
                long totalRead = 0;
                int bytesRead = 0;
                try
                {
                    bytesRead = TcpClientStream.Read(bytesReturn, 0, bytesReturn.Length);
                } catch
                {
                    return "-1";
                }
                totalRead += bytesRead;
                stringOut = getStrFromChar(bytesReturn);
                while (!stringOut.Contains("\\\\_!end_!\\\\"))
                {
                    Array.Clear(bytesReturn, 0, bytesReturn.Length);
                    bytesRead = TcpClientStream.Read(bytesReturn, 0, bytesReturn.Length);
                    totalRead += bytesRead;
                    stringOut += getStrFromChar(bytesReturn);
                }
                Console.WriteLine($"[{DateTime.Now.ToString("HH:mm:ss")}] API: Request: Data retreived, {totalRead:n0}B with data: {stringOut.Substring(0,5)}...");
                stringOut = stringOut.Replace("\\\\_!end_!\\\\","");
                return stringOut;
            }
        }

        public void StartServer()
        {
            Request("setval|pwr|0");
        }

        public void StopServer()
        {
            Request("setval|pwr|1");
        }

        public void RestartServer()
        {
            Request("setval|pwr|3");
        }

        public void KillServer()
        {
            Request("setval|pwr|2");
        }

        public string GetLog()
        {
            return Request("getval|log|0");
        }

        public int GetPower()
        {
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
            Request("setval|log|" + data);
        }
    }
}
