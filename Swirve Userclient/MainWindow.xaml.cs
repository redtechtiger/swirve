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
        private Button selectedButton;
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {

        }

        private void exitButton_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void Grid_Loaded(object sender, RoutedEventArgs e)
        {
        }

        private void Rectangle_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if(e.ChangedButton == MouseButton.Left) {
                DragMove();
            }
        }

        private void minimiseButton_Click(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            
        }

        //private void Window_Loaded(object sender, RoutedEventArgs e)
        //{
        //}

        private void set50_Click(object sender, RoutedEventArgs e)
        {
            testbar.Value = 50;
        }

        private void set100_Click(object sender, RoutedEventArgs e)
        {
            testbar.Value = 100;
        }

        private void ListViewScrollViewer_PreviewMouseWheel(object sender, System.Windows.Input.MouseWheelEventArgs e)
        {
            ScrollViewer scv = (ScrollViewer)sender;
            scv.ScrollToVerticalOffset(scv.VerticalOffset - e.Delta / 2);
            e.Handled = true;
        }

        private void overviewButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 0;
            selectedButton.IsEnabled = true;
            selectedButton = overviewButton;
            updatePageSelectedMarker();
        }

        private void performanceButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 1;
            selectedButton.IsEnabled = true;
            selectedButton = performanceButton;
            updatePageSelectedMarker();
        }

        private void playersButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 2;
            selectedButton.IsEnabled = true;
            selectedButton = playersButton;
            updatePageSelectedMarker();
        }

        private void configurationButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 3;
            selectedButton.IsEnabled = true;
            selectedButton = configurationButton;
            updatePageSelectedMarker();
        }

        private void consoleButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 4;
            selectedButton.IsEnabled = true;
            selectedButton = consoleButton;
            updatePageSelectedMarker();
        }

        private void ftpButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 5;
            selectedButton.IsEnabled = true;
            selectedButton = ftpButton;
            updatePageSelectedMarker();
        }

        private void updatePageSelectedMarker()
        {
            if (selectedButton == null) return;
            var transform = selectedButton.TransformToVisual(mainGrid as FrameworkElement);
            Point absolutePosition = transform.Transform(new Point(0, 0));
            double avgX = ((absolutePosition.X + selectedButton.ActualWidth) + (absolutePosition.X - pageSelectedMarker.ActualWidth)) / 2;
            pageSelectedMarker.Margin = new Thickness((int)avgX, absolutePosition.Y-1, 0, 0);

            selectedButton.IsEnabled = false;
            

        }

        private void Window_LayoutUpdated(object sender, EventArgs e)
        {
            updatePageSelectedMarker();
        }

        private async void Window_ContentRendered(object sender, EventArgs e)
        {
            Hide();
            initScreen _splash = new initScreen();
            _splash.Show();
            _splash.workDescriptor.Content = "Please wait...";
            await Task.Delay(5000);
            _splash.Close();

            pageView.SelectedIndex = 0;

            Show();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            selectedButton = overviewButton;
        }
    }
}
