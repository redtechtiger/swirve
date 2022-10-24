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
            MainWindow mainWindow = new MainWindow();
            await Task.Delay(500);


            //Connect (somehow) to backend
            workDescriptor.Content = "Waiting for backend...";
            await Task.Delay(300);


            //Fetch MySQL Data
            workDescriptor.Content = "Connecting to database...";
            await Task.Delay(1000);


            //Connect to server client
            workDescriptor.Content = "Connecting to server...";
            await Task.Delay(800);


            //Initialize actual application
            workDescriptor.Content = "Initializing application...";
            await Task.Delay(1000);


            Close();
        }
    }
}
