using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.Ports;
using lunabotics.Configuration;
using System.Threading;

namespace lunabotics.RCU.Models
{
    public class RangeFinder
    {
        public static readonly int MIN_STEP = 44;
        public static readonly int MAX_STEP = 725;
        public static readonly int MAX_DATA_SIZE = MAX_STEP - MIN_STEP + 1;

        private static readonly int MAX_KEY_SIZE = 15;
        private static readonly string RESET_ACK_POSTFIX = "00P\n\n";
        private static readonly string MS_ACK_POSTFIX = "00P\n\n";

        private static readonly string MS_SUCCESS_STATUS = "99b";

        private volatile bool isActive = false;
        private static readonly object dataSync = new object();

        private RangeFinderConfiguration configuration;
        private SerialPort port;
        private Thread monitor_thread;
        CancellationTokenSource tokenSource;

        double[] data_storage = new double[MAX_DATA_SIZE];
        double[] temp_data = new double[MAX_DATA_SIZE];
        private volatile int data_length = 0;
        private volatile int data_timestamp = 0;

        public RangeFinder(RangeFinderConfiguration config)
        {
            VerifyConfiguration(config);
            this.configuration = config;
            port = new SerialPort(config.Port, 19200, Parity.None, 8, StopBits.One);
            port.WriteTimeout = 1000;
            port.ReadTimeout = 1000;
        }

        private static void VerifyConfiguration(RangeFinderConfiguration config)
        {
            if (config == null)
                throw new ArgumentNullException("config");

            if (String.IsNullOrWhiteSpace(config.Port))
                throw new ArgumentNullException("port");

            if (config.StartStep < MIN_STEP || config.StartStep > MAX_STEP)
                throw new ArgumentOutOfRangeException("StartStep");

            if (config.EndStep < MIN_STEP || config.EndStep > MAX_STEP)
                throw new ArgumentOutOfRangeException("EndStep");

            if (config.StartStep >= config.EndStep)
                throw new ArgumentException("EndStep must be greater than StartStep");

            if (config.ClusterCount < 0 || config.ClusterCount > 99)
                throw new ArgumentOutOfRangeException("ClusterCount");
        }

        public void Activate()
        {
            if (isActive)
                return;

            tokenSource = new CancellationTokenSource();
            try
            {
                port.Open();
                port.DiscardOutBuffer();

                /*send & verify reset command*/
                this.reset();

                /*send & verify reset command*/
                this.send_ms_command();

                /*launch monitor*/
                monitor_thread = new Thread(new ThreadStart(monitor_worker));
                monitor_thread.Start();

                isActive = true;
            }
            catch (Exception ex)
            {
                InternalDeactive();
                throw;
            }
        }

        public void DeActivate()
        {
            if (!isActive)
                return;

            InternalDeactive();
            isActive = false;
        }

        private void InternalDeactive()
        {
            tokenSource.Cancel();
            try
            {
                if (port != null && port.IsOpen)
                    port.Close();
            }catch(Exception){}

            try
            {
            if (monitor_thread != null && monitor_thread.ThreadState != ThreadState.Unstarted)
                monitor_thread.Join();
            }
            catch(Exception){}
        }

        public bool IsActive
        {
            get { return isActive; }
        }

        public double[] CopyData(double[] destination, out int length, out int timing)
        {
            double[] output;
            lock (dataSync)
            {
                timing = data_timestamp;
                length = data_length;
                if (data_length == 0) //no data copied
                {
                    return destination;
                }
                else
                {
                    if (destination == null || destination.Length < data_length)
                        output = new double[data_length];
                    else
                        output = destination;

                    Array.Copy(data_storage, output, data_length);
                    return output;
                }
            }

        }

        private void reset()
        {
            string resetString = "RS;" + GenerateKey() + "\n";
            port.Write(resetString);
            string response = port.ReadTo(resetString + RESET_ACK_POSTFIX);
        }

        private void send_ms_command()
        {
            string send_string = "MS" + configuration.StartStep.ToString().PadLeft(4, '0') + configuration.EndStep.ToString().PadLeft(4, '0') +
                configuration.ClusterCount.ToString().PadLeft(2, '0') + "000\n";
            port.Write(send_string);
            string response = port.ReadTo(send_string + MS_ACK_POSTFIX);

        }

        private string GenerateKey()
        {
            string toReturn = System.Guid.NewGuid().ToString();

            if (toReturn.Length > MAX_KEY_SIZE)
                return toReturn.Substring(0, MAX_KEY_SIZE);
            else
                return toReturn;
        }

        private void monitor_worker()
        {
            char[] delimiter = new char[] { '\n' };
            int timestamp;
            while (!tokenSource.Token.IsCancellationRequested)
            {
                try
                {
                    string buffer = port.ReadTo("\n\n");

                    string[] lines = buffer.Split(delimiter, StringSplitOptions.RemoveEmptyEntries);

                    if (lines.Length < 3) //not enough data
                        continue; //todo : log/raise event

                    if (!lines[1].Equals(MS_SUCCESS_STATUS)) //check status
                        continue; //todo : log/raise event

                    //timestamp in line 3 (index 2)
                    timestamp = convert_timestamp(lines[2]);

                    /*process data*/
                    process_data(lines, 3, timestamp);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("RangeFinder: " + ex.Message);
                }
            }
        }

        private void process_data(string[] lines, int first_data_line_index, int timestamp)
        {
            int temp_data_size = 0;

            for (int i = first_data_line_index; i < lines.Length; ++i)
            {
                int line_length = lines[i].Length;

                if ((line_length > 65) || (line_length % 2 == 0) || !verify_checksum(lines[i]))
                { //if line is too long, even length or fails checksum
                    lock (dataSync)
                    {
                        this.data_timestamp = timestamp;
                        data_length = 0;
                    }
                    return;
                }
                for (int j = 0; j < line_length - 1; j += 2)
                {
                    double value = decode(lines[i], j, 2);
                    temp_data[temp_data_size++] = Math.Max(configuration.ScalingSlope * value + configuration.ScalingIntercept, 0);
                }
            }

            lock (dataSync)
            {
                this.data_timestamp = timestamp;
                data_length = temp_data_size;
                Array.Copy(temp_data, data_storage, temp_data_size);
            }

        }

        bool verify_checksum(string string_with_checksum)
        {
            uint val = 0;
            int length = string_with_checksum.Length;
            for (int i = 0; i < length - 1; ++i)
            {
                val += string_with_checksum[i];
            }
            val &= 0x3f;

            return ((char)(val) == (string_with_checksum[length - 1] - 0x30));
        }

        private int convert_timestamp(string timestamp_with_checksum)
        {
            if (timestamp_with_checksum.Length != 5)
                return -1;
            else if (!verify_checksum(timestamp_with_checksum))
                return -1;
            else return decode(timestamp_with_checksum, 0, 4);
        }

        int decode(string data, int start_index, int length)
        {
            int value = 0;
            for (int i = 0; i < length; ++i)
            {
                value <<= 6;
                value &= ~0x3f;
                value |= data[start_index + i] - 0x30;
            }
            return value;
        }

    }

    public class RangeDataUpdateEventArgs : EventArgs
    {
        public double[] data;

        public RangeDataUpdateEventArgs(double[] data)
        {
            this.data = data;
        }
    }
}
