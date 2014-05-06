using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    [Serializable]
    public class PWMMapping
    {
        int real_low, real_high;
        int pwm_low, pwm_high;

        private PWMMapping()
        {
        }

        public PWMMapping(int real_low, int pwm_low, int real_high, int pwm_high)
        {
            this.real_low = real_low;
            this.real_high = real_high;
            this.pwm_low = pwm_low;
            this.pwm_high = pwm_high;
        }

        public PWMMapping(int pwm_low, int pwm_high)
        {
            this.pwm_low = pwm_low;
            this.pwm_high = pwm_high;
        }

        public int Real_Low
        {
            get { return real_low; }
            set { real_low = value; }
        }

        public int Real_High
        {
            get { return real_high; }
            set { real_high = value; }
        }

        public int PWM_Low
        {
            get { return pwm_low; }
            set { pwm_low = value; }
        }

        public int PWM_High
        {
            get { return pwm_high; }
            set { pwm_high = value; }
        }
    }
}
