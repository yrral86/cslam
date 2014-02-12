using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Configuration;

namespace lunabotics.RCU.Controllers
{
    public interface IController
    {
        void Activate();
        void Deactivate();

        void SetValues(Dictionary<Devices, int> device_mapping);
    }
}
