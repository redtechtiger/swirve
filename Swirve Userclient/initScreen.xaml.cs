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

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {

            Version _version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
            DateTime _buildDate = new DateTime(2000, 1, 1).AddDays(_version.Build).AddSeconds(_version.Revision * 2);
            versionNumber.Content = $"Devbuild {_version}, Build date: {_buildDate}";
            workDescriptor.Visibility = Visibility.Visible;

        }

        private void Grid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                DragMove();
            }
        }

        private void Swirve_Splash_MediaEnded(object sender, RoutedEventArgs e)
        {
            workDescriptor.Visibility = Visibility.Visible;
        }
    }
}
