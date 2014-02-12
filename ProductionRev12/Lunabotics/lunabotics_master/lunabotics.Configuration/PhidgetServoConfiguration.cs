using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    [Serializable]
    public class PhidgetServoConfiguration
    {
        private List<PhidgetServoOutputSettings> outputSettings;
        private int id;
        private int timeout;

        private PhidgetServoConfiguration()
        {
        }

        public PhidgetServoConfiguration(int timeout, int id)
            : this(timeout, id, new List<PhidgetServoOutputSettings>())
        {
        }

        public PhidgetServoConfiguration(int timeout, int id, List<PhidgetServoOutputSettings> settings)
        {
            if (timeout <= 0)
                throw new ArgumentOutOfRangeException("timeout");


            if (settings == null)
                throw new ArgumentNullException("settings");

            this.timeout = timeout;
            this.id = id;
            this.outputSettings = settings;
        }

        public void AddOutputSettings(PhidgetServoOutputSettings setting)
        {
            if (setting == null)
                throw new ArgumentNullException("setting");

            // todo : validate
            outputSettings.Add(setting);
        }

        public List<PhidgetServoOutputSettings> OutputSettings
        {
            get { return outputSettings; }
            set { outputSettings = value; }
        }

        public int ID
        {
            get { return id; }
            set { id = value; }
        }

        public int Timeout
        {
            get { return timeout; }
            set { timeout = value; }
        }
    }
}
