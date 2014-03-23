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
        public static double WheelRadius = 195; // mm
        public static double WheelTrack = 650; // mm
        public static double HallCountPerRev = 1200;
        public static double CMPerCount = (WheelRadius * 2 * Math.PI) / HallCountPerRev;

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
            output[Devices.FrontLeftWheel] = leftCommand * 100;
            output[Devices.RearLeftWheel] = leftCommand * 100;
            output[Devices.FrontRightWheel] = rightCommand * 100;
            output[Devices.RearRightWheel] = rightCommand * 100;

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

        public static Dictionary<Pose, double> UpdatePose(int stepsRight, int stepsLeft, Dictionary<Pose, double> LastPose)
        {
            Dictionary<Pose, double> updatedPose = new Dictionary<Pose, double>();
            Dictionary<Pose, double> deltaPose = new Dictionary<Pose, double>();

            double RightWheelDeltaS = stepsRight * CMPerCount; //[millimeteres]
            double LeftWheelDeltaS = stepsLeft * CMPerCount; //[millimeters]  
            double LeftRightDistance = (RightWheelDeltaS - LeftWheelDeltaS); //[millimeters]
            double RobotCenterDeltaS = (RightWheelDeltaS + LeftWheelDeltaS) / 2; //[millimeters]

            //driving straight
            if (Math.Abs(LeftRightDistance) < 0.01)
            {
                updatedPose[Pose.Heading] = LastPose[Pose.Heading]; //if driving straight, no heading change
                updatedPose[Pose.Xpos] = (int)(LastPose[Pose.Xpos] + RobotCenterDeltaS * Math.Cos(updatedPose[Pose.Heading]));
                updatedPose[Pose.Ypos] = (int)(LastPose[Pose.Ypos] + RobotCenterDeltaS * Math.Sin(updatedPose[Pose.Heading]));
            }
            else //not driving straight
            {
                double ICR = (WheelTrack * RobotCenterDeltaS) / LeftRightDistance;
                double HeadingChange = LeftRightDistance / WheelTrack;
                updatedPose[Pose.Heading] = LastPose[Pose.Heading] + HeadingChange;
                updatedPose[Pose.Xpos] = (int)(LastPose[Pose.Xpos] + ICR * Math.Sin(updatedPose[Pose.Heading]) - ICR * Math.Sin(LastPose[Pose.Heading]));
                updatedPose[Pose.Ypos] = (int)(LastPose[Pose.Ypos] - ICR * Math.Cos(updatedPose[Pose.Heading]) + ICR * Math.Cos(LastPose[Pose.Heading]));
            }

            deltaPose[Pose.Heading] = updatedPose[Pose.Heading] - LastPose[Pose.Heading];
            Console.WriteLine(deltaPose[Pose.Heading].ToString());
            deltaPose[Pose.Xpos] = updatedPose[Pose.Xpos] - LastPose[Pose.Xpos];
            Console.WriteLine(deltaPose[Pose.Xpos].ToString());
            deltaPose[Pose.Ypos] = updatedPose[Pose.Ypos] - LastPose[Pose.Ypos];
            Console.WriteLine(deltaPose[Pose.Ypos].ToString());
            return deltaPose;
        }
    }

    public enum Pose
    {
        Xpos,
        Ypos,
        Heading
    }
}
