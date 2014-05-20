using lunabotics.Comms.CommandEncoding;
using lunabotics.Comms.TelemetryEncoding;
using lunabotics.Configuration;
using lunabotics.RCU.Telemetry;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Timers;
using System.Threading;
using lunabotics.RCU.Autonomy.Algorithms;
using lunabotics.RCU.Models;
using lunabotics.RCU.Hokuyo;
using lunabotics.RCU.Localization;
using System.Net.Sockets;



namespace lunabotics.RCU.Autonomy.Localization
{
    public class Filtering
    {
        Thread t;
        public Filtering(int radius)
        {
            t = new Thread(new ThreadStart(() => Run_Particle_Filter(radius)));
        }

        public void startFiltering()
        {
            t.Start();
        }

        public static void Run_Particle_Filter(int sensorradius)
        {
            int[] EthernetSensorData;
            get_distance_ethernet utm = new get_distance_ethernet();

            //Start Swarm for Testing
            Swarm.swarm_init(721, 180, 7380, 3880, 1940, sensorradius);

            EthernetSensorData = utm.EthernetScan();

            while(true)
            {
                    Swarm.swarm_update(EthernetSensorData);
                    EthernetSensorData = utm.EthernetScan();
                    Swarm.swarm_update_finalize();
                    Swarm.swarm_move(0, 0, 0);
            }
        }   
    }
}
