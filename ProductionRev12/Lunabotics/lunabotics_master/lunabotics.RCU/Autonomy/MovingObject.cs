using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Utility;
using lunabotics.Comms.CommandEncoding;
using lunabotics.Configuration;
using lunabotics.RCU.Telemetry;

namespace lunabotics.RCU.Autonomy
{
    public class MovingObject
    {
        #region Constants
        // TODO: Measure these, and possibly put them in the configuration
        public const double RotationalVelocityVariance = 10;
        public const double TranslationalVelocityVariance = 30;
        #endregion

        #region Fields
        protected double angle;
        protected double x;
        protected double y;
        protected double translationalVelocity;
        protected double rotationalVelocity;
        private double translationalVelocitySigma;
        private double rotationalVelocitySigma;

        #endregion

        #region Constructor
        public MovingObject()
        {
            translationalVelocitySigma = Math.Sqrt(TranslationalVelocityVariance);
            rotationalVelocitySigma = Math.Sqrt(RotationalVelocityVariance);
        }
        #endregion

        #region Methods
        public void Set(double x, double y, double angle)
        {
            this.x = x;
            this.y = y;
            this.angle = angle;
        }

        public void SetVelocities(double translationalVelocity, double rotationalVelocity)
        {
            this.translationalVelocity = translationalVelocity;
            this.rotationalVelocity = rotationalVelocity;
        }
        #endregion

        #region Properties
        public double Angle
        {
            get { return angle; }
            set { angle = value; }
        }

        public double RotationalVelocity
        {
            get { return rotationalVelocity; }
            set { rotationalVelocity = value; }
        }

        public double TranslationalVelocity
        {
            get { return translationalVelocity; }
            set { translationalVelocity = value; }
        }

        public double X
        {
            get { return x; }
            set { x = value; }
        }

        public double Y
        {
            get { return y; }
            set { y = value; }
        }
        #endregion
    }
}
