using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Globalization;

namespace lunabotics.OCU.Converters
{
    public class SecondsToTimeStringConverter : IValueConverter
    {

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            double seconds = (double)value;

            int hours = (int)Math.Floor(seconds/3600);
            seconds -= hours * 3600;

            int minutes = (int)Math.Floor(seconds/60);
            seconds -= minutes * 60;

            return (hours.ToString("D2") + ":" + minutes.ToString("D2") + ":" + ((int)(seconds)).ToString("D2"));

        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
