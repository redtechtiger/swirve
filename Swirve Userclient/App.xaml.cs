using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace Swirve_Userclient
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            if(e.Args.Length==4 && e.Args[0] == "overwrite")
            {
                Swirve_Userclient.Properties.Settings.Default.ip = e.Args[1];
                Swirve_Userclient.Properties.Settings.Default.port = int.Parse(e.Args[2]);
                Swirve_Userclient.Properties.Settings.Default.mcport = int.Parse(e.Args[3]);
                Swirve_Userclient.Properties.Settings.Default.Save();
                ErrorMessage debugMsg = new ErrorMessage();
                debugMsg.errorDescription.Content = "IP Port overwrite [OK]";
                debugMsg.errorAction.Content = "Shut down";
                debugMsg.ShowDialog();
                Current.Shutdown();
            }
            MainWindow mw = new MainWindow();
            mw.Show();
        }
    }
}
