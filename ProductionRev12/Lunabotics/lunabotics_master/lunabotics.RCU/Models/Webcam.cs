using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using AForge.Video;
using AForge.Video.DirectShow;
using System.Net;
using System.Net.Sockets;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Diagnostics;
using System.Timers;
using lunabotics.Configuration;

namespace lunabotics.RCU.Models
{

    public class Webcam
    {
        private VideoCaptureDevice videoSource;
        private Comms.UDP_Sender udp_sender;
        private ImageCodecInfo jpegCodec;
        private volatile bool isActive;
        System.Threading.CancellationTokenSource tokenSource;
        private Utility.UpdateQueue<byte[]> raw_frame_queue;

        private CameraConfiguration config;

        private volatile int quality;
        private volatile int frameRate;

        private Timer updateWorker;
        private readonly object workerSync = new object();

        public Webcam(string camera, IPAddress ipAddress, int port, int frame_rate = 5)
        {
            FilterInfoCollection videoDevices = new FilterInfoCollection(FilterCategory.VideoInputDevice);
            if (videoDevices.Count < 1)
                throw new Exception("No video input devices available");

            if (!string.IsNullOrEmpty(camera))
                videoSource = new VideoCaptureDevice(camera);
            else
            {
                videoSource = new VideoCaptureDevice(videoDevices[0].MonikerString);
            }

            updateWorker = new Timer();
            updateWorker.Elapsed += new ElapsedEventHandler(updateWorker_Elapsed);
            FrameRate = frame_rate;
            JPEGQuality = 10;
            isActive = false;
            jpegCodec = GetEncoderInfo("image/jpeg");
            videoSource.DesiredFrameSize = new Size(320, 240);
            videoSource.NewFrame += new NewFrameEventHandler(videoSource_NewFrame);
            udp_sender = new Comms.UDP_Sender(ipAddress, port);
            raw_frame_queue = new Utility.UpdateQueue<byte[]>(-1);
        }

        public Webcam(string ocu_ipAddress, CameraConfiguration config)
        {
            this.config = config;
            FilterInfoCollection videoDevices = new FilterInfoCollection(FilterCategory.VideoInputDevice);
            if (videoDevices.Count < 1)
                throw new Exception("No video input devices available");
            if (!string.IsNullOrEmpty(config.DeviceName))
                videoSource = new VideoCaptureDevice(config.DeviceName);
                //videoSource = new VideoCaptureDevice(videoDevices[1].MonikerString);
            else
            {
                videoSource = new VideoCaptureDevice(videoDevices[0].MonikerString);
            }

            updateWorker = new Timer();
            updateWorker.Elapsed += new ElapsedEventHandler(updateWorker_Elapsed);
            FrameRate = config.RCU_To_OCU_Framerate;
            JPEGQuality = config.JPEGQuality;
            isActive = false;
            jpegCodec = GetEncoderInfo("image/jpeg");
            videoSource.DesiredFrameSize = new Size(320, 240);
            videoSource.NewFrame += new NewFrameEventHandler(videoSource_NewFrame);
            udp_sender = new Comms.UDP_Sender(ocu_ipAddress, config.SendPort);
            raw_frame_queue = new Utility.UpdateQueue<byte[]>(-1);
        }

        public int FrameRate
        {
            get { return frameRate; }
            set
            {
                if (value != frameRate)
                {
                    if (value < 0 || value > 30)
                        throw new ArgumentOutOfRangeException("value", "Framerate must be between 0-30");

                    frameRate = value;
                    videoSource.DesiredFrameRate = Math.Max(1, value);
                    updateWorker.Interval = 1000.0 / Math.Max(1, value);
                }
            }
        }

        //between 0-100
        public int JPEGQuality
        {
            get { return quality; }
            set
            {
                if (value < 0 || value > 100)
                    throw new ArgumentOutOfRangeException("value", "Quality must be between 1-100");

                if (quality != value)
                    quality = value;
            }
        }

        public void UpdateState(lunabotics.Comms.States.VideoState state)
        {
            FrameRate = state.FrameRate;
            JPEGQuality = state.Quality * 5;
        }

        private void videoSource_NewFrame(object sender, NewFrameEventArgs eventArgs)
        {
            try
            {
                if (frameRate == 0) //return if framerate is zero
                    return;

                Bitmap frame = eventArgs.Frame;
                EncoderParameter qualityParameter = new EncoderParameter(System.Drawing.Imaging.Encoder.Quality, quality);
                EncoderParameters parameters = new EncoderParameters(1);
                parameters.Param[0] = qualityParameter;
                //todo : might be better initialize stream to an appropriate size?
                using (MemoryStream stream = new MemoryStream())
                {
                    frame.Save(stream, jpegCodec, parameters);
                    raw_frame_queue.Enqueue(stream.ToArray());
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }

        private ImageCodecInfo GetEncoderInfo(string mimeType)
        {
            ImageCodecInfo[] codecs = ImageCodecInfo.GetImageEncoders();
            return codecs.First(codec => (codec.MimeType == mimeType));
        }

        public bool IsActive
        {
            get { return isActive; }
        }

        public void Activate()
        {
            if (isActive)
                throw new Exception("Already activated");
            tokenSource = new System.Threading.CancellationTokenSource();
            try
            {
                updateWorker.Start();
                videoSource.Start();
                isActive = true;
            }
            catch (Exception ex)
            {
                Internal_Deactivate();
                throw;
            }
        }

        public void Deactivate()
        {
            if (!isActive)
                return;

            Internal_Deactivate();
        }

        private void Internal_Deactivate()
        {
            //disable timer
            updateWorker.Enabled = false;
            tokenSource.Cancel();
            isActive = false;

            if (videoSource != null && videoSource.IsRunning)
            {
                videoSource.SignalToStop();
                videoSource.WaitForStop();
            }
        }

        void updateWorker_Elapsed(object sender, ElapsedEventArgs e)
        {
            //using a monitor so event overlap is not an issue
            if (System.Threading.Monitor.TryEnter(workerSync))
            {
                try
                {
                    byte[] frame = raw_frame_queue.Dequeue(tokenSource.Token);
                    udp_sender.Send(frame);
                }
                catch (Comms.DataTooLargeException ex)
                {
                    Console.WriteLine(ex.Message);
                    //todo : try to trim size down? decrease image size/quality?
                }
                catch (OperationCanceledException ex)
                {
                    Console.WriteLine("Webcam: " + ex.Message);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                    // todo: log!
                }
                finally
                {
                    System.Threading.Monitor.Exit(workerSync);
                }
            }
        }

    }
}
