using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Utility;

namespace lunabotics.RCU.Autonomy
{
    public class Tracker
    {
        #region Fields
        private double angle;
        private double x;
        private double y;
        #endregion

        #region Constructors
        public Tracker()
        {
            // Initialize fields
            this.x = 0.0;
            this.y = 0.0;
            this.angle = 0.0;
        }
        #endregion

        #region Methods
        public void FindCenter(List<Particle> particles)
        {
            double sum = 0.0;
            double newX = 0.0;
            double newY = 0.0;
            double dx = 0.0;
            double dy = 0.0;

            // Calculate weight sum
            particles.ForEach(p => sum += p.Weight);

            // Calculate new x, y, and angle
            foreach (Particle particle in particles)
            {
                newX += particle.X * particle.Weight / sum;
                newY += particle.Y * particle.Weight / sum;
                dx += Math.Cos(MathHelper.DegreeToRadian(particle.Angle)) * particle.Weight / sum;
                dy += Math.Sin(MathHelper.DegreeToRadian(particle.Angle)) * particle.Weight / sum;
            }

            // Assign new values
            x = newX;
            y = newY;
            angle = MathHelper.RadianToDegree(Math.Atan2(dy, dx));
        }
        #endregion

        #region Properties
        public double Angle
        {
            get { return angle; }
            set { angle = value; }
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
