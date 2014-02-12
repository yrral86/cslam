using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Comms.CommandEncoding
{
    public enum CommandFields : ushort
    {
        Mode = 0x1000, // Auto/manual
        TranslationalVelocity = 0x2000, //between -1000,1000
        RotationalVelocity = 0x3000, // 0->straight, 1000->full left, -1000->full right
        BucketPivot = 0x4000, //between -1000,1000
        BucketPitch = 0x5000, //between -1000,1000
        LeftBucketActuator = 0x6000, //between -1000, 1000
        RightBucketActuator = 0x6000, //between -1000, 1000
        FrontCameraState = 0x7000,
        RearCameraState = 0x8000,
        BucketCameraState = 0x9000,
        RangeFinderServo = 0xA000
    }
}
