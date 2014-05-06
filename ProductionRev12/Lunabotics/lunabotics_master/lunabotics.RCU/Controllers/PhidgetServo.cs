using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Phidgets;
using System.Threading;
using lunabotics.Configuration;

namespace lunabotics.RCU.Controllers
{
    public class PhidgetServo : IController
    {
        private Servo servo_controller;
        private PhidgetServoConfiguration configuration;

        private Dictionary<PhidgetServoChannel, PhidgetServoOutputSettings> channel_map;

        private Thread update_thread;
        private volatile bool isActive = false;
        private CancellationTokenSource tokenSource;
        private Utility.UpdateQueue<Dictionary<Devices, int>> queue;

        public PhidgetServo(PhidgetServoConfiguration config)
        {
            if (config == null)
                throw new ArgumentNullException("config");

            configuration = config;
            ParseConfiguration();
            queue = new Utility.UpdateQueue<Dictionary<Devices, int>>();
            servo_controller = new Servo();
        }

        private void ParseConfiguration()
        {
            channel_map = new Dictionary<PhidgetServoChannel, PhidgetServoOutputSettings>();

            foreach (var setting in configuration.OutputSettings)
                channel_map.Add(setting.Channel, setting);
        }

        public bool IsActive
        {
            get { return isActive; }
        }

        public void Activate()
        {
            if (isActive)
                throw new Exception("Aready activated");

            tokenSource = new CancellationTokenSource();
            if (!servo_controller.Attached) //connect to controller
            {
                if (configuration.ID == -1)
                    servo_controller.open();
                else
                    servo_controller.open(configuration.ID);
                servo_controller.waitForAttachment(3000);
            }

            for (int i = 0; i < servo_controller.servos.Count; i++)
            {
                servo_controller.servos[i].Engaged = false;
                servo_controller.servos[i].Type = ServoServo.ServoType.RAW_us_MODE;
                //initialize
                if (channel_map.ContainsKey((PhidgetServoChannel)i))
                {
                    servo_controller.servos[i].Position = channel_map[(PhidgetServoChannel)i].NeutralValue;
                    servo_controller.servos[i].Engaged = true;
                }
            }


            try
            {
                update_thread = new Thread(new ThreadStart(UpdateWorker));
                update_thread.IsBackground = true;
                isActive = true;
                update_thread.Start();
            }
            catch (Exception ex)
            {
                CleanupUpdateWorker();
                isActive = false;
            }
        }

        public void Deactivate()
        {
            if (!isActive)
                return;

            //shutdown worker thread
            CleanupUpdateWorker();
            isActive = false;
        }

        private void CleanupUpdateWorker()
        {
            tokenSource.Cancel();
            if (update_thread != null && update_thread.ThreadState != ThreadState.Unstarted)
            {
                update_thread.Join();
                update_thread = null;
            }
        }

        public void SetValues(Dictionary<Devices, int> device_mapping)
        {
            //post update to queue
            queue.Enqueue(device_mapping);
        }

        private void UpdateWorker()
        {
            while (!tokenSource.Token.IsCancellationRequested)
            {
                try
                { //todo : doesn't check state to see if it contains necessary device info!!
                    Dictionary<Devices, int> state = queue.Dequeue(tokenSource.Token);

                    for (int i = 0; i < servo_controller.servos.Count; i++)
                    {
                        if (channel_map.ContainsKey((PhidgetServoChannel)i))
                        {
                            if (state.ContainsKey(channel_map[(PhidgetServoChannel)i].Device))
                            {
                                //calculate value...
                                var pwm_map = channel_map[(PhidgetServoChannel)i].PWM_Map;

                                double scalar = (double)(pwm_map.PWM_High - pwm_map.PWM_Low) / (double)(pwm_map.Real_High - pwm_map.Real_Low);

                                int position = Convert.ToInt32(pwm_map.PWM_Low + (state[channel_map[(PhidgetServoChannel)i].Device] - pwm_map.Real_Low) * scalar);
                                servo_controller.servos[i].Position = position;
                            }
                        }
                    }
                }
                catch (TimeoutException ex)
                {
                    Console.WriteLine("Phidget servo controller timeout");
                }
                catch (PhidgetException ex)
                {
                    Console.WriteLine("Phidget Exception " + ex.Code + ":" + ex.Message);
                }
                catch (OperationCanceledException ex)
                {
                    Console.WriteLine("Phidget: " + ex.Message);
                    break;
                }
                catch (Exception ex)
                {
                    Console.WriteLine("General Exception:" + ex.Message);
                }
            }
        }

    }
}
