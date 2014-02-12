using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.OCU.Properties;

namespace lunabotics.OCU.ViewModels
{
    public sealed class SettingsViewModel : SettingsViewModelBase
    {
        private static readonly object instanceSync = new object();
        private static SettingsViewModel instance;

        private SettingsViewModel()
        {
        }

        public static SettingsViewModel Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (instanceSync)
                    {
                        if (instance == null)
                            instance = new SettingsViewModel();
                    }
                }
                return instance;
            }
        }

        public string MaxVelocity
        {
            get { return Settings.Default.MaxVelocity.ToString(); }
            set 
            {
                if (!Settings.Default.MaxVelocity.ToString().Equals(value))
                {
                    Settings.Default.MaxVelocity = Int32.Parse(value);
                    OnPropertyChanged("MaxVelocity");
                }
            }
        }

        public string RobotIPAddress
        {
            get { return Settings.Default.RobotIPAddress; }
            set
            {
             if (Settings.Default.RobotIPAddress != value)
                {
                    Settings.Default.RobotIPAddress = value;
                    OnPropertyChanged("RobotIPAddress");
                }
            }
        }

        public string CommandPort
        {
            get { return Settings.Default.CommandPort.ToString(); }
            set
            {
                if (!Settings.Default.CommandPort.ToString().Equals(value))
                {
                    Settings.Default.CommandPort = Int32.Parse(value);
                    OnPropertyChanged("CommandPort");
                }
            }
        }

        public string OutputInterval
        {
            get { return Settings.Default.OutputInterval.ToString(); }
            set
            {
                if (!Settings.Default.OutputInterval.ToString().Equals(value))
                {
                    Settings.Default.OutputInterval = Int32.Parse(value);
                    OnPropertyChanged("OutputInterval");
                }
            }
        }

        public override void Save()
        {
            Settings.Default.Save();
        }
    }
}
