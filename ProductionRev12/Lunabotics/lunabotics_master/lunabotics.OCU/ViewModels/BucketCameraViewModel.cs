using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.OCU.ViewModels
{
    public sealed class BucketCameraViewModel : CameraViewModel
    {
        private static readonly object instanceSync = new object();
        private static BucketCameraViewModel instance;

        private BucketCameraViewModel()
        {
            FrameRate = Properties.CameraSettings.Default.BucketCameraFrameRate;
            Quality = Properties.CameraSettings.Default.BucketCameraQuality;
        }

        public static BucketCameraViewModel Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (instanceSync)
                    {
                        if (instance == null)
                            instance = new BucketCameraViewModel();
                    }
                }
                return instance;
            }
        }

        public override string Name
        {
            get { return "Bucket Camera"; }
        }

        public override int Port
        {
            get { return Properties.CameraSettings.Default.BucketCameraPort; }
        }

    }
}
