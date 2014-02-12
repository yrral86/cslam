using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Comms;
using lunabotics.Comms.TelemetryEncoding;

namespace lunabotics.OCU.Models
{
    public class TelemetryReceivedEventArgs : EventArgs
    {
        private Comms.TelemetryEncoding.TelemetryFeedback telemetry;

        public TelemetryReceivedEventArgs(TelemetryFeedback telemetry)
        {
            if(telemetry == null)
                throw new ArgumentNullException("telemetry");

            this.telemetry = telemetry;
        }

        public TelemetryFeedback Telemetry
        {
            get { return telemetry;}
        }
    }

    public class TelemetryReceiver : IDisposable
    {
        private Comms.UDP_Receiver receiver;
        private bool isdisposed = false;

        public TelemetryReceiver(int port, int opt_timeout = 1000)
        {
            receiver = new Comms.UDP_Receiver(port, opt_timeout);
        }

        ~TelemetryReceiver()
        {
            Dispose(false);
        }

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
            receiver.DataReceived += new EventHandler<DataArgs>(receiver_DataReceived);
            receiver.Activate();
        }

        public void Deactivate()
        {
            receiver.DataReceived -= receiver_DataReceived;
            receiver.Deactivate();
        }

        void receiver_DataReceived(object sender, DataArgs e)
        {
            Console.WriteLine("dataReceived");
            try
            {
                TelemetryFeedback feedback = TelemetryFeedback.Decode(e.Data);

                Console.WriteLine(feedback.PowerUsed);

                if(TelemetryReceived != null)
                    TelemetryReceived(this, new TelemetryReceivedEventArgs(TelemetryFeedback.Decode(e.Data)));
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }
        
        public event EventHandler<TelemetryReceivedEventArgs> TelemetryReceived;
    }
}
