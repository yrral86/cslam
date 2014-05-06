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

class get_distance_serial
{
    static void Main(string[] args)
    {
        const int GET_NUM = 10;
        const int start_step = 0;
        const int end_step = 760;
        try {
            string port_name;
            int baudrate;
            get_serial_information(out port_name, out baudrate);

            SerialPort urg = new SerialPort(port_name, baudrate);
            urg.NewLine = "\n\n";

            urg.Open();

            urg.Write(SCIP_Writer.SCIP2());
            urg.ReadLine(); // ignore echo back
            urg.Write(SCIP_Writer.MD(start_step, end_step));
            urg.ReadLine(); // ignore echo back

            List<long> distances = new List<long>();
            long time_stamp = 0;
            for (int i = 0; i < GET_NUM; ++i) {
                string receive_data = urg.ReadLine();
                if (!SCIP_Reader.MD(receive_data, ref time_stamp, ref distances)) {
                    Console.WriteLine(receive_data);
                    break;
                }
                if (distances.Count == 0) {
                    Console.WriteLine(receive_data);
                    continue;
                }
                // show distance data
                Console.WriteLine("time stamp: " + time_stamp.ToString() + " distance[100] : " + distances[100].ToString());
            }

            urg.Write(SCIP_Writer.QT()); // stop measurement mode
            urg.ReadLine(); // ignore echo back
            urg.Close();
        } catch (Exception ex) {
            Console.WriteLine(ex.Message);
        } finally {
            Console.WriteLine("Press any key.");
            Console.ReadKey();
        }
    }

    /// <summary>
    /// get connection information from user.
    /// </summary>
    private static void get_serial_information(out string port_name, out int baudrate)
    {
        port_name = "COM3";
        baudrate = 115200;
        Console.WriteLine("Please enter port name. [default: " + port_name + "]");
        string str = Console.ReadLine();
        if (str != "") {
            port_name = str;
        }
        Console.WriteLine("Please enter baudrate. [default: " + baudrate.ToString() + "]");
        str = Console.ReadLine();
        if (str != "") {
            baudrate = int.Parse(str);
        }
        Console.WriteLine("Connect setting = Port name : " + port_name + " Baudrate : " + baudrate.ToString());
    }
}