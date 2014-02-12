using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    public class TelemetryConfiguration
    {
        public int UpdateInterval; //how often to send data to OCU
        public int TelemetryPort; //port to send telemtry on

        public CleanPowerAmpsConversion CleanPowerAmpsConversion;
        public CleanPowerVoltageConversion CleanPowerVoltageConversion;
        public PitchConversion PitchConversion;
        public PivotConversion PivotConversion;
    }

    public class CleanPowerAmpsConversion
    {
        public double MaxCurrent;
        public int MaxCurrentSensorValue;

        public double MinCurrent;
        public int MinCurrentSensorValue;

        public double Convert(int raw_value)
        {
            double sensor_step = (MaxCurrent - MinCurrent) / (double)(MaxCurrentSensorValue - MinCurrentSensorValue);
            return (MinCurrent + (raw_value - MinCurrentSensorValue) * sensor_step);
        }
    }

    public class CleanPowerVoltageConversion
    {
        public double MaxVoltage;
        public int MaxVoltageSensorValue;

        public double MinVoltage;
        public int MinVoltageSensorValue;

        public double Convert(int raw_value)
        {
            double sensor_step = (MaxVoltage - MinVoltage) / (double)(MaxVoltageSensorValue - MinVoltageSensorValue);
            return (MinVoltage + (raw_value - MinVoltageSensorValue) * sensor_step);
        }
    }

    public class PitchConversion
    {
        public int MinPitch;
        public int MinPitchSensorValue;

        public int MaxPitch;
        public int MaxPitchSensorValue;

        public double Convert(int raw_value)
        {
            double sensor_step = (MaxPitch - MinPitch) / (double)(MaxPitchSensorValue - MinPitchSensorValue);
            return (MinPitch + (raw_value - MinPitchSensorValue) * sensor_step);
        }
    }

    public class PivotConversion
    {
        public int MinPivot;
        public int MinPivotSensorValue;

        public int MaxPivot;
        public int MaxPivotSensorValue;

        public double Convert(int raw_value)
        {
            double sensor_step = (MaxPivot - MinPivot) / (double)(MaxPivotSensorValue - MinPivotSensorValue);
            return (MinPivot + (raw_value - MinPivotSensorValue) * sensor_step);
        }
    }
}
