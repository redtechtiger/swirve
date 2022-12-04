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

namespace Swirve_Userclient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Hide();
            initScreen _splash = new initScreen();
            _splash.Show();
            _splash.workDescriptor.Content = "Please wait...";
            //_splash.workDescriptor.Content = "Launching application...";
            await Task.Delay(5000);
            this.mainGrid.Visibility = Visibility.Visible;
            this.mainTabControl.SelectedIndex = 1;

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
            Close();
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
    }
}
