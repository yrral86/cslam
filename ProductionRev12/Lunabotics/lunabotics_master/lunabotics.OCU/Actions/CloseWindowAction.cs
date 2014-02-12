using System;
using System.Windows;
using System.Windows.Interactivity;
using System.Windows.Media.Animation;

namespace lunabotics.OCU.Actions
{
    public class CloseWindowAction : TriggerAction<DependencyObject>
    {
        protected override void Invoke(object o)
        {
            Window toClose = Window.GetWindow(AssociatedObject);
            toClose.Close();
        }
    }
}
