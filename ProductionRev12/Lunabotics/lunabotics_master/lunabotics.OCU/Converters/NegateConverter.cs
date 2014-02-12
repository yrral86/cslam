using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;

namespace lunabotics.OCU.Converters
{
    public class NegateConverter : IValueConverter
    {


        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value == null)
            {
                return null;
            }

            if (value is double)
            {
                return (-(double)value);
            }
            if (value is int)
            {
                return (-(int)value);
            }

            if (value is bool)
            {
                return (!(bool)value);
            }

            if (value is long)
            {
                return (-(long)value);
            }

            throw new ArgumentException("Unsupported type");
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return Convert(value, targetType, parameter, culture);
        }
    }

}
