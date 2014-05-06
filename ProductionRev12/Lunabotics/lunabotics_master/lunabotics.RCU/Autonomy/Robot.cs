using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Comms.TelemetryEncoding;
using lunabotics.Configuration;

namespace lunabotics.RCU.Autonomy
{
    public class Robot : MovingObject
    {
        #region Enums
        public enum Zone
        {
            Starting,
            Obstacle,
            Mining,
            Unknown
        }
        #endregion

        #region Fields
        private double regolith;
        private AutonomyConfiguration configuration;
        private Zone expectedZone;
        private TelemetryFeedback telemetryFeedback;
        #endregion

        #region Constructor
        public Robot(AutonomyConfiguration configuration)
            : base()
        {
            // Set configuration
            this.configuration = configuration;

            // Set initial zone
            this.expectedZone = Zone.Starting;
        }
        #endregion

        #region Methods
        //public double[] ReadSensors(bool useFront)
        //{
        //    if (useFront)
        //    {
        //        // Combine arrays
        //        double[] measurements = new double[configuration.NumberOfFrontRFSignals + configuration.NumberOfRearRFSignals];
        //        // Copy front measurements
        //        for (int i = 0; i < configuration.NumberOfFrontRFSignals - 1; i++)
        //            measurements[i] = telemetryFeedback.FrontRangeFinderData[i * (511 / (configuration.NumberOfFrontRFSignals - 1))];
        //        // Copy rear measurements
        //        for (int i = 0; i < configuration.NumberOfRearRFSignals - 1; i++)
        //            measurements[i + configuration.NumberOfFrontRFSignals] = telemetryFeedback.RearRangeFinderData[i * (511 / (configuration.NumberOfRearRFSignals - 1))];
        //        return measurements;
        //    }
        //    else
        //    {
        //        // Combine arrays
        //        double[] measurements = new double[configuration.NumberOfRearRFSignals];
        //        // Copy rear measurements
        //        for (int i = 0; i < configuration.NumberOfRearRFSignals - 1; i++)
        //            measurements[i] = telemetryFeedback.RearRangeFinderData[i * (511 / (configuration.NumberOfRearRFSignals - 1))];
        //        return measurements;
        //    }
        //}
        //public void SetState(Tracker tracker)
        //{
        //    this.x = tracker.X;
        //    this.y = tracker.Y;
        //    this.angle = tracker.Angle;
        //}
        #endregion

        #region Propterties
        public double Regolith
        {
            get { return regolith; }
            set { regolith = value; }
        }
        public Zone ExpectedZone
        {
            get { return expectedZone; }
            set { expectedZone = value; }
        }
        public TelemetryFeedback TelemetryFeedback
        {
            get { return telemetryFeedback; }
            set { telemetryFeedback = value; }
        }
        #endregion
    }
}
