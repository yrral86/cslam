using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Utility;
using System.Threading.Tasks;

namespace lunabotics.RCU.Autonomy
{
    public class RangeFinderModel
    {
        #region Constants
        private const double MaxRange = 400.0; // cm
        private const double BeginSweep = 0.0; // degrees
        private const double EndSweep = 180.0; // degrees
        public const int MeasurementCount = 512;
        public const double Variance = 10;
        public const double CutoffProbability = 0.1;
        #endregion

        #region Fields
        private double offset;
        private double angleDelta;
        private double mountAngle;
        private double standardDeviation;
        private double startingAngle;
        private double[] measurements = new double[MeasurementCount];
        private object updateLock = new object();
        private Field field;
        private MovingObject entity;
        #endregion

        #region Constructor
        public RangeFinderModel(Field field, MovingObject entity, double mountAngle, double offset)
        {
            // Initialize fields
            this.field = field;
            this.entity = entity;
            this.mountAngle = mountAngle;
            this.offset = offset;

            // Calculate angle delta
            this.angleDelta = (EndSweep - BeginSweep) / ((double)MeasurementCount - 1.0);

            // Calculate starting angle
            startingAngle = mountAngle + entity.Angle - ((EndSweep - BeginSweep) / 2.0);
            // Standard deviation
            standardDeviation = Math.Sqrt(Math.Sqrt(Variance));
        }        
        #endregion

        #region Methods
        public void UpdateMeasurements()
        {
            double x = entity.X + offset * Math.Cos(MathHelper.DegreeToRadian(entity.Angle));
            double y = entity.Y + offset * Math.Sin(MathHelper.DegreeToRadian(entity.Angle));
            double dx = field.Width - x;
            double dy = field.Height - y;

            // Iterate through measure angles
            Parallel.For(0, MeasurementCount, i =>
                {
                    // Get current angle
                    double sensorAngle = startingAngle + i * angleDelta;

                    // Precalc radian angle
                    double radAngle = MathHelper.DegreeToRadian(sensorAngle);
                    double cosAngle = Math.Cos(radAngle);
                    double sinAngle = Math.Sin(radAngle);

                    // Find possible distances                
                    double[] distances = new double[4];
                    distances[0] = -x / cosAngle;
                    distances[1] = dx / cosAngle;
                    distances[2] = -y / sinAngle;
                    distances[3] = dy / sinAngle;

                    // Eliminate negative numbers
                    for (int j = 0; j < 4; j++)
                    {
                        if (distances[j] < 0)
                            distances[j] = Double.MaxValue;
                    }
                    // Get minimum
                    double value = distances.Min();

                    // Remove outside range
                    if (value > MaxRange)
                        value = 0;

                    // Set proper measurement
                    if (MathHelper.random.NextDouble() > CutoffProbability)
                    {
                        measurements[i] = value + MathHelper.NextGaussianDouble(MathHelper.random, 0, standardDeviation);
                    }
                    else
                    {
                        measurements[i] = 0;
                    }
                });
        }
        #endregion

        #region Properties
        public double[] Measurements
        {
            get 
            {
                lock (updateLock)
                {
                    return measurements;                     
                }
            }
        }
    	#endregion
    }
}
