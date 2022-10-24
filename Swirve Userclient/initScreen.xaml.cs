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
using System.Runtime.InteropServices;
using System.Windows.Interop;
using Swirve_Userclient.Acrylic;

namespace Swirve_Userclient
{
    /// <summary>
    /// Interaction logic for initScreen.xaml
    /// </summary>
    public partial class initScreen : Window
    {
        public initScreen()
        {
            InitializeComponent();
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            
            //BlurEffect
            BlurEffect blurEffect = new BlurEffect(this);
            blurEffect.EnableBlur(this);

            MainWindow mainWindow = new MainWindow();

            Version _version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
            DateTime _buildDate = new DateTime(2000, 1, 1).AddDays(_version.Build).AddSeconds(_version.Revision * 2);
            versionNumber.Content = $"Devbuild {_version}, Build date: {_buildDate}";



            await Task.Delay(500);


            //Connect (somehow) to backend
            workDescriptor.Content = "Waiting for backend...";
            await Task.Delay(100);


            //Fetch MySQL Data
            workDescriptor.Content = "Connecting to database...";
            await Task.Delay(100);


            //Connect to server client
            workDescriptor.Content = "Connecting to server...";
            await Task.Delay(100);


            //Initialize actual application
            workDescriptor.Content = "Initializing application...";
            await Task.Delay(100);



            workDescriptor.Content = "";
            await Task.Delay(2000);
            Close();
        }

        private void Grid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                this.DragMove();
            }
        }
    }
}
