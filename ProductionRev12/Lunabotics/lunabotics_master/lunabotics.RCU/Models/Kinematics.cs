using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Configuration;

namespace lunabotics.RCU.Models
{
    public class Kinematics
    {
        public static double MaximumWheelVelocity = 2.827; // rad/s
        public static double MaximumRotationalVelocity = 1.713; // rad/s
        public static double MaximumTranslationalVelocity = 55.66; // m/s
        public static double WheelRadius = 19.65; // cm
        public static double WheelTrack = 65.0; // cm

        public static Dictionary<Devices, int> GetWheelStates(double translationalVelocity, double rotationalVelocity)
        {
            //Console.WriteLine("V: " + translationalVelocity + ", R: " + rotationalVelocity);
            // Calculate wheel angular velocities
            double omegaLeft = (translationalVelocity + ((WheelTrack / 2.0) * rotationalVelocity)) / WheelRadius;
            double omegaRight = (translationalVelocity - ((WheelTrack / 2.0) * rotationalVelocity)) / WheelRadius;

            //Console.WriteLine("omega L/R: " + omegaLeft + ", " + omegaRight);

            // Calculate commands
            int leftCommand = (int)((omegaLeft / MaximumWheelVelocity) * 1000.0d);
            int rightCommand = (int)((omegaRight / MaximumWheelVelocity) * 1000.0d);

            //Console.WriteLine("Command: " + leftCommand + ", " + rightCommand);

            // Set output
            Dictionary<Devices, int> output = new Dictionary<Devices, int>();
            output[Devices.FrontLeftWheel] = leftCommand;
            output[Devices.RearLeftWheel] = leftCommand;
            output[Devices.FrontRightWheel] = rightCommand;
            output[Devices.RearRightWheel] = rightCommand;

            return output;
        }

        public static double GetRotationalVelocity(short command)
        {
            return ((double)command / 1000.0d) * MaximumRotationalVelocity;
        }

        public static double GetTranslationalVelocity(short command)
        {
            return ((double)command / 1000.0d) * MaximumTranslationalVelocity;
        }
    }
}
