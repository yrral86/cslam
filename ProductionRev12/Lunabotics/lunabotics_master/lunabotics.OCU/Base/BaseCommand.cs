using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace lunabotics.OCU.Base
{
    public abstract class BaseCommand : ICommand
    {
        Action<object> toExecute = null;
        Predicate<object> condition = null;

        public void RegisterCommand(Action<object> execute, Predicate<object> pred = null)
        {
            toExecute = execute;
            condition = pred;
        }

        public bool CanExecute(object parameter)
        {
            if (toExecute == null)
                return false;

            return condition == null ? true : condition(parameter);
        }

        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }

        public void Execute(object parameter)
        {
            if (toExecute != null)
                toExecute(parameter);
        }
    }
}
