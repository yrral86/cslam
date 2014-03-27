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

        public int[] EthernetScan(NetworkStream stream)
        {
            const int start_step = 0;
            const int end_step = 1080;

            //List to hold all 1080 distance values
            List<int> distances = new List<int>();

            int[] distanceArray;

            try
            {
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
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine(ex.StackTrace);
            }

            distanceArray = distances.ToArray();
            return distanceArray;
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

        /* Doesn't really seem necessary - put code in autonomyhandler instead.
        public TcpClient init()
        {
            TcpClient urg = new TcpClient();
            return urg;
        }*/

        public void quit(NetworkStream stream, TcpClient urg)
        {
            write(stream, SCIP_Writer.QT());    // stop measurement mode
            read_line(stream); // ignore echo back
            stream.Close();
            urg.Close();
        }
    }
}