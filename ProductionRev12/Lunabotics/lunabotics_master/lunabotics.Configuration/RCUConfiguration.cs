using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    public class RCUConfiguration
    {
        public int CommandPort;
        public string OCU_IP_Address;

        public AutonomyConfiguration AutonomyConfiguration;
        public List<RoboteqConfiguration> RoboteqConfigurations;
        public CameraConfiguration FrontCameraConfiguration;
        public CameraConfiguration BucketCameraConfiguration;
        public CameraConfiguration RearCameraConfiguration;

        public PhidgetServoConfiguration PhidgetConfiguration;
        public TelemetryConfiguration TelemetryConfiguration;

        public RangeFinderConfiguration FrontRangeFinder;
        public RangeFinderConfiguration RearRangeFinder;
    }
}
