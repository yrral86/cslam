using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    public class RangeFinderConfiguration
    {
        public string Port;
        public int StartStep;
        public int EndStep;
        public int ClusterCount;
        public double ScalingIntercept;
        public double ScalingSlope;
    }
}
