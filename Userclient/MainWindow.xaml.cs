/* Copyright (c) 1998-2008 RedTechTiger Media */

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
using System.Threading;
using System.Reflection;
using System.Runtime.Remoting.Channels;
using Swirve_Userclient;
using System.Windows.Media.Animation;
using System.Drawing.Drawing2D;
using LiveCharts;
using LiveCharts.Wpf;
using Swirve_Userclient.Properties;
using System.Net;
using System.Windows.Markup.Localizer;
using static Swirve_Userclient.MainWindow;
using System.Collections.ObjectModel;

namespace Swirve_Userclient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
     
    public static class ProgressBarExtensions
    {
        private static TimeSpan duration = TimeSpan.FromSeconds(.2);

        public static void SetPercent(this ProgressBar progressBar, double percentage)
        {
            DoubleAnimation animation = new DoubleAnimation(percentage, duration);
            progressBar.BeginAnimation(ProgressBar.ValueProperty, animation);
        }
    }

    public partial class MainWindow : Window
    {
        string[] PowerString = new string[] { "Node is offline 🚫", "Node starting... 🕑", "Node is online ☁", "Node stopping... ✋", "Node restarting... 🕑", "Node stopping... ✋", "Node crashed ⚠" };

        object powerdownlock = new object();

        FrameworkApi api = new FrameworkApi();
        FrameworkApi.ServerArchive serverArchive = new FrameworkApi.ServerArchive();
        DispatcherTimer refreshTimer = new DispatcherTimer();
        SemaphoreSlim refreshsf = new SemaphoreSlim(1);

        int TotalModules = 0;
        int OnlineModules = 0;

        int user_serverjava;

        bool user_legacyapi;
        bool user_darkmode;

        List<PlayerVisual> playerVisualsCache = new List<PlayerVisual>();
        string selectedPlayer;


        public SeriesCollection SeriesCollection { get; set; }
        public List<string> GraphTimes { get; set; }
        public Func<double, string> YDataFormatter { get; set; }


        public MainWindow()
        {
            InitializeComponent();

            SeriesCollection = new SeriesCollection
            {
                new LineSeries
                {
                    Title = "Utilization",
                    Values = new ChartValues<double> { }
                },
                new LineSeries
                {
                    Title = "Allocated",
                    Values = new ChartValues<double> { }
                }
                ,
                new LineSeries
                {
                    Title = "Memory",
                    Values = new ChartValues<double> { }
                }
            };

            GraphTimes = new List<string>();
            YDataFormatter = value => value.ToString() + "%";

            DataContext = this;
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            rootTabControl.SelectedIndex = 0;
            rootStart_Progressbar.Value = 0;
            ErrorMessage errMsg = new ErrorMessage(); // Windows to be used

            // Fetch connection prerequisites
            string ip = Properties.Settings.Default.ip;
            int port = Properties.Settings.Default.port;

            // Connect
            await Task.Run(() => api.Connect(ip, port));
            if (!api.connected)
            {

                // If failed, show this error
                errMsg.errorDescription.Content = "Cluster Error";
                errMsg.errorContext.Text = "Couldn't connect to Framework";
                errMsg.errorAction.Content = "Exit";
                errMsg.ShowDialog();

                // Shut down Userclient
                Close();
                Application.Current.Shutdown();
                return;
            }

            rootStart_Progressbar.SetPercent(10);

            // Fetch all servers
            List<FrameworkApi.ServerModule> modules = new List<FrameworkApi.ServerModule>();
            await Task.Run(() => { modules = api.GetModules(); });

            rootStart_Progressbar.SetPercent(30);

            // User selects server
            ulong selectedModuleID = getUserServer(modules);

            
        }

        private ulong getUserServer(List<FrameworkApi.ServerModule> modules)
        {
            UploadModules(modules);
            RenderModules();

            rootTabControl.SelectedIndex = 1;

            return 0;
        }

        private void stopuserclient()
        {
            lock(powerdownlock)
            {
                refreshTimer.Stop();
                ErrorMessage errMsg = new ErrorMessage();
                errMsg.errorDescription.Content = "Cluster Timeout";
                errMsg.errorContext.Text = "Framework stopped responding";
                errMsg.errorAction.Content = "Exit";
                errMsg.ShowDialog();
                api.Disconnect();
                Application.Current.Shutdown();
            }
        }

        private async void refresh(object sender, EventArgs e)
        {
            if(!await refreshsf.WaitAsync(TimeSpan.FromSeconds(2)))
            {
                return;
            } else
            {
                try
                {
                    // Asynchronously fetch all server values
                    FrameworkApi.DataResponse_Swiss dataresponse = new FrameworkApi.DataResponse_Swiss();

                    switch (Settings.Default.legacysupport)
                    {
                        case false:
                            await Task.Run(() => dataresponse = api.GetSwiss());
                            break;

                        case true:
                            string legacylog = "";
                            int legacypower = 0;
                            await Task.Run(() => legacylog = api.GetLog());
                            await Task.Run(() => legacypower = api.GetPower());
                            dataresponse.ServerPower = legacypower;
                            dataresponse.ServerLog = legacylog;
                            break;
                    }

                    // Make sure Framework responded correctly
                    if (!api.connected)
                    {
                        // Shut down userclient
                        stopuserclient();

                        // If the timer ticks again, simply return
                        return;
                    }

                    // Set values correctly in UI
                    this.Dispatcher.Invoke(() =>
                    {
                        Overview_ServerPower.Content = PowerString[dataresponse.ServerPower];

                        UpdatePowerSpinner.Visibility = Visibility.Hidden;

                        Overview_CpuProgressbar.Value = dataresponse.ServerCPU;
                        Overview_CpuText.Content = "" + dataresponse.ServerCPU.ToString() + "% utilization";
                        Overview_RamProgressbar.Maximum = serverArchive.RamAllocated * 1024; // From GB to MB
                        Overview_RamProgressbar.Value = dataresponse.ServerVmRSS;
                        Overview_RamText.Content = "" + Math.Round((double)dataresponse.ServerVmRSS / 1024, 1).ToString() + "GB memory";
                        Overview_PlayersOnline.Content = api.GetCachedPlayerCount() + " online";

                        Overview_Console.Text = dataresponse.ServerLog.Substring(dataresponse.ServerLog.Length - Math.Min(Settings.Default.consolecutoff, dataresponse.ServerLog.Length));
                        Overview_Console.ScrollToEnd();

                        // Performance_TotalRam.Content = "RAM " + Math.Round((double)dataresponse.SystemTotalMemory / 1024, 1).ToString() + " GB";
                        //Performance_TotalCores.Content = "CORES " + dataresponse.SystemTotalCores.ToString();
                        //Performance_SysCpuProgressbar.Value = dataresponse.SystemCPU;
                        //Performance_SysCpuText.Content = "CPU " + dataresponse.SystemCPU.ToString() + "%";
                        //Performance_SysRamProgressbar.Value = dataresponse.SystemMemory;
                        //Performance_SysRamProgressbar.Maximum = dataresponse.SystemTotalMemory;
                        //Performance_SysRamText.Content = "RAM " + Math.Round((double)dataresponse.SystemMemory / 1024, 1) + "GB";
                        Performance_CpuProgressbar.Value = dataresponse.ServerCPU;
                        Performance_CpuText.Content = "" + dataresponse.ServerCPU.ToString() + "% utilization";
                        Performance_VmSizeProgressbar.Value = dataresponse.ServerVmSize;
                        Performance_VmSizeProgressbar.Maximum = serverArchive.RamAllocated * 1024;
                        Performance_VmSizeText.Content = "" + Math.Round((double)dataresponse.ServerVmSize / 1024, 1).ToString() + "GB memory";
                        Performance_VmRSSProgressbar.Value = dataresponse.ServerVmRSS;
                        Performance_VmRSSProgressbar.Maximum = serverArchive.RamAllocated * 1024;
                        Performance_VmRSSText.Content = "" + Math.Round((double)dataresponse.ServerVmRSS / 1024, 1) + "GB allocated";

                        //Performance_ModulesLoaded.Content = "Loaded Modules: " + TotalModules;
                        //Performance_ModulesOnline.Content = "Online Modules: " + OnlineModules;

                        logOutput.Text = dataresponse.ServerLog.Substring(dataresponse.ServerLog.Length - Math.Min(Properties.Settings.Default.terminalcutoff, dataresponse.ServerLog.Length));
                        if (!logOutput.IsFocused) logOutput.ScrollToEnd();

                        players_count.Content = api.GetCachedPlayerCount();
                        buildPlayerList(api.GetCachedPlayers());


                        // Update graph data!!
                        SeriesCollection[0].Values.Add((double)dataresponse.ServerCPU);
                        SeriesCollection[1].Values.Add((double)dataresponse.ServerVmSize / (serverArchive.RamAllocated * 1024) * 100);
                        SeriesCollection[2].Values.Add((double)dataresponse.ServerVmRSS / (serverArchive.RamAllocated * 1024) * 100);
                        GraphTimes.Add(DateTime.Now.ToString("HH:mm:ss"));

                        DataChart.Update();

                        // Too much data in graph
                        if (SeriesCollection[0].Values.Count > 30)
                        {
                            SeriesCollection[0].Values.RemoveAt(0);
                            SeriesCollection[1].Values.RemoveAt(0);
                            SeriesCollection[2].Values.RemoveAt(0);
                            GraphTimes.RemoveAt(0);
                        }



                        // Disable the correct power buttons depending on power state
                        if (dataresponse.ServerPower == 0)
                        {
                            Overview_StartServer.IsEnabled = true;
                            Overview_StopServer.IsEnabled = false;
                            Overview_RestartServer.IsEnabled = false;
                            Overview_KillServer.IsEnabled = false;
                        }
                        if (dataresponse.ServerPower == 1)
                        {
                            Overview_StartServer.IsEnabled = false;
                            Overview_StopServer.IsEnabled = false;
                            Overview_RestartServer.IsEnabled = false;
                            Overview_KillServer.IsEnabled = true;
                        }
                        if (dataresponse.ServerPower == 2)
                        {
                            Overview_StartServer.IsEnabled = false;
                            Overview_StopServer.IsEnabled = true;
                            Overview_RestartServer.IsEnabled = true;
                            Overview_KillServer.IsEnabled = true;
                        }
                        if (dataresponse.ServerPower == 3)
                        {
                            Overview_StartServer.IsEnabled = false;
                            Overview_StopServer.IsEnabled = false;
                            Overview_RestartServer.IsEnabled = false;
                            Overview_KillServer.IsEnabled = true;
                        }
                        if (dataresponse.ServerPower == 4)
                        {
                            Overview_StartServer.IsEnabled = false;
                            Overview_StopServer.IsEnabled = false;
                            Overview_RestartServer.IsEnabled = false;
                            Overview_KillServer.IsEnabled = true;
                        }
                        if (dataresponse.ServerPower == 5)
                        {
                            Overview_StartServer.IsEnabled = true;
                            Overview_StopServer.IsEnabled = false;
                            Overview_RestartServer.IsEnabled = false;
                            Overview_KillServer.IsEnabled = true;
                        }
                        if (dataresponse.ServerPower == 6)
                        {
                            Overview_StartServer.IsEnabled = true;
                            Overview_StopServer.IsEnabled = false;
                            Overview_RestartServer.IsEnabled = false;
                            Overview_KillServer.IsEnabled = false;
                        }
                    });
                } finally
                {
                    refreshsf.Release();
                }
            }            
        }

        private void dragBar_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if(e.ChangedButton==MouseButton.Left)
            {
                DragMove();
            }
        }

        private void logout(object sender, RoutedEventArgs e)
        {
            refreshTimer.Stop();
            api.Disconnect();
            Application.Current.Shutdown();
        }

        private async void switch_server(object sender, RoutedEventArgs e)
        { 
            // Stop refresher
            refreshTimer.Stop();

            rootStart_Progressbar.SetPercent(10);

            // Fetch all servers
            List<FrameworkApi.ServerModule> modules = new List<FrameworkApi.ServerModule>();
            await Task.Run(() => { modules = api.GetModules(); });

            rootStart_Progressbar.SetPercent(30);

            // User selects server
            ulong selectedModuleID = getUserServer(modules);
        }

        private void show_dashboard(object sender, RoutedEventArgs e)
        {
            button_dashboard.IsEnabled = false;
            button_terminal.IsEnabled = true;
            button_players.IsEnabled = true;
            button_performance.IsEnabled = true;
            button_configuration.IsEnabled = true;
            button_admin.IsEnabled = true;
            button_settings.IsEnabled = true;
            ChangeTab(0);
        }

        private void show_terminal(object sender, RoutedEventArgs e)
        {
            button_dashboard.IsEnabled = true;
            button_terminal.IsEnabled = false;
            button_players.IsEnabled = true;
            button_performance.IsEnabled = true;
            button_configuration.IsEnabled = true;
            button_admin.IsEnabled = true;
            button_settings.IsEnabled = true;
            ChangeTab(1);
        }

        private void show_players(object sender, RoutedEventArgs e)
        {
            button_dashboard.IsEnabled = true;
            button_terminal.IsEnabled = true;
            button_players.IsEnabled = false;
            button_performance.IsEnabled = true;
            button_configuration.IsEnabled = true;
            button_admin.IsEnabled = true;
            button_settings.IsEnabled = true;
            selectedPlayer = "";
            ChangeTab(2);
        }

        private void show_performance(object sender, RoutedEventArgs e)
        {
            button_dashboard.IsEnabled = true;
            button_terminal.IsEnabled = true;
            button_players.IsEnabled = true;
            button_performance.IsEnabled = false;
            button_configuration.IsEnabled = true;
            button_admin.IsEnabled = true;
            button_settings.IsEnabled = true;
            ChangeTab(3);
        }

        private void show_configuration(object sender, RoutedEventArgs e)
        {
            button_dashboard.IsEnabled = true;
            button_terminal.IsEnabled = true;
            button_players.IsEnabled = true;
            button_performance.IsEnabled = true;
            button_configuration.IsEnabled = false;
            button_admin.IsEnabled = true;
            button_settings.IsEnabled = true;
            ChangeTab(4);
        }

        private void show_admin(object sender, RoutedEventArgs e)
        {
            button_dashboard.IsEnabled = true;
            button_terminal.IsEnabled = true;
            button_players.IsEnabled = true;
            button_performance.IsEnabled = true;
            button_configuration.IsEnabled = true;
            button_admin.IsEnabled = false;
            button_settings.IsEnabled = true;
            ChangeTab(5);
        }

        private void show_settings(object sender, RoutedEventArgs e)
        {
            button_dashboard.IsEnabled = true;
            button_terminal.IsEnabled = true;
            button_players.IsEnabled = true;
            button_performance.IsEnabled = true;
            button_configuration.IsEnabled = true;
            button_admin.IsEnabled = true;
            button_settings.IsEnabled = false;
            ChangeTab(6);
        }

        private async void StartRemoteServer(object sender, RoutedEventArgs e)
        {
            UpdatePowerSpinner.Visibility = Visibility.Visible;
            await Task.Run(() => api.StartServer());
        }

        private async void StopRemoteServer(object sender, RoutedEventArgs e)
        {
            UpdatePowerSpinner.Visibility = Visibility.Visible;
            await Task.Run(() => api.StopServer());
        }

        private async void RestartRemoteServer(object sender, RoutedEventArgs e)
        {
            UpdatePowerSpinner.Visibility = Visibility.Visible;
            await Task.Run(() => api.RestartServer());
        }

        private async void KillRemoteServer(object sender, RoutedEventArgs e)
        {
            UpdatePowerSpinner.Visibility = Visibility.Visible;
            await Task.Run(() => api.KillServer());
        }

        private void logInput_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.Key==Key.Enter)
            {
                sendLogBtn_Click(sender, e);
            } else if (e.Key==Key.Escape)
            {
                Keyboard.ClearFocus();
            }
        }

        private void asynclog(ref string data)
        {
            data = api.GetLog();
        }

        private void logInput_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if(logInput.Text == "Enter command...") {
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

        private void logInput_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if(logInput.Text == "") {
                logInput.Text = "Enter command...";
            }
        }

        private async void sendLogBtn_Click(object sender, RoutedEventArgs e)
        {
            if (!sendBtn.IsEnabled) return;
            sendBtn.IsEnabled = false;
            Keyboard.ClearFocus();
            string text = logInput.Text;
            if (text == "") text = " ";
            await Task.Run(() => api.SetLog(text));
            logInput.Text = "";
            sendBtn.IsEnabled = true;
        }

        private void buildPlayerList(List<string> players) {
            playerVisualsCache.Clear();
            player_list.ItemsSource = null;
            foreach(string player in players)
            {
                if(player == selectedPlayer)
                {
                    PlayerVisual playerVisual = new PlayerVisual()
                    {
                        PlayerName = player,
                        PlayerNameTag = player,
                        HighlightColor = (Brush)FindResource("Color_Palette3"),
                    };
                    playerVisualsCache.Add(playerVisual);
                } else
                {
                    PlayerVisual playerVisual = new PlayerVisual()
                    {
                        PlayerName = player,
                        PlayerNameTag = player,
                        HighlightColor = (Brush)FindResource("Color_Palette2"),
                    };
                    playerVisualsCache.Add(playerVisual);
                }
                
            }
            player_list.ItemsSource = playerVisualsCache;
            player_list.UpdateLayout();
        }

        private void logOutput_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.Escape)
            {
                console_log_border.BorderThickness = new Thickness(0);
                mainGrid.Focus();
            }
        }

        private void logOutput_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            console_log_border.BorderThickness = new Thickness(2);
            console_log_border.BorderBrush = Brushes.DarkCyan;
        }

        private void playerSelectedEvent(object sender, MouseButtonEventArgs e)
        {
            (sender as Border).Background = (Brush)FindResource("Color_Palette3");
            selectedPlayer = (sender as Border).Tag.ToString();
        }

        public class PlayerVisual
        {
            public string PlayerName { get; set; }
            public string PlayerNameTag { get; set; }
            public Brush HighlightColor { get; set; }
        };

        private async void ChangeTab(int index)
        {
            if(index == mainTabControl.SelectedIndex) { return; }
            DoubleAnimation animation = new DoubleAnimation();
            animation.Duration = (Duration)new DurationConverter().ConvertFrom("0:0:0.1");
            animation.From = 1.0;
            animation.To = 0.0;
            mainTabControl.BeginAnimation(OpacityProperty, animation);
            await Task.Run(() => Thread.Sleep(200));
            mainTabControl.SelectedIndex = index;
        }

        private void Border_Unloaded(object sender, RoutedEventArgs e)
        {
            stopuserclient();
        }

        private void java8btn_MouseDown(object sender, RoutedEventArgs e)
        {
            java16btn.IsEnabled = true;
            java17btn.IsEnabled = true;
            java18btn.IsEnabled = true;
            java8btn.IsEnabled = false;
            user_serverjava = 8;
        }

        private void java16btn_MouseDown(object sender, RoutedEventArgs e)
        {
            java16btn.IsEnabled = false;
            java17btn.IsEnabled = true;
            java18btn.IsEnabled = true;
            java8btn.IsEnabled = true;
            user_serverjava = 16;
        }

        private void java17btn_MouseDown(object sender, RoutedEventArgs e)
        {
            java16btn.IsEnabled = true;
            java17btn.IsEnabled = false;
            java18btn.IsEnabled = true;
            java8btn.IsEnabled = true;
            user_serverjava = 17;
        }

        private void java18btn_MouseDown(object sender, RoutedEventArgs e)
        {
            java16btn.IsEnabled = true;
            java17btn.IsEnabled = true;
            java18btn.IsEnabled = false;
            java8btn.IsEnabled = true;
            user_serverjava = 18;
        }

        private void DiscardButton_Click(object sender, RoutedEventArgs e)
        {
            // Update config page
            List<string> credentials = api.GetCredentials();
            ftp_ip.Text = credentials[0];
            ftp_port.Text = credentials[1];
            ftp_username.Text = credentials[2];
            ftp_password.Text = credentials[3];
            ftp_id.Text = serverArchive.ID.ToString();
            Configuration_servername.Text = serverArchive.Name;
            Configuration_serverram.Text = serverArchive.RamAllocated.ToString() + "GB";
            Configuration_serverpath.Text = serverArchive.LaunchPath;
            switch (serverArchive.JavaVersion)
            {
                case 8:
                    java16btn.IsEnabled = true;
                    java17btn.IsEnabled = true;
                    java18btn.IsEnabled = true;
                    java8btn.IsEnabled = false;
                    user_serverjava = 8;
                    break;

                case 16:
                    java16btn.IsEnabled = false;
                    java17btn.IsEnabled = true;
                    java18btn.IsEnabled = true;
                    java8btn.IsEnabled = true;
                    user_serverjava = 16;
                    break;

                case 17:
                    java16btn.IsEnabled = true;
                    java17btn.IsEnabled = false;
                    java18btn.IsEnabled = true;
                    java8btn.IsEnabled = true;
                    user_serverjava = 17;
                    break;

                case 18:
                    java16btn.IsEnabled = true;
                    java17btn.IsEnabled = true;
                    java18btn.IsEnabled = false;
                    java8btn.IsEnabled = true;
                    user_serverjava = 18;
                    break;
            }
        }

        private async void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            // Try to parse data
            int ram;
            try {
                ram = int.Parse(Configuration_serverram.Text.Replace("GB",""));
            } catch
            {
                ErrorMessage error = new ErrorMessage();
                error.errorDescription.Content = "Invalid Field";
                error.errorContext.Text = "Field ServerRAM is invalid. Please verify.";
                error.errorAction.Content = "OK";
                error.Show();
                return;
            }

            if(Configuration_servername.Text=="")
            {
                ErrorMessage error = new ErrorMessage();
                error.errorDescription.Content = "Invalid Field";
                error.errorContext.Text = "Field ServerNAME is invalid. Please verify.";
                error.errorAction.Content = "OK";
                error.Show();
                return;
            }

            if (Configuration_servername.Text == "")
            {
                ErrorMessage error = new ErrorMessage();
                error.errorDescription.Content = "Invalid Field";
                error.errorContext.Text = "Field ServerNAME is invalid. Please verify.";
                error.errorAction.Content = "OK";
                error.Show();
                return;
            }

            // Show loading wheel
            ApplicationWide_StartLoad("Saving...");

            // Overwrite!
            Console.WriteLine("Overwriting new server JAVA with " + user_serverjava);
            ulong id = serverArchive.ID;
            string name = Configuration_servername.Text;
            string launch = Configuration_serverpath.Text;

            Console.WriteLine();
            await Task.Run(() => api.OverwriteServer(id, name, ram, launch, user_serverjava));
            await Task.Run(() => Thread.Sleep(1000));

            ApplicationWide_StartLoad("Refreshing...");
            await Task.Run(() => { serverArchive = api.GetArchive(serverArchive.ID); });
            Overview_Servername.Content = serverArchive.Name;
            Overview_ServerPort.Text = serverArchive.AssignedPort.ToString();
            Overview_ServerRamTotal.Content = "💾 " + serverArchive.RamAllocated + "GB";
            Overview_ServerJava.Content = "⚙ Java " + serverArchive.JavaVersion;
            await Task.Run(() => Thread.Sleep(1000));

            // Hide loading wheel
            ApplicationWide_EndLoad();

        }

        private void ApplicationWide_StartLoad(string comment)
        {
            ApplicationWide_Parent.Visibility = Visibility.Visible;
            DoubleAnimation backgroundfade = new DoubleAnimation();
            backgroundfade.Duration = (Duration)new DurationConverter().ConvertFrom("0:0:0.1");
            backgroundfade.From = 0.0;
            backgroundfade.To = 0.7;
            DoubleAnimation foregroundfade = new DoubleAnimation();
            foregroundfade.Duration = (Duration)new DurationConverter().ConvertFrom("0:0:0.1");
            foregroundfade.From = 0.0;
            foregroundfade.To = 1;
            ApplicationWide_Background.BeginAnimation(OpacityProperty, backgroundfade);
            ApplicationWide_Foreground.BeginAnimation(OpacityProperty, foregroundfade);

            ApplicationWide_Text.Content = comment;
        }

        private async void ApplicationWide_EndLoad()
        {
            DoubleAnimation backgroundfade = new DoubleAnimation();
            backgroundfade.Duration = (Duration)new DurationConverter().ConvertFrom("0:0:0.1");
            backgroundfade.From = 0.7;
            backgroundfade.To = 0.0;
            DoubleAnimation foregroundfade = new DoubleAnimation();
            foregroundfade.Duration = (Duration)new DurationConverter().ConvertFrom("0:0:0.1");
            foregroundfade.From = 1.0;
            foregroundfade.To = 0;
            ApplicationWide_Background.BeginAnimation(OpacityProperty, backgroundfade);
            ApplicationWide_Foreground.BeginAnimation(OpacityProperty, foregroundfade);
            await Task.Run(() => Thread.Sleep(500));
            ApplicationWide_Parent.Visibility = Visibility.Hidden;
        }

        private async void DeleteButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("Deleting...");

            // Wipe server
            await Task.Run(() => api.DeleteServer(serverArchive.ID));
            await Task.Run(() => Thread.Sleep(1000));


            ApplicationWide_EndLoad();

            switch_server(sender, e);
        }

        private async void LoginButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("Authenticating...");

            string key = Admin_Key.Text;
            bool authenticated = false;
            await Task.Run(() => authenticated = api.Authenticate(key));
            await Task.Run(() => Thread.Sleep(1000));

            ApplicationWide_EndLoad();

            if(authenticated)
            {
                ChangeTab(7);
            } else
            {
                ErrorMessage errmsg = new ErrorMessage();
                errmsg.errorDescription.Content = "Authenticate Failure";
                errmsg.errorContext.Text = "Framework denied authentication request.";
                errmsg.errorAction.Content = "Cancel";
                errmsg.ShowDialog();
            }
        }

        private async void KickButton_Click(object sender, RoutedEventArgs e)
        {
            await Task.Run(() => api.SetLog("kick "+selectedPlayer));
            selectedPlayer = "";
        }

        private async void BanButton_Click(object sender, RoutedEventArgs e)
        {
            await Task.Run(() => api.SetLog("ban " + selectedPlayer));
            selectedPlayer = "";
        }

        private async void FrameworkRestartButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("Overriding...");

            await Task.Run(() => api.RestartFramework());

            ApplicationWide_StartLoad("Shutting Down");
        }

        private async void FrameworkKillButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("Killing...");

            await Task.Run(() => api.KillAllConnections());
            await Task.Run(() => Thread.Sleep(1000));

            ApplicationWide_StartLoad("Shutting Down");
        }

        private async void FrameworkShutdownButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("Stopping...");

            await Task.Run(() => api.ShutdownFramework());

            ApplicationWide_StartLoad("Shutting Down");
        }

        private async void ServerRebootButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("Rebooting...");

            await Task.Run(() => api.RebootSystem());

            ApplicationWide_StartLoad("Shutting Down");
        }

        private async void ServerShutdownButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("Stopping...");

            await Task.Run(() => api.RebootSystem());

            ApplicationWide_StartLoad("Shutting Down");
        }

        private async void RebuildButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("PLEASE WAIT");

            await Task.Run(() => api.PortPrepare());
            await Task.Run(() => Thread.Sleep(1000));

            ApplicationWide_EndLoad();
        }

        private void lightmodebtn_MouseDown(object sender, RoutedEventArgs e)
        {
                lightmodebtn.IsEnabled = false;
                darkmodebtn.IsEnabled = true;
                user_darkmode = false;
        }

        private void darkmodebtn_MouseDown(object sender, RoutedEventArgs e)
        {
                darkmodebtn.IsEnabled = false;
                lightmodebtn.IsEnabled = true;
                user_darkmode = true;
        }

        private void legacybtn_MouseDown(object sender, RoutedEventArgs e)
        {
                legacybtn.IsEnabled = false;
                gen2btn.IsEnabled = true;
                user_legacyapi = true;
        }

        private void gen2btn_MouseDown(object sender, RoutedEventArgs e)
        {
                gen2btn.IsEnabled = false;
                legacybtn.IsEnabled = true;
                user_legacyapi = false;
        }

        private async void SaveSettingsButton_Click(object sender, RoutedEventArgs e)
        {
            ApplicationWide_StartLoad("Saving...");
            try
            {
                Properties.Settings.Default.ip = frameworkip.Text;
                Properties.Settings.Default.port = int.Parse(frameworkport.Text);
                Properties.Settings.Default.pollfrequency = int.Parse(pollrate.Text);
                Properties.Settings.Default.consolecutoff = int.Parse(consolecutoff.Text);
                Properties.Settings.Default.terminalcutoff = int.Parse(terminalcutoff.Text);
                Properties.Settings.Default.legacysupport = user_legacyapi;
                await Task.Run(() => Thread.Sleep(500));
                Properties.Settings.Default.theme = (user_darkmode == true ? 1 : 0);

                Properties.Settings.Default.Save();
            } catch {
                await Task.Run(() => Thread.Sleep(500));
                ErrorMessage errnotif = new ErrorMessage();
                errnotif.errorDescription.Content = "Parse Error";
                errnotif.errorContext.Text = "Failed to parse new settings. Please verify all values.";
                errnotif.errorAction.Content = "Cancel";
                errnotif.ShowDialog();
                ApplicationWide_EndLoad();
                return;
            }

            await Task.Run(() => Thread.Sleep(500));
            
            ApplicationWide_EndLoad();
            ErrorMessage notif = new ErrorMessage();
            notif.errorDescription.Content = "Saved";
            notif.errorContext.Text = "Swirve may need to be restarted for some changes to take effect.";
            notif.errorAction.Content = "OK";
            notif.ShowDialog();

        }

        private void RestoreSettingsButton_Click(object sender, RoutedEventArgs e)
        {
            frameworkip.Text = Settings.Default.ip;
            frameworkport.Text = Settings.Default.port.ToString();
            pollrate.Text = Settings.Default.pollfrequency.ToString();
            consolecutoff.Text = Settings.Default.consolecutoff.ToString();
            terminalcutoff.Text = Settings.Default.terminalcutoff.ToString();
            switch(Properties.Settings.Default.legacysupport)
            {
                case false:
                    gen2btn.IsEnabled = false;
                    legacybtn.IsEnabled = true;
                    user_legacyapi = false;
                    break;

                case true:
                    gen2btn.IsEnabled = true;
                    legacybtn.IsEnabled = false;
                    user_legacyapi = true;
                    break;
            }
            switch(Properties.Settings.Default.theme)
            {
                case 0:
                    lightmodebtn.IsEnabled = false;
                    darkmodebtn.IsEnabled = true;
                    user_darkmode = false;
                    break;

                case 1:
                    lightmodebtn.IsEnabled = false;
                    darkmodebtn.IsEnabled = true;
                    user_darkmode = true;
                    break;

                default:
                    break;
            }
        }

        public class ServerVisual
        {
            public string ServerName { get; set; }
            public Brush ServerState { get; set; }
            public ulong ServerID { get; set; }
            public string IconPath { get; set; }
        }

        List<FrameworkApi.ServerModule> modulecache = new List<FrameworkApi.ServerModule>();
        ObservableCollection<ServerVisual> visualcache = new ObservableCollection<ServerVisual>();

        public void UploadModules(List<FrameworkApi.ServerModule> servermodules)
        {
            modulecache.Clear();
            modulecache = new List<FrameworkApi.ServerModule>(servermodules.ToArray());
        }

        public void RenderModules()
        {
            rootSelection_serverlist.Visibility = Visibility.Hidden;
            visualcache.Clear();
            rootSelection_serverlist.ItemsSource = visualcache;
            int index = 0;
            foreach (FrameworkApi.ServerModule module in modulecache)
            {
                String iconPath;
                if (module.EPWRState == 0) // Offline
                {
                    iconPath = "./Resources/icon_server_dark.png";
                }
                else if (module.EPWRState == 1 || module.EPWRState == 3 || module.EPWRState == 4 || module.EPWRState == 5) // Starting, stopping, restarting, killing (transition phase)
                {
                    iconPath = "./Resources/icon_server_rebooting_dark.png";
                }
                else if (module.EPWRState == 2) // Online
                {
                    iconPath = "./Resources/icon_server_online_dark.png";
                }
                else // Error ?
                {
                    iconPath = "./Resources/icon_server_error_dark.png";
                }
                visualcache.Add(new ServerVisual() { ServerName = module.Name, ServerID = module.ID, IconPath = iconPath });
                index++;
            }

            // Add "new server" button
            visualcache.Add(new ServerVisual() { ServerName = "New node...", ServerID = 0, IconPath = "./Resources/icon_add_dark.png" });


            // Show
            rootSelection_serverlist.ItemsSource = visualcache;
            rootSelection_serverlist.InvalidateVisual();
            rootSelection_serverlist.Visibility = Visibility.Visible;
        }

        private async void Rectangle_MouseDown(object sender, MouseButtonEventArgs e)
        {
            // foreach (ServerVisual visual in rootSelection_serverlist.Items)
            // {
            //      ContentPresenter borderCP = (ContentPresenter)rootSelection_serverlist.ItemContainerGenerator.ContainerFromItem(visual);
            //     Border border = VisualTreeHelper.GetChild(borderCP, 0) as Border;
            //     border.Background = (Brush)FindResource("Color_Palette1");
            // }
            // (sender as Border).Background = (Brush)FindResource("Color_Palette2");
            ulong serverid = (ulong)(sender as FrameworkElement).Tag;
            selectedID = serverid;

            rootTabControl.SelectedIndex = 0;

            Console.WriteLine($"ID = {selectedID}");

            if (selectedID == 0) // Create a new server
            {
                ServerCreationWizard creationWizard = new ServerCreationWizard();
                creationWizard.GiveAPI(ref api);
                creationWizard.ShowDialog();
                if (creationWizard.serverCreated)
                {
                    selectedID = creationWizard.serverId;
                }
                else
                {
                    if (!Settings.Default.legacysupport) Application.Current.Shutdown();
                    serverArchive.Name = "LEGACY SERVER";
                }
            }
            TotalModules = modulecache.Count;
            OnlineModules = modulecache.Count(module => module.EPWRState == 2);




            // Assign module
            await Task.Run(() => Thread.Sleep(300));
            await Task.Run(() => { api.SetModule(selectedID); });

            rootStart_Progressbar.SetPercent(50);

            // Fetch all neccesary information
            await Task.Run(() => { serverArchive = api.GetArchive(selectedID); });
            rootStart_Progressbar.SetPercent(60);
            await Task.Run(() => { api.GetSwiss(); });
            rootStart_Progressbar.SetPercent(70);
            await Task.Run(() => api.GetLog());
            rootStart_Progressbar.SetPercent(80);

            // Set all neccessary information
            Overview_Servername.Content = serverArchive.Name;
            Overview_ServerIp.Text = Properties.Settings.Default.ip;
            Overview_ServerPort.Text = serverArchive.AssignedPort.ToString();
            Overview_ServerRamTotal.Content = "💾 " + serverArchive.RamAllocated + "GB";
            Overview_ServerJava.Content = "⚙ Java " + serverArchive.JavaVersion;

            // Clear graph
            SeriesCollection[0].Values.Clear();
            SeriesCollection[1].Values.Clear();
            SeriesCollection[2].Values.Clear();
            GraphTimes.Clear();

            // Update config page
            List<string> credentials = api.GetCredentials();
            ftp_ip.Text = credentials[0];
            ftp_port.Text = credentials[1];
            ftp_username.Text = credentials[2];
            ftp_password.Text = credentials[3];
            ftp_id.Text = serverArchive.ID.ToString();
            Configuration_servername.Text = serverArchive.Name;
            Configuration_serverram.Text = serverArchive.RamAllocated.ToString() + "GB";
            Configuration_serverpath.Text = serverArchive.LaunchPath;
            switch (serverArchive.JavaVersion)
            {
                case 8:
                    java16btn.IsEnabled = true;
                    java17btn.IsEnabled = true;
                    java18btn.IsEnabled = true;
                    java8btn.IsEnabled = false;
                    user_serverjava = 8;
                    break;

                case 16:
                    java16btn.IsEnabled = false;
                    java17btn.IsEnabled = true;
                    java18btn.IsEnabled = true;
                    java8btn.IsEnabled = true;
                    user_serverjava = 16;
                    break;

                case 17:
                    java16btn.IsEnabled = true;
                    java17btn.IsEnabled = false;
                    java18btn.IsEnabled = true;
                    java8btn.IsEnabled = true;
                    user_serverjava = 17;
                    break;

                case 18:
                    java16btn.IsEnabled = true;
                    java17btn.IsEnabled = true;
                    java18btn.IsEnabled = false;
                    java8btn.IsEnabled = true;
                    user_serverjava = 18;
                    break;
            }

            frameworkip.Text = Settings.Default.ip;
            frameworkport.Text = Settings.Default.port.ToString();
            pollrate.Text = Settings.Default.pollfrequency.ToString();
            consolecutoff.Text = Settings.Default.consolecutoff.ToString();
            terminalcutoff.Text = Settings.Default.terminalcutoff.ToString();
            switch (Properties.Settings.Default.legacysupport)
            {
                case false:
                    gen2btn.IsEnabled = false;
                    legacybtn.IsEnabled = true;
                    user_legacyapi = false;
                    break;

                case true:
                    gen2btn.IsEnabled = true;
                    legacybtn.IsEnabled = false;
                    user_legacyapi = true;
                    break;
            }
            switch (Properties.Settings.Default.theme)
            {
                case 0:
                    lightmodebtn.IsEnabled = false;
                    darkmodebtn.IsEnabled = true;
                    user_darkmode = false;
                    break;

                case 1:
                    lightmodebtn.IsEnabled = false;
                    darkmodebtn.IsEnabled = true;
                    user_darkmode = true;
                    break;

                default:
                    break;
            }

            // Start updating timer
            refreshTimer.Tick -= new EventHandler(refresh);
            refreshTimer.Tick += new EventHandler(refresh);
            refreshTimer.Interval = new TimeSpan(0, 0, 2);
            refreshTimer.Start();

            // Show main window
            this.mainGrid.Visibility = Visibility.Visible;
            button_dashboard.IsEnabled = false;
            button_terminal.IsEnabled = true;
            button_players.IsEnabled = true;
            button_performance.IsEnabled = true;
            button_configuration.IsEnabled = true;
            button_admin.IsEnabled = true;
            button_settings.IsEnabled = true;
            this.ChangeTab(0);
            rootStart_Progressbar.Value = 100;

            await Task.Run(() => Thread.Sleep(500));

            rootTabControl.SelectedIndex = 2;
        }

        public ulong selectedID;

    }
}
