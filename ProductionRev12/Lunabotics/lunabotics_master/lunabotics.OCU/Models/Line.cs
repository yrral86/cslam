using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.OCU.Models
{
    public class Line : lunabotics.OCU.Base.BaseINotifyPropertyChanged
    {
        Models.Point _start, _end;

        public Line()
        {
        }

        public Line(Point start, Point end)
        {
            _start = start;
            _end = end;
        }

        public Point Start
        {
            get { return _start; }
            set
            {
                if(_start != value)
                {
                    _start = value;
                    OnPropertyChanged("Start");
                }
            }
        }

        public Point End
        {
            get { return _end; }
            set
            {
                if (_end != value)
                {
                    _end = value;
                    OnPropertyChanged("End");
                }
            }
        }
    }
}
