using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.OCU.ViewModels
{
    public sealed class RearCameraViewModel : CameraViewModel
    {
        private static readonly object instanceSync = new object();
        private static RearCameraViewModel instance;


        private RearCameraViewModel()
        {
            FrameRate = Properties.CameraSettings.Default.RearFrameRate;
            Quality = Properties.CameraSettings.Default.RearQuality;
        }

        public static RearCameraViewModel Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (instanceSync)
                    {
                        if (instance == null)
                            instance = new RearCameraViewModel();
                    }
                }
                return instance;
            }
        }

        public override string Name
        {
            get { return "Rear Camera"; }
        }

        public override int Port
        {
            get { return Properties.CameraSettings.Default.RearReceivePort; }
        }
    }
}
