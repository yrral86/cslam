using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.Specialized;
using System.Collections.ObjectModel;
using Microsoft.Xna.Framework;

namespace lunabotics.OCU.ViewModels
{
    public sealed class ControllerSettingsViewModel : SettingsViewModelBase
    {
        private static readonly object instanceSync = new object();
        private static ControllerSettingsViewModel instance;
        private static ObservableCollection<Models.Point> speedPoints;

        private ControllerSettingsViewModel()
        {
            speedPoints = new ObservableCollection<Models.Point>();
            for (double i = 0; i < 1.01; i += .04)
            {
                speedPoints.Add(new Models.Point(i, Models.ControllerInput.GetRelativeSpeed(i)));
            }
        }

        public static ControllerSettingsViewModel Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (instanceSync)
                    {
                        if (instance == null)
                            instance = new ControllerSettingsViewModel();
                    }
                }
                return instance;
            }
        }

        List<PlayerIndex> players;
        public List<PlayerIndex> Players
        {
            get
            {
                if (players == null)
                {
                    players = new List<PlayerIndex>();
                    players.Add(PlayerIndex.One);
                    players.Add(PlayerIndex.Two);
                    players.Add(PlayerIndex.Three);
                    players.Add(PlayerIndex.Four);
                }
                return players;
            }
        }
        public PlayerIndex SelectedPlayer
        {
            get { return lunabotics.OCU.Properties.ControllerSettings.Default.Player;}
            set
            {
                if (lunabotics.OCU.Properties.ControllerSettings.Default.Player != value)
                {
                    lunabotics.OCU.Properties.ControllerSettings.Default.Player = value;
                    OnPropertyChanged("SelectedPlayer");
                }
            }
        }

        public int SpeedSensitivity
        {
            get { return Properties.ControllerSettings.Default.SpeedSensitivity; }
            set
            {
                if (value == Properties.ControllerSettings.Default.SpeedSensitivity)
                    return;

                Properties.ControllerSettings.Default.SpeedSensitivity = value;
                OnPropertyChanged("SpeedSensitivity");

                //update chart points..
                int index = 0;
                for (double i = 0; i < 1.01; i += .04, ++index)
                {
                    speedPoints[index].X = i;
                    speedPoints[index].Y = Models.ControllerInput.GetRelativeSpeed(i);
                }
            }
        }

        public ObservableCollection<Models.Point> SpeedGraphData
        {
            get
            {
                return speedPoints;
            }
        }

        public override void Save()
        {
            Properties.ControllerSettings.Default.Save();
        }

    }
}
