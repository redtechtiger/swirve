/* Copyright (c) 1998-2008 RedTechTiger Media */

using System;
using System.Collections.Generic;
using System.ComponentModel;
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
    /// 

    public partial class App : Application
    {

        bool isDark;

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            if(e.Args.Length==4 && e.Args[0] == "overwrite")
            {
                Swirve_Userclient.Properties.Settings.Default.ip = e.Args[1];
                Swirve_Userclient.Properties.Settings.Default.port = int.Parse(e.Args[2]);
                Swirve_Userclient.Properties.Settings.Default.Save();
                ErrorMessage debugMsg = new ErrorMessage();
                debugMsg.errorDescription.Content = "IP Port overwrite [OK]";
                debugMsg.errorAction.Content = "Shut down";
                debugMsg.ShowDialog();
                Current.Shutdown();
            }

            isDark = Swirve_Userclient.Properties.Settings.Default.theme == 1;

            if(isDark)
            {
                this.Resources.MergedDictionaries.Clear();
                ResourceDictionary darkMode = new ResourceDictionary();
                darkMode.Source = new Uri("Themes/Dark.xaml", UriKind.RelativeOrAbsolute);
                this.Resources.MergedDictionaries.Add(darkMode);
            }

            Swirve_Userclient.Properties.Settings.Default.PropertyChanged += myEventHandler;

            MainWindow mw = new MainWindow();
            mw.Show();
        }

        void myEventHandler(object s, PropertyChangedEventArgs e)
        {
            if (e.PropertyName != "theme") return;

            Console.WriteLine("Theme has changed!");

            isDark = Swirve_Userclient.Properties.Settings.Default.theme == 1;
            if (isDark)
            {
                Console.WriteLine("Theme is dark");
                this.Resources.MergedDictionaries.Clear();
                ResourceDictionary darkMode = new ResourceDictionary();
                darkMode.Source = new Uri("Themes/Dark.xaml", UriKind.RelativeOrAbsolute);
                this.Resources.MergedDictionaries.Add(darkMode);
            } else
            {
                Console.WriteLine("Theme is light");
                this.Resources.MergedDictionaries.Clear();
                ResourceDictionary darkMode = new ResourceDictionary();
                darkMode.Source = new Uri("Themes/Light.xaml", UriKind.RelativeOrAbsolute);
                this.Resources.MergedDictionaries.Add(darkMode);
            }
        }
    }
}
