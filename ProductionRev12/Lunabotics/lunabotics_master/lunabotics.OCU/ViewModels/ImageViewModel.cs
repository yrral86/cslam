using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media.Imaging;
using System.ComponentModel;
using System.Windows.Input;

namespace lunabotics.OCU.ViewModels
{
    public abstract class ImageViewModel : Base.BaseINotifyPropertyChanged
    {

        protected Models.JPEGReceiver cameraReceiver;
        protected BitmapSource image;
        protected volatile int quality;
        protected volatile int frameRate;
        protected bool isActive = false;

        protected void cameraReceiver_ImageReceived(object sender, Models.ImageReceivedEventArgs e)
        {
            CameraFeed = e.Image;
        }

        public BitmapSource CameraFeed
        {
            get { return image; }
            private set
            {
                if (image != value)
                {
                    image = value;
                    OnPropertyChanged("CameraFeed");
                }
            }
        }

        public void Activate()
        {
            if (IsActive)
                return;

            SetupCamera();
            cameraReceiver.Activate();
            IsActive = true;

        }

        public void DeActivate()
        {
            if (!IsActive)
                return;

            cameraReceiver.ImageReceived -= cameraReceiver_ImageReceived;
            cameraReceiver.Deactivate();
            cameraReceiver.Dispose();
            cameraReceiver = null;
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

        public abstract int Port { get; }

        public abstract string Name { get; }

        public int Quality
        {
            get { return quality; }
            set
            {
                if (quality != value)
                {
                    quality = value;
                    OnPropertyChanged("Quality");
                }
            }
        }

        public int FrameRate
        {
            get { return frameRate; }
            set
            {
                if (frameRate != value)
                {
                    frameRate = value;
                    OnPropertyChanged("FrameRate");
                }
            }
        }

        internal string Quality_As_String
        {
            get { return quality.ToString(); }
            set
            {
                if (!quality.ToString().Equals(value))
                {
                    quality = Int32.Parse(value);
                    OnPropertyChanged("Quality_As_String");
                }
            }

        }

        internal string FrameRate_As_String
        {
            get { return frameRate.ToString(); }
            set
            {
                if (!frameRate.ToString().Equals(value))
                {
                    frameRate = Int32.Parse(value);
                    OnPropertyChanged("FrameRate_As_String");
                }
            }
        }

        protected void SetupCamera()
        {
            cameraReceiver = new Models.JPEGReceiver(Port, 0);
            cameraReceiver.ImageReceived += new EventHandler<Models.ImageReceivedEventArgs>(cameraReceiver_ImageReceived);
        }

        public ICommand ToggleCamera
        {
            get
            {
                return new Commands.GenericCommand(p => DoToggle());
            }
        }
        private void DoToggle()
        {
            try
            {
                if (this.IsActive)
                {
                    this.DeActivate();
                }
                else
                {
                    this.Activate();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("ImageViewModel: " + ex.Message);
            }
        }
    }
}
