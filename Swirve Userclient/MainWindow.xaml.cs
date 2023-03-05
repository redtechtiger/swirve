using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Runtime.InteropServices;
using System.Windows.Interop;
using System.Windows.Threading;

namespace Swirve_Userclient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        FrameworkApi api = new FrameworkApi();
        DispatcherTimer dpTimer = new DispatcherTimer();
        private readonly object timerLock = new object();
        public MainWindow()
        {
            InitializeComponent();
        }

        private async void asyncUpdate(object sender, EventArgs e)
        {
            if (System.Threading.Monitor.TryEnter(timerLock))
            {
                string statusContent = "";
                Brush statusColor = Brushes.Transparent;
                string console = "";
                await Task.Run(() => poll_refresh(ref statusContent, ref statusColor, ref console));
                statusLabel.Content = statusContent;
                statusLabel.Foreground = statusColor;
                if(statusContent=="[Bad EPowerState]")
                {
                    ErrorMessage errMsg = new ErrorMessage();
                    errMsg.errorDescription.Content = "Warning: Bad EPowerState from Framework API";
                    errMsg.ShowDialog();
                }
                if (console != "-1")
                {
                    Console.WriteLine(console);
                    if (logOutput.Text == console) Console.WriteLine("Note: Console log hasn't changed");
                    logOutput.Text = console;
                    if(!logOutput.IsFocused) logOutput.ScrollToEnd();
                } else
                {
                    ErrorMessage errMsg = new ErrorMessage();
                    errMsg.errorDescription.Content = "Warning: Bad Socket from Framework API";
                    errMsg.ShowDialog();
                }
                System.Threading.Monitor.Exit(timerLock);
            } else
            {
                Console.WriteLine("ASYNCUPDATE: WARNING! TIMER CALLED WHILE POLL REFRESH WAS LOCKED!!!");
            }
        }

        private void poll_refresh(ref string statusContent, ref Brush statusColor, ref string console)
        {
            int pwr = api.GetPower();
            console = api.GetLog();
            console = console.Substring(Math.Max(0, console.Length - 5000));
            switch (pwr)
            {
                case 0:
                    statusContent = "Offline";
                    statusColor = Brushes.Red;
                    break;

                case 1:
                    statusContent = "Starting";
                    statusColor = Brushes.Orange;
                    break;
                case 2:
                    statusContent = "Online";
                    statusColor = Brushes.Green;
                    break;

                case 3:
                    statusContent = "Stopping";
                    statusColor = Brushes.Orange;
                    break;

                case 4:
                    statusContent = "Restarting";
                    statusColor = Brushes.Orange;
                    break;

                case 5:
                    statusContent = "Killing";
                    statusColor = Brushes.Orange;
                    break;

                case 6:
                    statusContent = "Server crash";
                    statusColor = Brushes.Red;
                    break;

                default:
                    statusContent = "[Bad EPowerState]";
                    break;
            }
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            ErrorMessage errMsg = new ErrorMessage();
            Hide();
            ErrorMessage _errMsg = new ErrorMessage();
            _errMsg.errorDescription.Content = "Caution: You are running a release candidate.\nSystem instability may be present.";
            _errMsg.errorAction.Content = "I understand";
            _errMsg.ShowDialog();
            initScreen _splash = new initScreen();
            _splash.Show();
            _splash.workDescriptor.Content = "Preparing API";
            await Task.Run(() => System.Threading.Thread.Sleep(200));
            _splash.workDescriptor.Content = "Connecting to Framework";
            await Task.Run(() => System.Threading.Thread.Sleep(500));
            string ip = Properties.Settings.Default.ip;
            int port = Properties.Settings.Default.port;
            api.Connect(ip, port);
            if(!api.connected) {
                errMsg.errorDescription.Content = "Failed to connect to Framework";
                errMsg.errorAction.Content = "Shut down";
                errMsg.ShowDialog();
                _splash.Close();
                Close();
                Application.Current.Shutdown();
                return;
            }
            _splash.workDescriptor.Content = "Waiting for API";
            await Task.Run(() => System.Threading.Thread.Sleep(500));
            string ans = api.Request("getval|pwr|0");
            _splash.workDescriptor.Content = "Request data: " + ans;
            dpTimer.Tick += new EventHandler(asyncUpdate);
            dpTimer.Interval = new TimeSpan(0, 0, 2);
            dpTimer.Start();

            _splash.workDescriptor.Content = "Starting Swirve Userclient";
            await Task.Run(() => System.Threading.Thread.Sleep(500));
            this.playersButton.IsEnabled = false;
            this.performanceButton.IsEnabled = false;
            this.configButton.IsEnabled = false;
            this.settingsButton.IsEnabled = false;
            this.userButton.IsEnabled = false;
            this.mainGrid.Visibility = Visibility.Visible;
            this.mainTabControl.SelectedIndex = 1;
            this.userButton.Visibility = Visibility.Hidden;
            this.userButtonPng.Visibility = Visibility.Hidden;
            this.settingsButton.Visibility = Visibility.Hidden;
            this.settingsButtonPng.Visibility = Visibility.Hidden;
            this.playersButton.Visibility = Visibility.Hidden;
            this.playersButtonPng.Visibility = Visibility.Hidden;
            this.performanceButton.Visibility = Visibility.Hidden;
            this.performanceButtonPng.Visibility = Visibility.Hidden;
            this.configButton.Visibility = Visibility.Hidden;
            this.configButtonPng.Visibility = Visibility.Hidden;

            _splash.workDescriptor.Content = "Welcome to the Future";
            await Task.Run(() => System.Threading.Thread.Sleep(2000));

            _splash.Close();
            Show();
            await Task.Run(() => System.Threading.Thread.Sleep(500));
            this.serverName.Text = Properties.Settings.Default.servername;
            await Task.Run(() => System.Threading.Thread.Sleep(200));
            string _serverport = Properties.Settings.Default.mcport.ToString();
            this.serverConnect.Content = Properties.Settings.Default.ip + ":" + _serverport;
        }

        private void dragBar_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if(e.ChangedButton==MouseButton.Left)
            {
                DragMove();
            }
        }

        private void exitButton_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void userButton_Click(object sender, RoutedEventArgs e)
        {
            switchTab(0);
        }

        private void overviewButton_Click(object sender, RoutedEventArgs e)
        {
            switchTab(1);
        }

        private void consoleButton_Click(object sender, RoutedEventArgs e)
        {
            switchTab(2);
        }

        private void playersButton_Click(object sender, RoutedEventArgs e)
        {
            switchTab(3);
        }

        private void performanceButton_Click(object sender, RoutedEventArgs e)
        {
            switchTab(4);
        }

        private void configButton_Click(object sender, RoutedEventArgs e)
        {
            switchTab(5);
        }

        private void settingsButton_Click(object sender, RoutedEventArgs e)
        {
            switchTab(6);
        }

        private async void switchTab(int index)
        {
            mainTabControl.Opacity = 0;
            mainTabControl.SelectedIndex = index;
            await fadeInMainTabControl();
        }

        private async Task fadeOutMainTabControl()
        {
            for (int i = 10; i > 0; i--)
            {
                mainTabControl.Opacity = (float)i/10;
                await Task.Delay(1);
            }
        }

        private async Task fadeInMainTabControl()
        {
            for (int i = 0; i < 10; i++)
            {
                mainTabControl.Opacity = (float)i/10;
                await Task.Delay(1);
            }
        }

        private async void startBtn_Click(object sender, RoutedEventArgs e)
        {
            await Task.Run(() => api.StartServer());
        }

        private async void stopBtn_Click(object sender, RoutedEventArgs e)
        {
            await Task.Run(() => api.StopServer());
        }

        private async void restartBtn_Click(object sender, RoutedEventArgs e)
        {
            await Task.Run(() => api.RestartServer());
        }

        private async void killBtn_Click(object sender, RoutedEventArgs e)
        {
            await Task.Run(() => api.KillServer());
        }

        private async void logInput_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.Key==Key.Enter)
            {
                string text = logInput.Text;
                await Task.Run(() => api.SetLog(text));
                logInput.Text = "";
            }
        }

        private async void dataDumpBtn_Click(object sender, RoutedEventArgs e)
        {
            dumpProgress dpProgress = new dumpProgress();
            dpProgress.Show();
            dpProgress.Topmost = true;
            string rawdata = "";
            dpProgress.progressDone.Value = 0;
            await Task.Run(() => asynclog(ref rawdata));
            dpProgress.progressDone.Value = 50;
            await Task.Run(() => writeToLogFile(rawdata));
            await Task.Run(() => Task.Delay(500));
            dpProgress.progressDone.Value = 100;
            await Task.Run(() => Task.Delay(1000));
            dpProgress.Close();
        }

        private void writeToLogFile(string rawdata)
        {
            string folderPath = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory) + $"\\Swirve-LogDump-{DateTime.Now.ToString("dd-MM-yyyy")}#{DateTime.Now.ToString("FFF")}.txt";
            Console.WriteLine(folderPath);
            StreamWriter sw = new StreamWriter(folderPath);
            sw.Write(rawdata);
            sw.Flush();
            sw.Close();
            return;
        }

        private void asynclog(ref string data)
        {
            data = api.GetLog();
        }

        private void serverName_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (serverName.Text == "Wait ...") return;
            Properties.Settings.Default.servername = serverName.Text;
            Properties.Settings.Default.Save();
        }
    } 
}
