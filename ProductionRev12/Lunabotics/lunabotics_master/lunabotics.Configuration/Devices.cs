using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    public enum Devices
    {
        /* Drive System */
        FrontLeftWheel = 0,
        FrontRightWheel,
        RearLeftWheel,
        RearRightWheel,

        /* Scoop */
        Pivot,
        Actuators,

        /* Bucket */
        LeftBucketActuator,
        RightBucketActuator,

        /* Rangefinder */
        RangeFinderEnable
    }
}
