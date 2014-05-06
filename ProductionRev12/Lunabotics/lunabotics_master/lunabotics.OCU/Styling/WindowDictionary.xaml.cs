using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace lunabotics.OCU.Styling
{
    /// <summary>
    /// Interaction logic for WindowDictionary.xaml
    /// </summary>
    public partial class WindowDictionary : ResourceDictionary
    {
        public WindowDictionary()
        {
            InitializeComponent();
        }
        
        private void MoveWindow(object sender, MouseButtonEventArgs args)
        {
            var window = ((FrameworkElement)sender).TemplatedParent as Window;

            if (window != null)
                window.DragMove();
        }
    }
}
