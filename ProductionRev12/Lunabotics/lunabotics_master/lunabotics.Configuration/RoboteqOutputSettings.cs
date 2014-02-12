using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    public enum RoboteqChannel
    {
        Motor1,
        Motor2
    }

    [Serializable]
    public class RoboteqOutputSettings
    {
        Devices device;
        RoboteqChannel channel;
        bool invert_flag;

        public RoboteqOutputSettings()
        {
        }

        public RoboteqOutputSettings(Devices dev, bool invert, RoboteqChannel channel)
        {
            device = dev;
            invert_flag = invert;
            this.channel = channel;
        }

        public Devices Device
        {
            get { return device; }
            set { device = value; }
        }

        public bool Invert
        {
            get { return invert_flag; }
            set { invert_flag = value; }
        }

        public RoboteqChannel Channel
        {
            get { return channel; }
            set { channel = value; }
        }
    }
}
