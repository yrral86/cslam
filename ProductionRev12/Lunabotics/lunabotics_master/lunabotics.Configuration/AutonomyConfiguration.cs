using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace lunabotics.Configuration
{
    public class AutonomyConfiguration
    {
        #region Fields
        public double BlastPercent;
        public double BlastRadius;
        public double ConvergenceAllowance;
        public double Interval;
        public double LocalizationTurnDuration;
        public double MinRegolithPerRun;
        public double RandomReInitPercent;
        public double DesiredLocalizationConfidence;
        public double WallMargin;
        public int NumberOfFrontRFSignals;
        public int NumberOfRearRFSignals;
        public int NumberOfParticles;
        #endregion
        }
}
