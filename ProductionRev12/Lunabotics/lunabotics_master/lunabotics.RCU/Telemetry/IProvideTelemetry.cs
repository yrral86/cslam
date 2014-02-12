using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Configuration;

namespace lunabotics.RCU.Telemetry
{

    public class TelemetryEventArgs : EventArgs
    {
        private Dictionary<Configuration.Telemetry, int> updatedTelemetry;
        private int interval;

        public TelemetryEventArgs(int interval, Dictionary<Configuration.Telemetry, int> updatedTelemetry)
        {
            if (updatedTelemetry == null)
                throw new ArgumentNullException("updatedTelemetry");

            if (interval <= 0)
                throw new ArgumentOutOfRangeException("interval must be positive");

            this.interval = interval;
            this.updatedTelemetry = updatedTelemetry;
        }

        public Dictionary<Configuration.Telemetry, int> UpdatedTelemetry
        {
            get { return updatedTelemetry; }
        }

        public int Interval
        {
            get { return interval; }
        }
    }

    public interface IProvideTelemetry
    {
        event EventHandler<TelemetryEventArgs> TelemetryUpdated;
    }
}
