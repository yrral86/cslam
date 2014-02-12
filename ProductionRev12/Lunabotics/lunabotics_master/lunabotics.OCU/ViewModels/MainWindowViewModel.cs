using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace lunabotics.OCU.ViewModels
{
    public class MainWindowViewModel : Base.BaseINotifyPropertyChanged
    {
        private Models.Communications comms;

        public MainWindowViewModel()
        {
            comms = Models.Communications.Instance;
            comms.EnabledChanged += new EventHandler(comms_EnabledChanged);
        }

        void comms_EnabledChanged(object sender, EventArgs e)
        {
            OnPropertyChanged("CommsEnabled");
        }

        internal void DoShutdown()
        {
            comms.DisableOutput();
            FrontCamera.DeActivate();            
            RearCamera.DeActivate();
            BucketCamera.DeActivate();
            Telemetry.DeActivate();
        }

        public bool CommsEnabled
        {
            get
            {
                return comms.IsEnabled;
            }
        }

        #region Commands

        private ICommand shutdown;
        public ICommand Shutdown
        {
            get
            {
                if (shutdown == null)
                    shutdown = new Commands.GenericCommand(p => DoShutdown());

                return shutdown;
            }
        }

        private ICommand openSettings;
        public ICommand OpenSettings
        {
            get
            {
                if (openSettings == null)
                    openSettings = new Commands.GenericCommand(p => DoOpenSettings(), p => CanOpenSettings());

                return openSettings;
            }
        }
        private void DoOpenSettings()
        {
            Windows.SettingsWindow win = new Windows.SettingsWindow();
            win.Show();
        }
        private bool CanOpenSettings()
        {
            if (App.Current.Windows.OfType<Windows.SettingsWindow>().Count() > 0)
                return false;

            return true;
        }

        
        public ICommand ToggleComms
        {
            get
            {
                return new Commands.GenericCommand(p => DoToggleComms());
            }
        }
        private void DoToggleComms()
        {
            try
            {
                if (comms.IsEnabled)
                {
                    comms.DisableOutput();
                }
                else
                {
                    comms.StartOutput();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("MainWindowViewModel: " + ex.Message);
            }
        }

#endregion

        public FrontCameraViewModel FrontCamera
        {
            get { return FrontCameraViewModel.Instance; }
        }

        public RearCameraViewModel RearCamera
        {
            get { return RearCameraViewModel.Instance; }
        }

        public BucketCameraViewModel BucketCamera
        {
            get { return BucketCameraViewModel.Instance; }
        }

        public TelemetryViewModel Telemetry
        {
            get { return ViewModels.TelemetryViewModel.Instance; }
        }

    }
}
