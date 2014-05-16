/*!
 * \file
 * \brief Get distance data from Ethernet type URG
 * \author Jun Fujimoto
 * $Id: get_distance_ethernet.cs 403 2013-07-11 05:24:12Z fujimoto $
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using SCIP_library;

namespace lunabotics.RCU.Hokuyo
{

    class get_distance_ethernet
    {

        public int[] EthernetScan()
        {
//            const int start_step = 0;
//            const int end_step = 1080;
            const int start_step = 180; //left-most (Robot perspective)
            const int end_step = 900; //right-most


            //List to hold all 1080 distance values
            List<int> distances = new List<int>();

            int[] distanceArray;

            try
            {

                string ip_address = "192.168.1.8";
                int port_number = 10940;

                TcpClient urg = new TcpClient();
                urg.Connect(ip_address, port_number);
                NetworkStream stream = urg.GetStream();

                write(stream, SCIP_Writer.SCIP2());
                read_line(stream);  // ignore echo back
                write(stream, SCIP_Writer.MD(start_step, end_step));
                read_line(stream);  // ignore echo back

                long time_stamp = 0;

                string receive_data = read_line(stream);
                if (!SCIP_Reader.MD(receive_data, ref time_stamp, ref distances))
                {
                    Console.WriteLine(receive_data);
                }
                else
                {
                    if (distances.Count == 0)
                    {
                        Console.WriteLine(receive_data);
                    }
                    //// show distance data
                    //for (int j = 0; j < 760; j++)
                    //{
                    //    Console.WriteLine(distances[j].ToString());
                    //}
                }

                write(stream, SCIP_Writer.QT());    // stop measurement mode
                read_line(stream); // ignore echo back
                stream.Close();
                urg.Close();

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine(ex.StackTrace);
            }

            distanceArray = distances.ToArray();
            return distanceArray;
        }

        //take value array from EthernetScan and return averaged cenenter with +/- 10 degrees
        //return array: [int leftVal, int CenterVal, int rightVal]
        public int[] getLCR()
        {
            /*  
                720 scan points
                180 degrees

                720/180  = 4 scanpoints/degrees

                90 degrees:
                90*4 = 360
                scanpoint 360 is 90 degrees
              
                110 degrees (+20):
                110*4 = 440
                scanpoint 440 is 110 degrees

                70 degrees (-20):
                70*4 = 280
                scanpoint 280 is 70 degrees
             */

            int[] LCR = new int[3];
            int[] scanData = EthernetScan(); //hokuyo values from 180 to 900 (180 degree scan)
            //center-most values @ sample 360
            //average 10 center-most values:
            int sum = 0;
            for (int i = 355; i < 365; i++)
            {
                sum += scanData[i];
            }
            LCR[1] = (int)(sum / 10); //center

            //-20 degrees off-center values @ sample 460
            //10 average -20 degrees off-center values:
            sum = 0;
            for (int i = 275; i < 285; i++)
            {
                sum += scanData[i];
            }
            LCR[0] = (int)(sum / 10); //left

            //+20 degrees off-center values @ sample 620
            //10 average +20 degrees off-center values:
            sum = 0;
            for (int i = 435; i < 445; i++)
            {
                sum += scanData[i];
            }
            LCR[2] = (int)(sum / 10); //right

            return LCR;
        }


        /// <summary>
        /// Read to "\n\n" from NetworkStream
        /// </summary>
        private string read_line(NetworkStream stream)
        {
            if (stream.CanRead)
            {
                StringBuilder sb = new StringBuilder();
                bool is_NL2 = false;
                bool is_NL = false;
                do
                {
                    char buf = (char)stream.ReadByte();
                    if (buf == '\n')
                    {
                        if (is_NL)
                        {
                            is_NL2 = true;
                        }
                        else
                        {
                            is_NL = true;
                        }
                    }
                    else
                    {
                        is_NL = false;
                    }
                    sb.Append(buf);
                } while (!is_NL2);

                return Convert.ToString(sb);
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// write data
        /// </summary>
        private bool write(NetworkStream stream, string data)
        {
            if (stream.CanWrite)
            {
                byte[] buffer = Encoding.ASCII.GetBytes(data);
                stream.Write(buffer, 0, buffer.Length);
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}