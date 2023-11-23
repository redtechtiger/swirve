using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.RightsManagement;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace Swirve_Userclient
{
    /// <summary>
    /// Interaction logic for ServerCreationWizard.xaml
    /// </summary>
    public partial class ServerCreationWizard : Window
    {
        public bool serverCreated = false;
        public ulong serverId = 0;
        private FrameworkApi frameworkapi;

        int user_serverram;
        int user_serverjava = 8;
        string user_servername;
        string user_launchpath;

        public ServerCreationWizard()
        {
            InitializeComponent();
        }

        private async void init_continue_Click(object sender, RoutedEventArgs e)
        {
            bool validinfo = true;
            try // Makes sure RAM is formatted correctly
            {
                user_serverram = int.Parse(init_serverram.Text);
            } catch
            {
                Error_Verify.Content = "PLEASE VERIFY SERVER RAM";
                validinfo = false;
            }

            if (init_servername.Text == "")
            {
                Error_Verify.Content = "PLEASE VERIFY SERVER NAME";
                validinfo = false;
            }

            if (!validinfo) return;

            init_serverram.BorderThickness = new Thickness(0);
            init_servername.BorderThickness = new Thickness(0);

            user_servername = init_servername.Text;
            main_tabcontrol.SelectedIndex = 1;
            List<string> credentials = new List<string>();


            // Create the actual server!
            await Task.Run(() =>
            {
                serverId = frameworkapi.CreateServer(user_servername, user_serverram);
                credentials = frameworkapi.GetCredentials();
            });

            if(serverId>0) // Success
            {
                ftp_ip.Content = credentials[0];
                ftp_port.Content = credentials[1];
                ftp_username.Content = credentials[2];
                ftp_password.Content = credentials[3];
                ftp_id.Content = serverId;
                main_tabcontrol.SelectedIndex = 2;
            } else
            {
                Console.WriteLine("Failed to create server");
                ErrorMessage errmsg = new ErrorMessage();
                errmsg.errorDescription.Content = "API Denied";
                errmsg.errorContext.Text = "Framework denied the creation of a new server.";
                errmsg.errorAction.Content = "Shut Down";
                errmsg.ShowDialog();
                Close();
            }
        }

        private void init_quit_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        public void GiveAPI(ref FrameworkApi api)
        {
            frameworkapi = api;
            main_tabcontrol.SelectedIndex = 0;
        }

        private void Border_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if(e.ChangedButton == MouseButton.Left)
            {
                DragMove();
            }
        }

        private void Grid_Loaded(object sender, RoutedEventArgs e)
        {
            // main_tabcontrol.SelectedIndex = 1;
        }

        private void ftp_continue_Click(object sender, RoutedEventArgs e)
        {
            main_tabcontrol.SelectedIndex = 3;
        }

        private async void launch_continue_Click(object sender, RoutedEventArgs e)
        {
            if (launch_path.Text == "")
            {
                launch_path.BorderThickness = new Thickness(1);
                launch_path.BorderBrush = Brushes.Red;
                return;
            }
            user_launchpath = launch_path.Text;
            main_tabcontrol.SelectedIndex = 4;
            bool success = false;
            await Task.Run(() => { // FIRST: Overwrite the launch directory, then initiate port preparation
                success = frameworkapi.OverwriteServer(serverId, user_servername, user_serverram, user_launchpath, user_serverjava);
                if (success) success = frameworkapi.SetModule(serverId) == 0;
                if (success) success = frameworkapi.PortPrepare();
            });

            if (success)
            {
                main_tabcontrol.SelectedIndex = 5;
                serverCreated = true;
            } else
            {
                ErrorMessage errmsg = new ErrorMessage();
                errmsg.errorDescription.Content = "API Denied";
                errmsg.errorContext.Text = "Framework automatic configuration failed.";
                errmsg.errorAction.Content = "Shut Down";
                errmsg.ShowDialog();
                Close();
            }
        }

        private void connect_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void java8btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if(e.ChangedButton == MouseButton.Left)
            {
                java16btn.Background = (Brush)FindResource("Color_Highlight1");
                java17btn.Background = (Brush)FindResource("Color_Highlight1");
                java18btn.Background = (Brush)FindResource("Color_Highlight1");
                java8btn.Background = (Brush)FindResource("Color_Highlight2");
                user_serverjava = 18;
            }
        }

        private void java16btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                java16btn.Background = (Brush)FindResource("Color_Highlight2");
                java17btn.Background = (Brush)FindResource("Color_Highlight1");
                java18btn.Background = (Brush)FindResource("Color_Highlight1");
                java8btn.Background = (Brush)FindResource("Color_Highlight1");
                user_serverjava = 16;
            }
        }

        private void java17btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                java16btn.Background = (Brush)FindResource("Color_Highlight1");
                java17btn.Background = (Brush)FindResource("Color_Highlight2");
                java18btn.Background = (Brush)FindResource("Color_Highlight1");
                java8btn.Background = (Brush)FindResource("Color_Highlight1");
                user_serverjava = 17;
            }
        }

        private void java18btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                java16btn.Background = (Brush)FindResource("Color_Highlight1");
                java17btn.Background = (Brush)FindResource("Color_Highlight1");
                java18btn.Background = (Brush)FindResource("Color_Highlight2");
                java8btn.Background = (Brush)FindResource("Color_Highlight1");
                user_serverjava = 18;
            }
        }
    }
}
