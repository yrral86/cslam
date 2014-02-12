using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows.Media.Imaging;
using System.IO;
using System.Diagnostics;

namespace lunabotics.OCU.Models
{
    public class ImageReceivedEventArgs : EventArgs
    {
        private BitmapSource image;

        public ImageReceivedEventArgs(BitmapSource image)
        {
            if (image == null)
                throw new ArgumentNullException("image");

            this.image = image;
        }

        public BitmapSource Image
        {
            get { return image; }
        }
    }


    public class JPEGReceiver : IDisposable
    {
        private Comms.UDP_Receiver receiver;

        private bool isdisposed = false;

        public JPEGReceiver(int port, int opt_timeout = 1000)
        {
            receiver = new Comms.UDP_Receiver(port, opt_timeout);
        }

        ~JPEGReceiver()
        {
            Dispose(false);
        }

        void receiver_ReceiverError(object sender, Comms.ErrorArgs e)
        {
            if (this.ReceiveError != null)
                this.ReceiveError(this, e);
        }

        void receiver_DataReceived(object sender, Comms.DataArgs e)
        {
            Console.WriteLine("UDP Received");
            // todo : validate against jpeg header/footer
            // header: 255, 216 (0xFF, 0xD8)
            // footer: 255, 217 (0xFF, 0xD9)
            //check header...
            try
            {
                Debug.WriteLine(e.Data.Length);
                if (e.Data[0] == 0xFF && e.Data[1] == 0xD8 && e.Data[e.Data.Length - 2] == 0xFF && e.Data[e.Data.Length - 1] == 0xD9)
                { //image is validated!
                    using (MemoryStream ms = new MemoryStream(e.Data))
                    {
                        BitmapSource source = BitmapFrame.Create(ms, BitmapCreateOptions.None, BitmapCacheOption.OnLoad); //OnLoad option is necessary so stream doesn't go away

                        if (ImageReceived != null)
                            ImageReceived(this, new ImageReceivedEventArgs(source));
                    }
                }
                else
                    Debug.WriteLine("Data did not have correct Jpeg header/footer");
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }

        public bool IsActive
        {
            get { return receiver.IsActive; }
        }

        public int Timeout
        {
            get { return receiver.Timeout; }
            set { receiver.Timeout = value; }
        }

        public void Activate()
        {

            receiver.DataReceived += new EventHandler<Comms.DataArgs>(receiver_DataReceived);
            receiver.ReceiverError += new EventHandler<Comms.ErrorArgs>(receiver_ReceiverError);
            receiver.Activate();
        }

        public void Deactivate()
        {

            receiver.DataReceived -= receiver_DataReceived;
            receiver.ReceiverError -= receiver_ReceiverError;
            receiver.Deactivate();
        }

        public event EventHandler<Comms.ErrorArgs> ReceiveError;

        public event EventHandler<ImageReceivedEventArgs> ImageReceived;

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (!isdisposed)
            {
                if (disposing)
                {
                    receiver.Dispose();
                }
                isdisposed = true;
            }
        }
    }
}
