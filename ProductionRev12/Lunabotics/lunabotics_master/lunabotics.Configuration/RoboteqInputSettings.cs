using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    public class RoboteqInputSettings
    {
        public string TelemetryString; //string to send to retrieve data
        public int Interval; //time between requests (ms)

        public List<TelemetryMapping> TelemetryMappings;
    }

    public class TelemetryMapping
    {
        public string TypeLabel; //such as "BA" for battery amps, "DI" for digital inputs
        public List<TelemetryIndexMapping> IndexMappings;        
    }

    public class TelemetryIndexMapping
    {
        public int Index;
        public Telemetry Telemetry;
    }
}
