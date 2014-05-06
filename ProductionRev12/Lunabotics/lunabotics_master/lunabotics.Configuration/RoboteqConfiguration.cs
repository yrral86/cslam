using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    [Serializable]
    public class RoboteqConfiguration
    {
        public RoboteqConfiguration()
        {
        }

        public string Name;

        public string COM_Port;

        public int Timeout;

        public List<RoboteqOutputSettings> OutputSettings;

        public RoboteqInputSettings InputSettings;
    }
}
