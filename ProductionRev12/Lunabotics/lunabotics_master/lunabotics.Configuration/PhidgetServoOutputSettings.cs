using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    [Serializable]
    public class PhidgetServoOutputSettings
    {
        Devices device;
        PhidgetServoChannel channel;
        PWMMapping mapping;
        int neutral_value;

        public PhidgetServoOutputSettings()
        {
        }

        public PhidgetServoOutputSettings(Devices dev, int neutral_val, PWMMapping pwm_map, PhidgetServoChannel channel)
        {
            if (pwm_map == null)
                throw new ArgumentNullException("pwm_map");

            this.device = dev;
            this.neutral_value = neutral_val;
            this.mapping = pwm_map;
            this.channel = channel;
        }

        public Devices Device
        {
            get { return device; }
            set { device = value; }
        }

        public PWMMapping PWM_Map
        {
            get { return mapping; }
            set { mapping = value; }
        }

        public int NeutralValue
        {
            get { return neutral_value; }
            set { neutral_value = value; }
        }

        public PhidgetServoChannel Channel
        {
            get { return channel; }
            set { channel = value; }
        }
    }
}
