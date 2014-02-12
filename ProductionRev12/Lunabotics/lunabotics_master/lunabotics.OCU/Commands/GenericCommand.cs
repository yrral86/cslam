using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.OCU.Commands
{
    public class GenericCommand : Base.BaseCommand
    {
        public GenericCommand(Action<object> execute, Predicate<object> pred = null)
        {
            base.RegisterCommand(execute, pred);
        }
    }
}
