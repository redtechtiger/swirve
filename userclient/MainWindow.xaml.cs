﻿/* Copyright (c) 1998-2008 RedTechTiger Media */

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

namespace Swirve_Userclient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>

    public partial class MainWindow : Window
    {
        string[] PowerString = new string[] { "Offline", "Starting", "Online", "Stopping", "Restarting", "Killing", "Fault" };

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
                    Title = "CPU",
                    Values = new ChartValues<double> { }
                },
                new LineSeries
                {
                    Title = "VmSize",
                    Values = new ChartValues<double> { }
                }
                ,
                new LineSeries
                {
                    Title = "VmRSS",
                    Values = new ChartValues<double> { }
                }
            };

            GraphTimes = new List<string>();
            YDataFormatter = value => value.ToString() + "%";

            DataContext = this;
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Hide(); // Show splash first

            ErrorMessage errMsg = new ErrorMessage(); // Windows to be used
            initScreen _splash = new initScreen();

            _splash.Show(); // Show splash

            // Fetch connection prerequisites
            string ip = Properties.Settings.Default.ip;
            int port = Properties.Settings.Default.port;

            // Connect
            await Task.Run(() => api.Connect(ip, port));
            if (!api.connected)
            {

                // If failed, show this error
                errMsg.errorDescription.Content = "Connection Error";
                errMsg.errorContext.Text = "Failed to connect to Framework.";
                errMsg.errorAction.Content = "Shut Down";
                errMsg.ShowDialog();

                // Shut down Userclient
                _splash.Close();
                Close();
                Application.Current.Shutdown();
                return;
            }

            // Fetch all servers
            List<FrameworkApi.ServerModule> modules = new List<FrameworkApi.ServerModule>();
            await Task.Run(() => { modules = api.GetModules(); });

            // User selects server
            ulong selectedModuleID = getUserServer(modules);

            // Assign module
            await Task.Run(() => Thread.Sleep(1000));
            await Task.Run(() => { api.SetModule(selectedModuleID); });

            // Fetch all neccesary information
            await Task.Run(() => { serverArchive = api.GetArchive(selectedModuleID); });
            await Task.Run(() => { api.GetSwiss(); });
            await Task.Run(() => api.GetLog());

            // Set all neccessary information
            Overview_Servername.Content = serverArchive.Name;
            Overview_ServerIp.Content = Properties.Settings.Default.ip;
            Overview_ServerPort.Content = serverArchive.AssignedPort;
            Overview_ServerRamTotal.Content = serverArchive.RamAllocated + "GB";
            Overview_ServerJava.Content = "Java " + serverArchive.JavaVersion;

            // Clear graph
            SeriesCollection[0].Values.Clear();
            SeriesCollection[1].Values.Clear();
            SeriesCollection[2].Values.Clear();
            GraphTimes.Clear();

            // Update config page
            List<string> credentials = api.GetCredentials();
            ftp_ip.Content = credentials[0];
            ftp_port.Content = credentials[1];
            ftp_username.Content = credentials[2];
            ftp_password.Content = credentials[3];
            ftp_id.Content = serverArchive.ID;
            Configuration_servername.Text = serverArchive.Name;
            Configuration_serverram.Text = serverArchive.RamAllocated.ToString();
            Configuration_serverpath.Text = serverArchive.LaunchPath;
            switch(serverArchive.JavaVersion)
            {
                case 8:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_Highlight1");
                    user_serverjava = 8;
                    break;

                case 16:
                    java16btn.Background = (Brush)FindResource("Color_Highlight1");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
                    user_serverjava = 16;
                    break;

                case 17:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_Highlight1");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
                    user_serverjava = 17;
                    break;

                case 18:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_Highlight1");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
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
                    gen2btn.Background = (Brush)FindResource("Color_Highlight1");
                    legacybtn.Background = (Brush)FindResource("Color_TextInverted");
                    user_legacyapi = false;
                    break;

                case true:
                    darkmodebtn.Background = (Brush)FindResource("Color_Highlight1");
                    lightmodebtn.Background = (Brush)FindResource("Color_TextInverted");
                    user_legacyapi = true;
                    break;
            }
            switch (Properties.Settings.Default.theme)
            {
                case 0:
                    lightmodebtn.Background = (Brush)FindResource("Color_Highlight1");
                    darkmodebtn.Background = (Brush)FindResource("Color_TextInverted");
                    user_darkmode = false;
                    break;

                case 1:
                    darkmodebtn.Background = (Brush)FindResource("Color_Highlight1");
                    lightmodebtn.Background = (Brush)FindResource("Color_TextInverted");
                    user_darkmode = true;
                    break;

                default:
                    break;
            }

            // Start updating timer
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
            _splash.Close();
            Show();
        }

        private ulong getUserServer(List<FrameworkApi.ServerModule> modules)
        {
            ModuleSelector moduleSelector = new ModuleSelector();
            moduleSelector.UploadModules(modules);
            moduleSelector.RenderModules();

            moduleSelector.ShowDialog();
            ulong selectedModuleID = moduleSelector.selectedID;
            Console.WriteLine($"ID = {selectedModuleID}");

            if (selectedModuleID == 0) // Create a new server
            {
                ServerCreationWizard creationWizard = new ServerCreationWizard();
                creationWizard.GiveAPI(ref api);
                creationWizard.ShowDialog();
                if (creationWizard.serverCreated)
                {
                    selectedModuleID = creationWizard.serverId;
                }
                else
                {
                    if(!Settings.Default.legacysupport) Application.Current.Shutdown();
                    serverArchive.Name = "LEGACY SERVER";
                }
            }
            TotalModules = modules.Count;
            OnlineModules = modules.Count(module => module.EPWRState == 2);
            return selectedModuleID;
        }

        private void stopuserclient()
        {
            lock(powerdownlock)
            {
                refreshTimer.Stop();
                ErrorMessage errMsg = new ErrorMessage();
                errMsg.errorDescription.Content = "Disconnected";
                errMsg.errorContext.Text = "Framework remotely shut down the connection";
                errMsg.errorAction.Content = "Power Down";
                errMsg.ShowDialog();
                Hide();
                initScreen loadingscreen = new initScreen();
                loadingscreen.Show();
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
                        Overview_CpuText.Content = "CPU " + dataresponse.ServerCPU.ToString() + "%";
                        Overview_RamProgressbar.Maximum = serverArchive.RamAllocated * 1024; // From GB to MB
                        Overview_RamProgressbar.Value = dataresponse.ServerVmRSS;
                        Overview_RamText.Content = "RAM " + Math.Round((double)dataresponse.ServerVmRSS / 1024, 1).ToString() + "GB";
                        Overview_PlayersOnline.Content = api.GetCachedPlayerCount();

                        Overview_Console.Text = dataresponse.ServerLog.Substring(dataresponse.ServerLog.Length - Math.Min(Settings.Default.consolecutoff, dataresponse.ServerLog.Length));
                        Overview_Console.ScrollToEnd();

                        Performance_TotalRam.Content = "RAM " + Math.Round((double)dataresponse.SystemTotalMemory / 1024, 1).ToString() + " GB";
                        Performance_TotalCores.Content = "CORES " + dataresponse.SystemTotalCores.ToString();
                        Performance_SysCpuProgressbar.Value = dataresponse.SystemCPU;
                        Performance_SysCpuText.Content = "CPU " + dataresponse.SystemCPU.ToString() + "%";
                        Performance_SysRamProgressbar.Value = dataresponse.SystemMemory;
                        Performance_SysRamProgressbar.Maximum = dataresponse.SystemTotalMemory;
                        Performance_SysRamText.Content = "RAM " + Math.Round((double)dataresponse.SystemMemory / 1024, 1) + "GB";
                        Performance_CpuProgressbar.Value = dataresponse.ServerCPU;
                        Performance_CpuText.Content = "CPU " + dataresponse.ServerCPU.ToString() + "%";
                        Performance_VmSizeProgressbar.Value = dataresponse.ServerVmSize;
                        Performance_VmSizeProgressbar.Maximum = serverArchive.RamAllocated * 1024;
                        Performance_VmSizeText.Content = "VmSize " + Math.Round((double)dataresponse.ServerVmSize / 1024, 1).ToString() + "GB";
                        Performance_VmRssProgressbar.Value = dataresponse.ServerVmRSS;
                        Performance_VmRssProgressbar.Maximum = serverArchive.RamAllocated * 1024;
                        Performance_VmRssText.Content = "VmRSS " + Math.Round((double)dataresponse.ServerVmRSS / 1024, 1) + "GB";

                        Performance_ModulesLoaded.Content = "Loaded Modules: " + TotalModules;
                        Performance_ModulesOnline.Content = "Online Modules: " + OnlineModules;

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
            // Hide window
            Hide();
            initScreen _splash = new initScreen();
            _splash.Show();

            // Stop refresher
            refreshTimer.Stop();

            // Fetch all servers
            List<FrameworkApi.ServerModule> modules = new List<FrameworkApi.ServerModule>();
            await Task.Run(() => { modules = api.GetModules(); });

            // User selects server
            ulong selectedModuleID = getUserServer(modules);

            // Assign module
            await Task.Run(() => { api.SetModule(selectedModuleID); });

            // Fetch all neccesary information
            await Task.Run(() => { serverArchive = api.GetArchive(selectedModuleID); });
            await Task.Run(() => { api.GetSwiss(); });
            await Task.Run(() => refresh(this, new EventArgs()));

            // Set all neccessary information
            Overview_Servername.Content = serverArchive.Name;
            Overview_ServerIp.Content = Properties.Settings.Default.ip;
            Overview_ServerPort.Content = serverArchive.AssignedPort;
            Overview_ServerRamTotal.Content = serverArchive.RamAllocated + "GB";
            Overview_ServerJava.Content = "Java " + serverArchive.JavaVersion;

            // Clear graph
            SeriesCollection[0].Values.Clear();
            SeriesCollection[1].Values.Clear();
            SeriesCollection[2].Values.Clear();
            GraphTimes.Clear();

            // Update config page
            List<string> credentials = api.GetCredentials();
            ftp_ip.Content = credentials[0];
            ftp_port.Content = credentials[1];
            ftp_username.Content = credentials[2];
            ftp_password.Content = credentials[3];
            ftp_id.Content = serverArchive.ID;
            Configuration_servername.Text = serverArchive.Name;
            Configuration_serverram.Text = serverArchive.RamAllocated.ToString();
            Configuration_serverpath.Text = serverArchive.LaunchPath;
            switch (serverArchive.JavaVersion)
            {
                case 8:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_Highlight1");
                    user_serverjava = 8;
                    break;

                case 16:
                    java16btn.Background = (Brush)FindResource("Color_Highlight1");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
                    user_serverjava = 16;
                    break;

                case 17:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_Highlight1");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
                    user_serverjava = 17;
                    break;

                case 18:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_Highlight1");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
                    user_serverjava = 18;
                    break;
            }

            // Start refresher
            refreshTimer.Start();

            // Show main window
            await Task.Run(() => Thread.Sleep(200));
            await Task.Run(() => Thread.Sleep(200));
            this.mainGrid.Visibility = Visibility.Visible;
            button_dashboard.IsEnabled = false;
            button_terminal.IsEnabled = true;
            button_players.IsEnabled = true;
            button_performance.IsEnabled = true;
            button_configuration.IsEnabled = true;
            button_admin.IsEnabled = true;
            button_settings.IsEnabled = true;
            this.ChangeTab(0);
            _splash.Close();
            Show();
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

        private void java8btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                java16btn.Background = (Brush)FindResource("Color_TextInverted");
                java17btn.Background = (Brush)FindResource("Color_TextInverted");
                java18btn.Background = (Brush)FindResource("Color_TextInverted");
                java8btn.Background = (Brush)FindResource("Color_Highlight1");
                user_serverjava = 8;
            }
        }

        private void java16btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                java16btn.Background = (Brush)FindResource("Color_Highlight1");
                java17btn.Background = (Brush)FindResource("Color_TextInverted");
                java18btn.Background = (Brush)FindResource("Color_TextInverted");
                java8btn.Background = (Brush)FindResource("Color_TextInverted");
                user_serverjava = 16;
            }
        }

        private void java17btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                java16btn.Background = (Brush)FindResource("Color_TextInverted");
                java17btn.Background = (Brush)FindResource("Color_Highlight1");
                java18btn.Background = (Brush)FindResource("Color_TextInverted");
                java8btn.Background = (Brush)FindResource("Color_TextInverted");
                user_serverjava = 17;
            }
        }

        private void java18btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                java16btn.Background = (Brush)FindResource("Color_TextInverted");
                java17btn.Background = (Brush)FindResource("Color_TextInverted");
                java18btn.Background = (Brush)FindResource("Color_Highlight1");
                java8btn.Background = (Brush)FindResource("Color_TextInverted");
                user_serverjava = 18;
            }
        }

        private void DiscardButton_Click(object sender, RoutedEventArgs e)
        {
            // Update config page
            List<string> credentials = api.GetCredentials();
            ftp_ip.Content = credentials[0];
            ftp_port.Content = credentials[1];
            ftp_username.Content = credentials[2];
            ftp_password.Content = credentials[3];
            ftp_id.Content = serverArchive.ID;
            Configuration_servername.Text = serverArchive.Name;
            Configuration_serverram.Text = serverArchive.RamAllocated.ToString();
            Configuration_serverpath.Text = serverArchive.LaunchPath;
            switch (serverArchive.JavaVersion)
            {
                case 8:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_Highlight1");
                    user_serverjava = 18;
                    break;

                case 16:
                    java16btn.Background = (Brush)FindResource("Color_Highlight1");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
                    user_serverjava = 16;
                    break;

                case 17:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_Highlight1");
                    java18btn.Background = (Brush)FindResource("Color_TextInverted");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
                    user_serverjava = 17;
                    break;

                case 18:
                    java16btn.Background = (Brush)FindResource("Color_TextInverted");
                    java17btn.Background = (Brush)FindResource("Color_TextInverted");
                    java18btn.Background = (Brush)FindResource("Color_Highlight1");
                    java8btn.Background = (Brush)FindResource("Color_TextInverted");
                    user_serverjava = 18;
                    break;
            }
        }

        private async void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            // Try to parse data
            int ram;
            try {
                ram = int.Parse(Configuration_serverram.Text);
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
            Overview_ServerPort.Content = serverArchive.AssignedPort;
            Overview_ServerRamTotal.Content = serverArchive.RamAllocated + "GB";
            Overview_ServerJava.Content = "Java " + serverArchive.JavaVersion;
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

        private void lightmodebtn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                lightmodebtn.Background = (Brush)FindResource("Color_Highlight1");
                darkmodebtn.Background = (Brush)FindResource("Color_TextInverted");
                user_darkmode = false;
            }
        }

        private void darkmodebtn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                darkmodebtn.Background = (Brush)FindResource("Color_Highlight1");
                lightmodebtn.Background = (Brush)FindResource("Color_TextInverted");
                user_darkmode = true;
            }
        }

        private void legacybtn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                legacybtn.Background = (Brush)FindResource("Color_Highlight1");
                gen2btn.Background = (Brush)FindResource("Color_TextInverted");
                user_legacyapi = true;
            }
        }

        private void gen2btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                gen2btn.Background = (Brush)FindResource("Color_Highlight1");
                legacybtn.Background = (Brush)FindResource("Color_TextInverted");
                user_legacyapi = false;
            }
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
                    gen2btn.Background = (Brush)FindResource("Color_Highlight1");
                    legacybtn.Background = (Brush)FindResource("Color_TextInverted");
                    user_legacyapi = false;
                    break;

                case true:
                    darkmodebtn.Background = (Brush)FindResource("Color_Highlight1");
                    lightmodebtn.Background = (Brush)FindResource("Color_TextInverted");
                    user_legacyapi = true;
                    break;
            }
            switch(Properties.Settings.Default.theme)
            {
                case 0:
                    lightmodebtn.Background = (Brush)FindResource("Color_Highlight1");
                    darkmodebtn.Background = (Brush)FindResource("Color_TextInverted");
                    user_darkmode = false;
                    break;

                case 1:
                    darkmodebtn.Background = (Brush)FindResource("Color_Highlight1");
                    lightmodebtn.Background = (Brush)FindResource("Color_TextInverted");
                    user_darkmode = true;
                    break;

                default:
                    break;
            }
        }
    }
}