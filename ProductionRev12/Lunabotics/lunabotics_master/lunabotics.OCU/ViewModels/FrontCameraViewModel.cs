using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.OCU.ViewModels
{
    public sealed class FrontCameraViewModel : CameraViewModel
    {
        private static readonly object instanceSync = new object();
        private static FrontCameraViewModel instance;


        private FrontCameraViewModel()
        {
            FrameRate = Properties.CameraSettings.Default.FrontFrameRate;
            Quality = Properties.CameraSettings.Default.FrontQuality;
        }

        public static FrontCameraViewModel Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (instanceSync)
                    {
                        if (instance == null)
                            instance = new FrontCameraViewModel();
                    }
                }
                return instance;
            }
        }

        public override string Name
        {
            get { return "Front Camera"; }
        }

        public override int Port
        {
            get { return Properties.CameraSettings.Default.FrontReceivePort; }
        }
    }
}
