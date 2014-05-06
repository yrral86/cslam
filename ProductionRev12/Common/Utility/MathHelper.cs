using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Utility
{
    public class MathHelper
    {
        private const double ToRadianConversion = Math.PI / 180.0;
        private const double FromRadianConversion = 180.0 / Math.PI;

        private static double a;
        public static Random random;

        public static double DegreeToRadian(double angle)
        {
            return angle * ToRadianConversion;
        }

        public static double RadianToDegree(double angle)
        {
            return angle * FromRadianConversion;
        }

        public static double NextGaussianDouble(Random random, double mu, double sigma)
        {
            // Generate uniform random numbers [0, 1]
            double u1 = random.NextDouble();
            double u2 = random.NextDouble();

            // Random normal (0,1)
            double randStdNormal = Math.Sqrt(-2.0 * Math.Log(u1)) * Math.Sin(2.0 * Math.PI * u2);
            // Random normal
            double randNormal = mu + sigma * randStdNormal;

            return randNormal;
        }

        public static double Erf(double z)
        {
            // Initialize a if first run
            if (a == 0)
            {
                a = (8.0 * (Math.PI - 3.0)) / ((3.0 * Math.PI) * (4.0 - Math.PI));
            }

            double zSquared = Math.Pow(z, 2.0);
            double azSquared = a * zSquared;

            return Math.Sign(z) * (Math.Sqrt(1.0 - Math.Exp(-zSquared * (((4.0 / Math.PI) + azSquared) / (1.0 + azSquared)))));
        }

        public static double CDF(double value, double mu, double variance)
        {
            return 0.5 * (1 + Erf((value - mu) / Math.Sqrt(2.0 * Math.Pow(variance, 2.0))));
        }
    }
}
