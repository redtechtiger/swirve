using System;
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
                if (console != "-1")
                {

                    logOutput.Text = console;
                    logOutput.ScrollToEnd();
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
            Console.WriteLine("---------- Refresher: Getting API Power ----------");
            int pwr = api.GetPower();
            Console.WriteLine("---------- Refresher: Getting Log ----------");
            console = api.GetLog();
            Console.WriteLine("---------- Refresher: API Work done ----------");
            switch(pwr)
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
                    ErrorMessage errMsg = new ErrorMessage();
                    errMsg.errorDescription.Content = "Warning: Bad PWRSTATE from Framework API";
                    errMsg.ShowDialog();
                    break;
            }
            Console.WriteLine("Power set, done");
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            ErrorMessage errMsg = new ErrorMessage();
            Hide();
            ErrorMessage _errMsg = new ErrorMessage();
            _errMsg.errorDescription.Content = "Warning: This is a devbuild and is strictly\nnot meant for production environments!!!";
            _errMsg.errorAction.Content = "I understand";
            _errMsg.ShowDialog();
            initScreen _splash = new initScreen();
            _splash.Show();
            _splash.workDescriptor.Content = "Preparing API";
            await Task.Run(() => System.Threading.Thread.Sleep(200));
            _splash.workDescriptor.Content = "Connecting to Framework";
            await Task.Run(() => System.Threading.Thread.Sleep(500));
            api.Connect();
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

            _splash.workDescriptor.Content = "Welcome to the Future";
            await Task.Run(() => System.Threading.Thread.Sleep(2000));

            _splash.Close();
            Show();
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
    }
}
