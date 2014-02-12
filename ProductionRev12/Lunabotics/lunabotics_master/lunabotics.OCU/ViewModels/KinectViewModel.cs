using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media.Imaging;
using System.ComponentModel;
using System.Windows.Input;

namespace lunabotics.OCU.ViewModels
{
    public abstract class KinectViewModel : Base.BaseINotifyPropertyChanged
    {
        protected Models.JPEGReceiver colorReceiver, depthReceiver;
        protected BitmapSource colorImage, depthImage;

        protected volatile int colorQuality, depthQuality;
        protected volatile int colorFrameRate, depthFrameRate;

        protected bool isActive;
        protected bool colorEnabled = true, depthEnabled = false;

        public KinectViewModel()
        {
        }

        public BitmapSource ColorFeed
        {
            get { return colorImage; }
            private set
            {
                if (colorImage != value)
                {
                    colorImage = value;
                    OnPropertyChanged("ColorFeed");
                }
            }
        }

        public BitmapSource DepthFeed
        {
            get { return depthImage; }
            private set
            {
                if (depthImage != value)
                {
                    depthImage = value;
                    OnPropertyChanged("DepthFeed");
                }
            }
        }

        public void Activate()
        {
            if (isActive)
                return;

            Setup();
            colorReceiver.Activate();
            depthReceiver.Activate();
            IsActive = true;
        }

        public void Deactivate()
        {
            if (!IsActive)
                return;

            Teardown();
            IsActive = false;
        }

        public bool IsActive
        {
            get { return isActive; }
            private set
            {
                if (isActive != value)
                {
                    isActive = value;
                    OnPropertyChanged("IsActive");
                }
            }
        }

        public bool ColorEnabled
        {
            get { return colorEnabled; }
            set
            {
                if (colorEnabled != value)
                {
                    colorEnabled = value;
                    OnPropertyChanged("ColorEnabled");
                }
            }
        }

        public bool DepthEnabled
        {
            get { return depthEnabled; }
            set
            {
                if (depthEnabled != value)
                {
                    depthEnabled = value;
                    OnPropertyChanged("DepthEnabled");
                }
            }
        }

        public abstract string Name { get; }
        public abstract int DepthPort { get; }
        public abstract int ColorPort { get; }

        public int ColorQuality 
        {
            get { return colorQuality; }
            set
            {
                if (colorQuality != value)
                {
                    colorQuality = value;
                    OnPropertyChanged("ColorQuality");
                }
            }
        }
        public int ColorFrameRate
        {
            get { return colorFrameRate; }
            set
            {
                if (colorFrameRate != value)
                {
                    colorFrameRate = value;
                    OnPropertyChanged("ColorFrameRate");
                }
            }
        }

        public int DepthQuality
        {
            get { return depthQuality; }
            set
            {
                if (depthQuality != value)
                {
                    depthQuality = value;
                    OnPropertyChanged("DepthQuality");
                }
            }
        }
        public int DepthFrameRate
        {
            get { return depthFrameRate; }
            set
            {
                if (depthFrameRate != value)
                {
                    depthFrameRate = value;
                    OnPropertyChanged("DepthFrameRate");
                }
            }
        }

        public short GetEncodedColorState()
        {
            int frameRate = (this.IsActive && this.ColorEnabled ? this.ColorFrameRate : 0);
            return Comms.States.VideoState.EncodeVideoState(this.ColorQuality, frameRate);
        }

        public short GetEncodedDepthState()
        {
            int frameRate = (this.IsActive && this.DepthEnabled ? this.DepthFrameRate : 0);
            return Comms.States.VideoState.EncodeVideoState(this.DepthQuality, frameRate);
        }

        private void Setup()
        {
            colorReceiver = new Models.JPEGReceiver(ColorPort, 0);
            colorReceiver.ImageReceived += new EventHandler<Models.ImageReceivedEventArgs>(colorReceiver_ImageReceived);

            depthReceiver = new Models.JPEGReceiver(DepthPort, 0);
            depthReceiver.ImageReceived += new EventHandler<Models.ImageReceivedEventArgs>(depthReceiver_ImageReceived);
        }

        void depthReceiver_ImageReceived(object sender, Models.ImageReceivedEventArgs e)
        {
            DepthFeed = e.Image;
        }

        void colorReceiver_ImageReceived(object sender, Models.ImageReceivedEventArgs e)
        {
            ColorFeed = e.Image;
        }

        private void Teardown()
        {
            depthReceiver.ImageReceived -= depthReceiver_ImageReceived;
            depthReceiver.Deactivate();
            depthReceiver.Dispose();
            depthReceiver = null;

            colorReceiver.ImageReceived -= colorReceiver_ImageReceived;
            colorReceiver.Deactivate();
            colorReceiver.Dispose();
            colorReceiver = null;

            IsActive = false;
        }

        ICommand toggleKinect;
        public ICommand ToggleKinect
        {
            get
            {
                if(toggleKinect == null)
                    toggleKinect = new Commands.GenericCommand(p => DoToggle());

                return toggleKinect;
            }
        }
        private void DoToggle()
        {
            if (this.IsActive)
            {
                this.Deactivate();
            }
            else
            {
                this.Activate();
            }
        }
    }
}
