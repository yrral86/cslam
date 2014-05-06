using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Utility;

namespace lunabotics.RCU.Autonomy
{
    public class Particle : MovingObject, ICloneable
    {
        #region Fields
        private double normalizedWeight;
        private double weight;
        private Field field;
        private RangeFinderModel rangeFinderFront;
        private RangeFinderModel rangeFinderRear;
        #endregion

        #region Constructors
        public Particle(Field field, double x, double y, double angle)
        {
            // Assign fields
            this.field = field;
            this.x = x;
            this.y = y;
            this.angle = angle;

            // Create rangefinders
            rangeFinderFront = new RangeFinderModel(field, this, 0.0, 46.99);
            rangeFinderRear = new RangeFinderModel(field, this, 180.0, -18*2.54);

        }
        #endregion

        #region Methods
        public object Clone()
        {
            return new Particle(field, x, y, angle);
        }

        public double[] ReadSensors(bool useFrontRangeFinder)
        {
            if (useFrontRangeFinder)
            {
                double[] measurements = new double[rangeFinderFront.Measurements.Length + rangeFinderRear.Measurements.Length];

                // Update sensors
                rangeFinderFront.UpdateMeasurements();
                rangeFinderRear.UpdateMeasurements();

                // Combine arrays
                rangeFinderFront.Measurements.CopyTo(measurements, 0);
                rangeFinderRear.Measurements.CopyTo(measurements, rangeFinderFront.Measurements.Length);

                return measurements;
            }
            else
            {
                double[] measurements = new double[rangeFinderRear.Measurements.Length];

                // Update sensors
                rangeFinderRear.UpdateMeasurements();

                // Combine arrays
                rangeFinderRear.Measurements.CopyTo(measurements, 0);

                return measurements;
            }
        }

        public double UpdateWeight(double[] measurement, bool useFrontRangeFinder)
        {
            // Get exact measurements from particle
            double[] expected = ReadSensors(useFrontRangeFinder);

            // Add probabilites of individual signals
            double weightSum = 0.0;
            for (int i = 0; i < measurement.Length; i++)
            {
                double error = Math.Abs(measurement[i] - expected[i]);
                weightSum += 2.0 * (1.0 - MathHelper.CDF(error, 0.0, RangeFinderModel.Variance));
            }

            // Normalize
            weight = weightSum / measurement.Length;
            return weight;
        }
        #endregion

        #region Properties
        public double NormalizedWeight
        {
            get { return normalizedWeight; }
            set { normalizedWeight = value; }
        }
        public double Weight
        {
            get { return weight; }
            set { weight = value; }
        }
        #endregion

    }
}
