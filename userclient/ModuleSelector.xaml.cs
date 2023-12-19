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
using System.Windows.Shapes;

namespace Swirve_Userclient
{
    /// <summary>
    /// Interaction logic for moduleSelector.xaml
    /// </summary>
    public partial class ModuleSelector : Window
    {

        List<FrameworkApi.ServerModule> modulecache = new List<FrameworkApi.ServerModule>();
        List<ServerVisual> visualcache = new List<ServerVisual>();

        public ulong selectedID;
        int pageNumber; 
        public ModuleSelector()
        {
            InitializeComponent();
        }

        public void UploadModules(List<FrameworkApi.ServerModule> servermodules)
        {
            modulecache = new List<FrameworkApi.ServerModule>(servermodules.ToArray());
        }

        public void RenderModules()
        {
            ServerList.Visibility = Visibility.Hidden;
            visualcache.Clear();
            ServerList.ItemsSource = visualcache;
            int index = 0;
            foreach(FrameworkApi.ServerModule module in modulecache)
            {
                Brush serverState;
                if(module.EPWRState==0 || module.EPWRState==6) // Offline or crashed
                {
                    serverState = Brushes.Red;
                } else if(module.EPWRState==1 || module.EPWRState==3 || module.EPWRState==4 || module.EPWRState==5 ) // Starting, stopping, restarting, killing (transition phase)
                {
                    serverState = Brushes.Orange;
                } else if(module.EPWRState==2) // Online
                {
                    serverState = Brushes.Green;
                } else // Error ?
                {
                    serverState = Brushes.Purple;
                }
                visualcache.Add(new ServerVisual() { ServerName=module.Name, ServerState = serverState, ServerID = module.ID, IconPath = "./Resources/icon_server.png" });
                index++;
            }
            // Add "new server" button

            visualcache.Add(new ServerVisual() { ServerName = "New Server", ServerState = Brushes.Transparent, ServerID = 0, IconPath = "./Resources/icon_add.png" });


            // Calculate page
            ServerList.ItemsSource = visualcache.Skip(pageNumber*4).Take(4);

            selector_page_number.Content = pageNumber + 1;

            ServerList.Visibility = Visibility.Visible;
        }

        private void Rectangle_MouseDown(object sender, MouseButtonEventArgs e)
        {
            foreach(ServerVisual visual in ServerList.Items)
            {
                ContentPresenter borderCP = (ContentPresenter)ServerList.ItemContainerGenerator.ContainerFromItem(visual);
                Border border = VisualTreeHelper.GetChild(borderCP, 0) as Border;
                border.Background = (Brush)FindResource("Color_Palette1");
            }
            (sender as Border).Background = (Brush)FindResource("Color_Palette2");
            ulong serverid = (ulong) (sender as FrameworkElement).Tag;
            selectedID = serverid;
            connect_btn.IsEnabled = true;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void Border_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if(e.ChangedButton == MouseButton.Left)
            {
               this.DragMove();
            }
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            if(pageNumber>0)
            {
                pageNumber--;
                RenderModules();
            }
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            if ((pageNumber+1)*4 < visualcache.Count())
            {
                pageNumber++;
                RenderModules();
            }
        }
    }

    public class ServerVisual
    {
        public string ServerName { get; set; }
        public Brush ServerState { get; set; }
        public ulong ServerID { get; set; }
        public string IconPath { get; set; }
    }
}
