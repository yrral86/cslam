using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.OCU.Properties;
using System.Net;
using lunabotics.Comms;
using System.Timers;
using lunabotics.OCU.ViewModels;

namespace lunabotics.OCU.Models
{
    public sealed class Communications
    {
        private static Communications instance;
        private static readonly object instanceSync = new object();
        private bool isEnabled;

        private FrontCameraViewModel frontCameraVM;
        private RearCameraViewModel rearCameraVM;
        private BucketCameraViewModel bucketVM;

        private lunabotics.Comms.UDP_Sender udpSender;

        private Timer worker;
        private readonly object workerSync = new object();

        public static Communications Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (instanceSync)
                    {
                        if (instance == null)
                            instance = new Communications();
                    }
                }

                return instance;
            }
        }

        private Communications()
        {
            isEnabled = false;
            worker = new Timer();
            worker.Elapsed += new ElapsedEventHandler(worker_Elapsed);
            Properties.Settings.Default.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(Default_PropertyChanged);

            frontCameraVM = FrontCameraViewModel.Instance;
            rearCameraVM = RearCameraViewModel.Instance;
            bucketVM = BucketCameraViewModel.Instance;
        }

        void Default_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("OutputInterval"))
            {
                worker.Interval = Settings.Default.OutputInterval;
                if (udpSender != null)
                    udpSender.Timeout = Settings.Default.OutputInterval;
            }
        }

        public bool IsEnabled
        {
            get { return isEnabled; }
            private set
            {
                if (value != isEnabled)
                {
                    isEnabled = value;
                    OnEnabledChanged();
                }
            }
        }

        public event EventHandler EnabledChanged;

        private void OnEnabledChanged()
        {
            if (EnabledChanged != null)
                EnabledChanged(this, new EventArgs());
        }

        public void StartOutput()
        {
            //todo : exception handling
            if (isEnabled)
                return;

            //set timeout to output interval
            udpSender = new UDP_Sender(Settings.Default.RobotIPAddress, Settings.Default.CommandPort, Settings.Default.OutputInterval);
            worker.Interval = Settings.Default.OutputInterval;
            worker.Enabled = true;
            IsEnabled = true;
        }

        public void DisableOutput()
        {
            if (!isEnabled)
                return;

            worker.Enabled = false;
            IsEnabled = false;
        }

        void worker_Elapsed(object sender, ElapsedEventArgs e)
        {
            //get states and send data to RCU
            if (System.Threading.Monitor.TryEnter(workerSync))
            {
                try
                {
                    Dictionary<Comms.CommandEncoding.CommandFields, short> outputState = Models.ControllerInput.GetLunabotState();
                    
                    //append kinect states
                    outputState[Comms.CommandEncoding.CommandFields.FrontCameraState] = frontCameraVM.GetEncodedState();
                    outputState[Comms.CommandEncoding.CommandFields.RearCameraState] = rearCameraVM.GetEncodedState();
                    outputState[Comms.CommandEncoding.CommandFields.BucketCameraState] = bucketVM.GetEncodedState();
                    byte[] data = Comms.CommandEncoding.Encoder.Encode(outputState);
                    udpSender.Send(data);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
                finally
                {
                    System.Threading.Monitor.Exit(workerSync);
                }
            }
        }
    }
}
