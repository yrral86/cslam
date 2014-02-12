using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.Ports;
using System.Threading;
using lunabotics.Configuration;

namespace lunabotics.RCU.Controllers
{
    public class RoboteQ : Telemetry.IProvideTelemetry
    {
        private RoboteqConfiguration configuration;
        private Dictionary<RoboteqChannel, RoboteqOutputSettings> channel_map;
        private SerialPort serial_port;
        private CancellationTokenSource tokenSource;

        private Thread update_thread, monitor_thread;
        private System.Timers.Timer telemetry_requester;
        private System.Timers.Timer watch_dog;
        private volatile bool isActive;
        private readonly object serialPortSync = new object();
        private readonly object updateSync = new object();
        private readonly object watchdogSync = new object();

        private volatile int motor1Value = 0;
        private volatile int motor2Value = 0;

        private int watch_motor1_ignore = 0, watch_motor2_ignore = 0;

        private RoboteQUpdateQueue queue;

        public RoboteQ(RoboteqConfiguration config)
        {
            if (config == null)
                throw new ArgumentNullException("config");

            this.configuration = config;
            isActive = false;
            queue = new RoboteQUpdateQueue(-1);
            this.ParseConfiguration();
            telemetry_requester = new System.Timers.Timer();
            telemetry_requester.Elapsed += new System.Timers.ElapsedEventHandler(telemetry_requester_Elapsed);
            telemetry_requester.Interval = configuration.InputSettings.Interval;
            watch_dog = new System.Timers.Timer();
            watch_dog.Elapsed += new System.Timers.ElapsedEventHandler(watchdog_Elapsed);
            watch_dog.Interval = config.Timeout;
            serial_port = new SerialPort(config.COM_Port, 115200, Parity.None, 8, StopBits.One);
            serial_port.ReadTimeout = 1000;
            serial_port.ErrorReceived += new SerialErrorReceivedEventHandler(serial_port_ErrorReceived);
        }

        void serial_port_ErrorReceived(object sender, SerialErrorReceivedEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine(e.EventType.ToString());
        }

        public bool IsActive
        {
            get { return IsActive; }
        }

        public void Activate()
        {
            if (isActive)
                throw new Exception("Aready activated");

            tokenSource = new CancellationTokenSource();

            if (!serial_port.IsOpen) //ensure port is open
                serial_port.Open();

            serial_port.DiscardInBuffer();
            serial_port.DiscardOutBuffer();
            ////send initialization commands
            serial_port.Write("^ECHOF 1\r"); //disable echo
            //start worker thread
            try
            {
                update_thread = new Thread(new ThreadStart(UpdateWorker));
                update_thread.IsBackground = true;
                monitor_thread = new Thread(new ThreadStart(MonitorWorker));
                monitor_thread.IsBackground = true;
                telemetry_requester.Enabled = true;
                update_thread.Start();
                monitor_thread.Start();

                watch_dog.Enabled = true;
                isActive = true;
            }
            catch (Exception ex)
            {
                serial_port.Close();
                CleanupWorkers();
                isActive = false;
            }

        }

        public void Deactivate()
        {
            if (!isActive)
                return;

            //shutdown worker thread
            CleanupWorkers();
            E_Stop();
            isActive = false;
            if (serial_port != null && serial_port.IsOpen)
            {
                serial_port.Close();
            }
        }

        private void CleanupWorkers()
        {
            telemetry_requester.Enabled = false;
            watch_dog.Enabled = false;
            tokenSource.Cancel();
            if (update_thread != null && update_thread.ThreadState != ThreadState.Unstarted)
            {
                update_thread.Join();
                update_thread = null;
            }
            if (monitor_thread != null && monitor_thread.ThreadState != ThreadState.Unstarted)
            {
                monitor_thread.Join();
                monitor_thread = null;
            }
        }

        public void SetValues(Dictionary<Devices, int> device_mapping)
        {
            //post update to queue
            queue.Enqueue(device_mapping);
        }

        private void ParseConfiguration()
        {
            channel_map = new Dictionary<RoboteqChannel, RoboteqOutputSettings>();
            foreach (var setting in configuration.OutputSettings)
            {
                channel_map.Add(setting.Channel, setting);
            }
        }

        #region Motor operations & watchdogs

        private void UpdateWorker()
        {
            while (!tokenSource.Token.IsCancellationRequested)
            {
                try
                {
                    Dictionary<Devices, int> state = queue.Dequeue(tokenSource.Token);
                    lock (updateSync) //synchronization with watchdog timer
                    {
                        SetMotorValue(state, RoboteqChannel.Motor1);
                        SetMotorValue(state, RoboteqChannel.Motor2);
                        Update();
                    }
                }
                catch (InvalidOperationException ex)
                {
                    //port is not open!
                    //todo : how to handle?
                    Console.WriteLine("SERIAL PORT NOT OPEN: " + this.configuration.COM_Port.ToString());
                }
                catch (OperationCanceledException ex)
                {
                    Console.WriteLine(configuration.Name + ":" + ex.Message);
                    break;
                }
                catch (Exception ex)
                {
                    // log
                }
            }
        }

        private void SetMotorValue(Dictionary<Devices, int> state, RoboteqChannel channel)
        {
            int val;
            RoboteqOutputSettings outputSettings;
            if (channel_map.TryGetValue(channel, out outputSettings)) //check if this roboteq has config for this channel
            {
                if (state.TryGetValue(outputSettings.Device, out val)) //see if the state contains updated value
                {
                    if (outputSettings.Invert)
                        val *= -1;

                    switch (channel) //set the appropriate backing variable, set ignore flags
                    {
                        case RoboteqChannel.Motor1:
                            motor1Value = val;
                            Interlocked.Exchange(ref watch_motor1_ignore, 1);
                            break;
                        case RoboteqChannel.Motor2:
                            motor2Value = val;
                            Interlocked.Exchange(ref watch_motor2_ignore, 1);
                            break;
                        default:
                            break;
                    }
                }
            }
            //if not using motor channel, do nothing
        }

        void watchdog_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            if (Monitor.TryEnter(watchdogSync))
            {
                try
                {
                    int watch1, watch2;

                    watch1 = Interlocked.Exchange(ref watch_motor1_ignore, 0);
                    watch2 = Interlocked.Exchange(ref watch_motor2_ignore, 0);

                    if (watch1 == 0 || watch2 == 0)
                    { //see if either motor has timed out
                        System.Diagnostics.Debug.WriteLine("Timeout on " + this.configuration.Name);
                        lock (updateSync)
                        {
                            //it is possible the update worker has updated the flags, so double check
                            if (watch_motor1_ignore == 0) //if timeout, stop
                            {
                                motor1Value = 0;
                            }
                            if (watch_motor2_ignore == 0)
                            {
                                motor2Value = 0;
                            }
                            if (watch_motor1_ignore == 0 || watch_motor2_ignore == 0)
                            { //send update if either motor has timed out
                                try
                                {
                                    System.Diagnostics.Debug.WriteLine("Motor values: " + motor1Value + "," + motor2Value);
                                    Update();
                                }
                                catch (Exception) { }
                            }
                        }
                    }
                }
                catch (Exception) { }
                finally
                {
                    Monitor.Exit(watchdogSync);
                }
            }
        }
        #endregion

        private void MonitorWorker()
        {
            char[] type_split = new char[]{'=', '\r', '\n'};
            char[] value_separator = new char[]{':'};
            while (!tokenSource.IsCancellationRequested)
            {
                try
                {
                    string result = this.serial_port.ReadTo("\r");

                    //split up any 
                    string[] type_values = result.Split(type_split, StringSplitOptions.RemoveEmptyEntries);

                    if (type_values.Length == 2)
                    { //first index: look for type label match
                      //second index: colon-separated list of values

                        //if (configuration.Name == "RoboteQ_4")
                            //Console.WriteLine(type_values[0] + " " + type_values[1]);


                        foreach (TelemetryMapping tm in configuration.InputSettings.TelemetryMappings)
                        {
                            if(type_values[0].Equals(tm.TypeLabel, StringComparison.CurrentCultureIgnoreCase))
                            { //found type match!
                                //string[] temp = type_values[1].Split(value_separator, StringSplitOptions.RemoveEmptyEntries);
                                //if (configuration.Name == "RoboteQ_4")
                                //    if (tm.TypeLabel == "DI")
                                //        Console.WriteLine(temp.Length);
                                var telemetryDictionary = ParseType(tm, type_values[1].Split(value_separator, StringSplitOptions.RemoveEmptyEntries));
                                if (telemetryDictionary != null && telemetryDictionary.Count > 0)
                                    OnTelemetryUpdated(new Telemetry.TelemetryEventArgs(configuration.InputSettings.Interval,telemetryDictionary));
                                break;
                            }
                        }
                    }
                }
                catch (TimeoutException ex)
                {
                }
                catch (Exception ex)
                {

                }
            }
        }

        private void telemetry_requester_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            try
            {
                RequestTelemetry();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void RequestTelemetry()
        {
            lock (serialPortSync)
            {
                if (!serial_port.IsOpen) //try to recover serial port if it has become disconnected
                    serial_port.Open();

                serial_port.Write(configuration.InputSettings.TelemetryString + "\r");
            }
        }

        private  Dictionary<Configuration.Telemetry, int> ParseType(TelemetryMapping tm, string[] values)
        {
            Dictionary<Configuration.Telemetry, int> dict = new Dictionary<Configuration.Telemetry, int>();
            foreach (TelemetryIndexMapping tim in tm.IndexMappings)
            {
                dict[tim.Telemetry] = Int32.Parse(values[tim.Index]);
            }

            return dict;
        }

        private void E_Stop()
        {
            try
            {
                motor1Value = 0;
                motor2Value = 0;
                Update();
            }
            catch (Exception) { }
        }

        private void Update()
        {
            lock (serialPortSync)
            {
                if (!serial_port.IsOpen) //try to recover serial port if it has become disconnected
                    serial_port.Open();

                //Motor speed write to RoboteQ
                serial_port.Write("!M " + motor1Value.ToString() + " " + motor2Value.ToString() + "\r");
            }
        }

        public event EventHandler<Telemetry.TelemetryEventArgs> TelemetryUpdated;
        private void OnTelemetryUpdated(Telemetry.TelemetryEventArgs args)
        {
            if (TelemetryUpdated != null)
                TelemetryUpdated(this, args);
        }
    }
}
