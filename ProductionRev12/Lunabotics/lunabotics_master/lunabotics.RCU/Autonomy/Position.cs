using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.RCU.Autonomy
{
    public class Position
    {
        #region Fields
        private double x;
        private double y;
        private double angle;
        #endregion

        #region Constructor
        public Position(double x, double y, double angle)
        {
            this.x = x;
            this.y = y;
            this.angle = angle;
        }
        #endregion

        #region Properties
        public double X
        {
            get { return this.x; }
            set { this.x = value; }
        }
        public double Y
        {
            get { return this.y; }
            set { this.y = value; }
        }
        public double Angle
        {
            get { return this.angle; }
            set { this.angle = value; }
        }

        #endregion
    }
}
