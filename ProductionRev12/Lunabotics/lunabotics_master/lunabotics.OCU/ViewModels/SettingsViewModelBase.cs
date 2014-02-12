using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace lunabotics.OCU.ViewModels
{
    public abstract class SettingsViewModelBase : Base.BaseINotifyPropertyChanged
    {
        public ICommand _save;

        public ICommand SaveCommand
        {
            get
            {
                if (_save == null)
                    _save = new Commands.GenericCommand(p => Save());

                return _save;
            }
        }

        public abstract void Save();
    }
}
