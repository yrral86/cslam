/*!
 * \file
 * \brief Get distance data from serial type URG
 * \author Jun Fujimoto
 * $Id: get_distance_serial.cs 403 2013-07-11 05:24:12Z fujimoto $
 */

using System;
using System.Collections.Generic;
using System.IO.Ports;
using SCIP_library;

namespace lunabotics.RCU.Hokuyo
{

    class get_distance_serial
    {

        public int[] SerialScan()
        {
            const int GET_NUM = 1;
            const int start_step = 286;
            const int end_step = 494; // Only scanning 180 degrees

            List<int> distances = new List<int>();
            int[] distanceArray;

            try
            {
                string port_name = "COM4";
                int baudrate = 115200;

                SerialPort urg = new SerialPort(port_name, baudrate);
                urg.NewLine = "\n\n";

                urg.Open();

                urg.Write(SCIP_Writer.SCIP2());
                urg.ReadLine(); // ignore echo back
                urg.Write(SCIP_Writer.MD(start_step, end_step));
                urg.ReadLine(); // ignore echo back

                long time_stamp = 0;
                for (int i = 0; i < GET_NUM; ++i)
                {
                    string receive_data = urg.ReadLine();
                    if (!SCIP_Reader.MD(receive_data, ref time_stamp, ref distances))
                    {
                        Console.WriteLine(receive_data);
                        break;
                    }
                    if (distances.Count == 0)
                    {
                        Console.WriteLine(receive_data);
                        continue;
                    }
                    /*
                    // show distance data
                    for (int j = 365; j < 395; j++)
                    {
                        Console.WriteLine("time stamp: " + time_stamp.ToString() + " distance[" + j + "] : " + distances[j].ToString());
                    }*/
                }

                urg.Write(SCIP_Writer.QT()); // stop measurement mode
                urg.ReadLine(); // ignore echo back
                urg.Close();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            finally
            {
                Console.WriteLine("Press any key.");
                Console.ReadKey();
            }
            distanceArray = distances.ToArray();
            return distanceArray;
        }
    }
}