using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Configuration
{
    public enum Telemetry
    {
        // RoboteQ voltage/amperage measurements
        RoboteQ_1_BatteryVoltage,
        RoboteQ_1_MotorAmpsCh1,
        RoboteQ_1_MotorAmpsCh2,
        RoboteQ_1_HallCountCh1,  //Added to count number of hall turns
        RoboteQ_1_HallCountCh2,

        RoboteQ_2_BatteryVoltage,
        RoboteQ_2_MotorAmpsCh1,
        RoboteQ_2_MotorAmpsCh2,
        RoboteQ_2_HallCountCh1,  //Added to count number of hall turns
        RoboteQ_2_HallCountCh2,

        RoboteQ_3_BatteryVoltage,
        RoboteQ_3_MotorAmpsCh1,
        RoboteQ_3_MotorAmpsCh2,

        RoboteQ_4_BatteryVoltage,
        RoboteQ_4_MotorAmpsCh1,
        RoboteQ_4_MotorAmpsCh2,

        // Clean battery health
        CleanPowerBatteryVoltage,
        CleanPowerBatteryAmps,

        // Digging
        ScoopPitchRawValue,
        ArmAngleRawValue,
        ArmLowerLimitSwitch,
        ArmUpperLimitSwitch,
        ArmMotorAmps,

        // Dumping
        BinLeftMotorAmps,
        BinRightMotorAmps,
        BinLowerLimitSwitch,
        BinUpperLimitSwitch,

        // Tilt sensor
        TiltX,
        TiltY,

        // Rear proximity sensors
        RearProximityLeft,
        RearProximityRight,

        // Localization
        LocalizationX,
        LocalizationY,
        LocalizationPsi,        
        LocalizationConfidence,

        // Autonomy
        LocalizationState,
    }
}
