using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;

namespace lunabotics.OCU.ValidationRules
{
    class IntegerRule : ValidationRule
    {
        private int _min;
        private int _max;

        public IntegerRule()
        {
        }

        public int Min
        {
            get { return _min; }
            set { _min = value; }
        }

        public int Max
        {
            get { return _max; }
            set { _max = value; }
        }

        public override ValidationResult Validate(object value, System.Globalization.CultureInfo cultureInfo)
        {
            try
            {
                string number = value as String;

                if (String.IsNullOrWhiteSpace(number))
                    return new ValidationResult(false, "Field cannot be empty");

                int val = Int32.Parse(number);

                if (val < Min || val > Max)
                    return new ValidationResult(false, "Value is out of range: " + Min + " - " + Max);
            }
            catch (Exception ex)
            {
                return new ValidationResult(false, ex.Message);
            }

            return new ValidationResult(true, null);
        }
    }
}
