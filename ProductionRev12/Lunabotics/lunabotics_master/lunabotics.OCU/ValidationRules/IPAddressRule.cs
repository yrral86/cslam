using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;

namespace lunabotics.OCU.ValidationRules
{
    public class IPAddressRule : ValidationRule
    {
        public override ValidationResult Validate(object value, System.Globalization.CultureInfo cultureInfo)
        {
            string ip_string = value as String;
            if (String.IsNullOrWhiteSpace(ip_string))
                return new ValidationResult(false, "Field cannot be empty");

            try
            {
                var ip = System.Net.IPAddress.Parse(((string)value));
            }
            catch (Exception ex)
            {
                return new ValidationResult(false, ex.Message);
            }

            return new ValidationResult(true, null);
        }
    }
}
