using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Configuration;

namespace Kinematics
{
    public class Kinematics
    {
        public enum Rotation
        {
            Clockwise = 0,
            CounterClockwise = -2048
        }


        private int MAX_VELOCITY;

        public Kinematics(int max_vel)
        {
            if (max_vel <= 0)
                throw new ArgumentOutOfRangeException("Maximum velocity must be positive");
            MAX_VELOCITY = max_vel;
        }

        public Dictionary<Devices, int> GetWheelStates(int radius, int velocity)
        {
            if (Math.Abs(radius) == 2047)
            { //simple enough, just drive straight at the given velocities.
                return GetStraightSystemState(velocity);
            }
            else if(radius == (int)Rotation.Clockwise || radius == (int)Rotation.CounterClockwise)
            { //special case: spin in place! left side opposite right side.
                return GetSpinSystemState(velocity, (Rotation)radius);
            }
            else
            { //general case
                return GetGeneralSystemState(radius, velocity);
            }
        }

        private Dictionary<Devices, int> GetStraightSystemState(double velocity)
        {
            Dictionary<Devices, int> states = new Dictionary<Devices, int>();

            int adjusted_velocity = Convert.ToInt32(Math.Min(MAX_VELOCITY, Math.Abs(velocity))) * Math.Sign(velocity);

            states[Devices.FrontLeftWheel] = adjusted_velocity;
            states[Devices.FrontRightWheel] = adjusted_velocity;
            states[Devices.RearLeftWheel] = adjusted_velocity;
            states[Devices.RearRightWheel] = adjusted_velocity;

            return states;
        }

        private Dictionary<Devices, int> GetSpinSystemState(double velocity, Rotation rotation)
        {
            Dictionary<Devices, int> states = new Dictionary<Devices, int>();

            int vel = Convert.ToInt32(Math.Min(MAX_VELOCITY, Math.Abs(velocity))) * Math.Sign(velocity);

            if (rotation == Rotation.CounterClockwise)
            { //flip velocity to turn opposite
                vel *= -1;
            }

            states[Devices.FrontLeftWheel] = vel;
            states[Devices.FrontRightWheel] = -vel;
            states[Devices.RearLeftWheel] = vel;
            states[Devices.RearRightWheel] = -vel;

            return states;
        }

        private Dictionary<Devices, int> GetGeneralSystemState(double radius, double velocity)
        {
            Dictionary<Devices, int> states = new Dictionary<Devices, int>();

            int l_vel, r_vel;
            double l_scalar, r_scalar;

            if (Math.Abs(radius) == Constants.TRACK_WIDTH / 2.0)
            { //one side stop, other side goes
                if (radius < 0)
                {//center to left side
                    l_vel = 0;
                    r_vel = Convert.ToInt32(Math.Min(Math.Abs(velocity)*2, MAX_VELOCITY)) * Math.Sign(velocity);
                }
                else
                { //center to ride side
                    r_vel = 0;
                    l_vel = Convert.ToInt32(Math.Min(Math.Abs(velocity)*2, MAX_VELOCITY)) * Math.Sign(velocity);
                }
            }
            else if (Math.Abs(radius) < Constants.TRACK_WIDTH / 2.0)
            { //opposite sign, different magnitude

                //get scalars
                l_scalar = Math.Abs(radius + Constants.TRACK_WIDTH / 2.0);
                r_scalar = Math.Abs(radius - Constants.TRACK_WIDTH / 2.0);
                if (radius < 0)
                { //center to left: left vel opposite sign of given velocity; right side the same
                  //right side velocity will be greater
                    r_vel = Convert.ToInt32(Math.Min(r_scalar / Math.Abs(radius) * Math.Abs(velocity), MAX_VELOCITY))*Math.Sign(velocity);
                    l_vel = -Convert.ToInt32(l_scalar / r_scalar * r_vel);
                }
                else
                { //center to right: right side opposite sign of given velocity; left side the same
                  //left side velocity will be greater
                    l_vel = Convert.ToInt32(Math.Min(l_scalar / Math.Abs(radius) * Math.Abs(velocity), MAX_VELOCITY)) * Math.Sign(velocity);
                    r_vel = -Convert.ToInt32(r_scalar / l_scalar * l_vel);
                }
            }
            else
            { //same sign, different magnitude

                //get scalars
                l_scalar = Math.Abs(radius + Constants.TRACK_WIDTH / 2.0);
                r_scalar = Math.Abs(radius - Constants.TRACK_WIDTH / 2.0);
                if (r_scalar > l_scalar)
                {
                    r_vel = Convert.ToInt32(Math.Min(r_scalar / Math.Abs(radius) * Math.Abs(velocity), MAX_VELOCITY)) * Math.Sign(velocity);
                    l_vel = Convert.ToInt32(l_scalar / r_scalar * r_vel);
                }
                else
                {
                    l_vel = Convert.ToInt32(Math.Min(l_scalar / Math.Abs(radius) * Math.Abs(velocity), MAX_VELOCITY)) * Math.Sign(velocity);
                    r_vel = Convert.ToInt32(r_scalar / l_scalar * l_vel);
                }
            }
    
            states[Devices.FrontLeftWheel] = l_vel;
            states[Devices.FrontRightWheel] = r_vel;
            states[Devices.RearLeftWheel] = l_vel;
            states[Devices.RearRightWheel] = r_vel;
            return states;
        }
    }
}
