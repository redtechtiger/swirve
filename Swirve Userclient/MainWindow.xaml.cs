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

        private void Button_Click(object sender, RoutedEventArgs e)
        {

        }

        private void exitButton_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private async void Grid_Loaded(object sender, RoutedEventArgs e)
        {
            Hide();
            initScreen _splash = new initScreen();
            _splash.ShowDialog();
            Show();
            pageView.SelectedIndex = 0;
            await Task.Run(() => System.Threading.Thread.Sleep(5000));
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

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Loaded!");
        }

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
            movePageSelectedMarker(overviewButton);
        }

        private void performanceButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 1;
            movePageSelectedMarker(performanceButton);
        }

        private void playersButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 2;
            movePageSelectedMarker(playersButton);
        }

        private void configurationButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 3;
            movePageSelectedMarker(configurationButton);
        }

        private void consoleButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 4;
            movePageSelectedMarker(consoleButton);
        }

        private void ftpButton_Click(object sender, RoutedEventArgs e)
        {
            pageView.SelectedIndex = 5;
            movePageSelectedMarker(ftpButton);
        }

        private void movePageSelectedMarker(Button button)
        {
            var transform = button.TransformToVisual(mainGrid as FrameworkElement);
            Point absolutePosition = transform.Transform(new Point(0, 0));
            double avgX = ((absolutePosition.X + button.ActualWidth) + (absolutePosition.X - pageSelectedMarker.ActualWidth)) / 2;

            pageSelectedMarker.Margin = new Thickness((int)avgX, absolutePosition.Y, 0, 0);
        }
    }
}
