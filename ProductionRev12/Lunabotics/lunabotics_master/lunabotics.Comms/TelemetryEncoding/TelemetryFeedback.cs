using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Comms.TelemetryEncoding
{
    public class TelemetryFeedback : ICloneable
    {
        // Health
        public double DriveBatteryVoltage;
        public double CleanBatteryVoltage;
        public double PowerUsed;
        public double RightFrontMotorAmps, RightRearMotorAmps, LeftFrontMotorAmps, LeftRearMotorAmps;


        // Arm state
        public double ScoopPitchAngle;
        public double ArmSwingAngle;
        public double ArmMotorAmps;
        public double BucketPivotAngle;
        public bool ScoopLowerLimitSwitchDepressed;
        public bool ScoopUpperLimitSwitchDepressed;

        // Collection bin state
        //public double BinLeftMotorAmps; // From 'driver' perspective
        //public double BinRightMotorAmps;
        
        // Proximity sensors
        public double RearProximityLeft;
        public double RearProximityRight;

        // Localization
        public double X;
        public double Y;
        public double Psi;
        public int State;

        //// Front rangefinder
        //public int FrontRangeFinderDataLength;
        //public double[] FrontRangeFinderData;

        //// Rear rangefinder
        //public int RearRangeFinderDataLength;
        //public double[] RearRangeFinderData;

        //14 doubles for power/angles, 2 bools for limit switches, 1 ints for rear range finder data length
        private static int MIN_SIZE = 16 * sizeof(double) + 2 * sizeof(bool) + 1 * sizeof(int);

        public object Clone()
        {
            TelemetryFeedback feedback = (TelemetryFeedback)this.MemberwiseClone();

            //if (feedback.FrontRangeFinderData != null)
            //{
            //    feedback.FrontRangeFinderData = new double[FrontRangeFinderDataLength];
            //    this.FrontRangeFinderData.CopyTo(feedback.FrontRangeFinderData, 0);
            //}
            //if (feedback.RearRangeFinderData != null)
            //{
            //    feedback.RearRangeFinderData = new double[RearRangeFinderDataLength];
            //    this.RearRangeFinderData.CopyTo(feedback.RearRangeFinderData, 0);
            //}
            return feedback;
        }

        public byte[] Encode()
        {
            byte[] toReturn = new byte[MIN_SIZE];
            int position = 0;

            //first, take care of all the doubles
            byte[] dbv_bytes = BitConverter.GetBytes(DriveBatteryVoltage);
            Array.Copy(dbv_bytes, 0, toReturn, position, dbv_bytes.Length);
            position += sizeof(double);

            byte[] cbv_bytes = BitConverter.GetBytes(CleanBatteryVoltage);
            Array.Copy(cbv_bytes, 0, toReturn, position, cbv_bytes.Length);
            position += sizeof(double);

            byte[] pu_bytes = BitConverter.GetBytes(PowerUsed);
            Array.Copy(pu_bytes, 0, toReturn, position, pu_bytes.Length);
            position += sizeof(double);

            byte[] spa_bytes = BitConverter.GetBytes(ScoopPitchAngle);
            Array.Copy(spa_bytes, 0, toReturn, position, spa_bytes.Length);
            position += sizeof(double);

            byte[] asa_bytes = BitConverter.GetBytes(ArmSwingAngle);
            Array.Copy(asa_bytes, 0, toReturn, position, asa_bytes.Length);
            position += sizeof(double);

            byte[] ama_bytes = BitConverter.GetBytes(ArmMotorAmps);
            Array.Copy(ama_bytes, 0, toReturn, position, ama_bytes.Length);
            position += sizeof(double);

            byte[] bpa_bytes = BitConverter.GetBytes(BucketPivotAngle);
            Array.Copy(bpa_bytes, 0, toReturn, position, bpa_bytes.Length);
            position += sizeof(double);

            byte[] rfma_bytes = BitConverter.GetBytes(RightFrontMotorAmps);
            Array.Copy(rfma_bytes, 0, toReturn, position, rfma_bytes.Length);
            position += sizeof(double);

            byte[] rrma_bytes = BitConverter.GetBytes(RightRearMotorAmps);
            Array.Copy(rrma_bytes, 0, toReturn, position, rrma_bytes.Length);
            position += sizeof(double);

            byte[] lfma_bytes = BitConverter.GetBytes(LeftFrontMotorAmps);
            Array.Copy(lfma_bytes, 0, toReturn, position, lfma_bytes.Length);
            position += sizeof(double);

            byte[] lrma_bytes = BitConverter.GetBytes(LeftRearMotorAmps);
            Array.Copy(lrma_bytes, 0, toReturn, position, lrma_bytes.Length);
            position += sizeof(double);

            byte[] rpl_bytes = BitConverter.GetBytes(RearProximityLeft);
            Array.Copy(rpl_bytes, 0, toReturn, position, rpl_bytes.Length);
            position += sizeof(double);

            byte[] rpr_bytes = BitConverter.GetBytes(RearProximityRight);
            Array.Copy(rpr_bytes, 0, toReturn, position, rpr_bytes.Length);
            position += sizeof(double);

            byte[] x_bytes = BitConverter.GetBytes(X);
            Array.Copy(x_bytes, 0, toReturn, position, x_bytes.Length);
            position += sizeof(double);

            byte[] y_bytes = BitConverter.GetBytes(Y);
            Array.Copy(y_bytes, 0, toReturn, position, y_bytes.Length);
            position += sizeof(double);

            byte[] psi_bytes = BitConverter.GetBytes(Psi);
            Array.Copy(psi_bytes, 0, toReturn, position, psi_bytes.Length);
            position += sizeof(double);

            byte[] slls_bytes = BitConverter.GetBytes(ScoopLowerLimitSwitchDepressed);
            Array.Copy(slls_bytes, 0, toReturn, position, slls_bytes.Length);
            position += sizeof(byte);

            byte[] suls_bytes = BitConverter.GetBytes(ScoopUpperLimitSwitchDepressed);
            Array.Copy(suls_bytes, 0, toReturn, position, suls_bytes.Length);
            position += sizeof(byte);

            byte[] state_bytes = BitConverter.GetBytes(State);
            Array.Copy(state_bytes, 0, toReturn, position, state_bytes.Length);
            position += sizeof(int);

            //byte[] front_rf_length_bytes = BitConverter.GetBytes(FrontRangeFinderDataLength);
            //Array.Copy(front_rf_length_bytes, 0, toReturn, position, front_rf_length_bytes.Length);
            //position += sizeof(int);

            //byte[] rear_rf_length_bytes = BitConverter.GetBytes(RearRangeFinderDataLength);
            //Array.Copy(rear_rf_length_bytes, 0, toReturn, position, rear_rf_length_bytes.Length);
            //position += sizeof(int);

            //// Copy font rangefinder data
            //for (int i = 0; i < FrontRangeFinderDataLength; ++i)
            //{
            //    byte[] data_bytes = BitConverter.GetBytes(FrontRangeFinderData[i]);
            //    Array.Copy(data_bytes, 0, toReturn, position + i * sizeof(double), data_bytes.Length);
            //}
            //position += sizeof(double) * FrontRangeFinderDataLength;

            //// Copy font rangefinder data
            //for (int i = 0; i < RearRangeFinderDataLength; ++i)
            //{
            //    byte[] data_bytes = BitConverter.GetBytes(RearRangeFinderData[i]);
            //    Array.Copy(data_bytes, 0, toReturn, position + i * sizeof(double), data_bytes.Length);
            //}

            return toReturn;
        }

        public static TelemetryFeedback Decode(byte[] bytes)
        {
            int position = 0;
            if (bytes == null)
                throw new ArgumentNullException("Telemetry byte string is empty.");

            if (bytes.Length < MIN_SIZE)
                throw new ArgumentException("Incorrect size", "bytes");

            TelemetryFeedback feedback = new TelemetryFeedback();


            //Doubles
            feedback.DriveBatteryVoltage = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.CleanBatteryVoltage = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.PowerUsed = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.ScoopPitchAngle = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.ArmSwingAngle = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.ArmMotorAmps = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.BucketPivotAngle = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.RightFrontMotorAmps = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.RightRearMotorAmps = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.LeftFrontMotorAmps = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.LeftRearMotorAmps = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.RearProximityLeft = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.RearProximityRight = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.X = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.Y = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            feedback.Psi = BitConverter.ToDouble(bytes, position);
            position += sizeof(double);

            //Bools           
            feedback.ScoopLowerLimitSwitchDepressed = BitConverter.ToBoolean(bytes, position);
            position += sizeof(bool);

            feedback.ScoopUpperLimitSwitchDepressed = BitConverter.ToBoolean(bytes, position);
            position += sizeof(bool);

            //Ints
            feedback.State = BitConverter.ToInt32(bytes, position);
            position += sizeof(int);

            //feedback.FrontRangeFinderDataLength = BitConverter.ToInt32(bytes, position);
            //position += sizeof(int);

            //feedback.RearRangeFinderDataLength = BitConverter.ToInt32(bytes, position);
            //position += sizeof(int);

            ////process said length worth of data
            //feedback.FrontRangeFinderData = new double[feedback.FrontRangeFinderDataLength];
            //for (int i = 0; i < feedback.FrontRangeFinderDataLength; ++i)
            //{
            //    feedback.FrontRangeFinderData[i] = BitConverter.ToDouble(bytes, position);
            //    position += sizeof(double);
            //}
            //feedback.RearRangeFinderData = new double[feedback.RearRangeFinderDataLength];
            //for (int i = 0; i < feedback.RearRangeFinderDataLength; ++i)
            //{
            //    feedback.RearRangeFinderData[i] = BitConverter.ToDouble(bytes, position);
            //    position += sizeof(double);
            //}

            return feedback;
        }
    }
}
