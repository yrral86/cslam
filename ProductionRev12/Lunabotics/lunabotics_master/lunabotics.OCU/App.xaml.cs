using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;
using System.Drawing;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace lunabotics.OCU
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            MainWindow window = new MainWindow();
            window.DataContext = new ViewModels.MainWindowViewModel();
            Application.Current.MainWindow = window;
            Application.Current.MainWindow.Show();
        }
    }
}
