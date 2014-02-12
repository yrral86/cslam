using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.OCU.Models
{
    public class Point : Base.BaseINotifyPropertyChanged
    {
        double x, y;

        public Point() : this(0,0)
        {
        }

        public Point(double x, double y)
        {
            this.X = x;
            this.Y = y;
        }

        public double X
        {
            get { return x; }
            set 
            {
                if (x != value)
                {
                    x = value;
                    OnPropertyChanged("X");
                }
            }
        }

        public double Y
        {
            get { return y; }
            set 
            {
                if (y != value)
                {
                    y = value;
                    OnPropertyChanged("Y");
                }
            }
        }

    }
}
