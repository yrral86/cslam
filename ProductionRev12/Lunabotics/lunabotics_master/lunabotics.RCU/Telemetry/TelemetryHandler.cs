using lunabotics.Comms.TelemetryEncoding;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.RCU.Telemetry
{
    public class TelemetryHandler
    {
        public enum RangeFinderLocation
        {
            Front,
            Rear
        }
        
        //Hall Counter Variables
        public static double Robo1HallCount1;
        public static double Robo1HallCount2;
        public static double Robo2HallCount1;
        public static double Robo2HallCount2;

        // Telemetry received event
        public event EventHandler<TelemetryFeedbackArgs> TelemetryFeedbackProcessed;

        private static readonly double MS_IN_HOUR = 3600000;

        private static readonly double WATT_SCALAR = 100000;

        private Configuration.TelemetryConfiguration configuration;
        private System.Timers.Timer reporter;
        private readonly object reporterSync = new object();
        private Comms.UDP_Sender udp_sender;

        private Models.RangeFinder front_rf = null, rear_rf = null;

        //Power used is stored as integer (watts * 100000) to eliminate need for locks
        private uint power_used = 0;

        /*variables to store raw battery values (not actual values! each requires conversion)
            No power used will register until after the first battery values are obtained.
            i.e., amps will be ignored until battery voltage is known
         * 
         * Voltage is stored as an integer of Voltage*10 to remove need for locks and make things faster
         */
        private int cleanBatteryVoltage = 0;
        private int robo1BatteryVoltage = 0;
        private int robo2BatteryVoltage = 0;
        private int robo3BatteryVoltage = 0;
        private int robo4BatteryVoltage = 0;

        private int robo1Mot1Amps = 0;
        private int robo1Mot2Amps = 0;
        private int robo2Mot1Amps = 0;
        private int robo2Mot2Amps = 0;

        //stored as (int)(Amps*10)
        private int bucketLeftAmps;
        private int bucketRightAmps;

        // Sensors
        private double rearProximityLeft = 30;
        private double rearProximityRight = 30;

        // Localization
        private double x = 0;
        private double y = 0;
        private double psi = 0;
        private int state = 0;
        
        private bool ScoopLowerLimitSwitch = false;
        private bool ScoopUpperLimitSwitch = false;

        private double ScoopArmAmps = 0;
        private double ScoopPitch = 0;
        private double ScoopArmAngle = 0;
        private double BucketAngle = 0;
        //private double LeftScoopActuatorFeedback = 0;
        //private double RightScoopActuatorFeedback = 0;


        public TelemetryHandler(Configuration.TelemetryConfiguration config, string ocu_ip_address)
        {
            if (config == null)
                throw new ArgumentNullException("config");

            this.configuration = config;

            reporter = new System.Timers.Timer();
            reporter.Interval = config.UpdateInterval;
            reporter.Elapsed += new System.Timers.ElapsedEventHandler(reporter_Elapsed);
            udp_sender = new Comms.UDP_Sender(ocu_ip_address, config.TelemetryPort, config.UpdateInterval);
        }

        public void Activate()
        {
            reporter.Enabled = true;
        }

        public void Deactivate()
        {
            reporter.Enabled = false;
        }

        public bool IsActive
        {
            get { return reporter.Enabled; }
        }

        void reporter_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            if (System.Threading.Monitor.TryEnter(reporterSync)) //re-entrace protection
            {
                try
                {
                    //compile all data
                    Comms.TelemetryEncoding.TelemetryFeedback feedback = new Comms.TelemetryEncoding.TelemetryFeedback()
                    {
                        // Voltages
                        DriveBatteryVoltage = this.DriveBatteryVoltage,
                        CleanBatteryVoltage = this.CleanBatteryVoltage,
                        PowerUsed = this.PowerUsed,

                        //Motor Amps
                        RightFrontMotorAmps = this.Robo1Mot1Amps,
                        RightRearMotorAmps = this.Robo1Mot2Amps,
                        LeftFrontMotorAmps = this.Robo2Mot1Amps,
                        LeftRearMotorAmps = this.Robo2Mot2Amps,

                        // Mining
                        ArmMotorAmps = this.ScoopArmAmps,
                        ScoopPitchAngle = this.ScoopPitch,
                        ArmSwingAngle = this.ScoopArmAngle,
                        ScoopLowerLimitSwitchDepressed = this.ScoopLowerLimitSwitch,
                        ScoopUpperLimitSwitchDepressed = this.ScoopUpperLimitSwitch,

                        // Deposition
                        BucketPivotAngle = this.BucketPivotAngle,

                        // Sensory
                        RearProximityLeft = this.rearProximityLeft,
                        RearProximityRight = this.rearProximityRight,
                        
                        // Localization
                        X = this.X,
                        Y = this.Y,
                        Psi = this.Psi,
                        State = this.State
                    };

                    ////todo : store rf timing in telemetry
                    //int timing;
                    
                    //if (front_rf != null)
                    //    feedback.FrontRangeFinderData = front_rf.CopyData(null, out feedback.FrontRangeFinderDataLength, out timing);

                    //if (rear_rf != null)
                    //    feedback.RearRangeFinderData = rear_rf.CopyData(null, out feedback.RearRangeFinderDataLength, out timing);

                    // Raise event for telemetry processed
                    OnTelemetryFeedbackProcessed(new TelemetryFeedbackArgs(feedback));

                    //send
                    udp_sender.Send(feedback.Encode());
                }
                catch (TimeoutException ex)
                {
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
                finally
                {
                    System.Threading.Monitor.Exit(reporterSync);
                }
            }
        }

        //Add generic provider
        public void AddProvider(IProvideTelemetry provider)
        {
            if (provider == null)
                throw new ArgumentNullException("provider");

            provider.TelemetryUpdated += new EventHandler<TelemetryEventArgs>(provider_TelemetryUpdated);
        }

        public void AddProviders(IEnumerable<IProvideTelemetry> providers)
        {
            if (providers == null)
                throw new ArgumentNullException("providers");

            foreach (IProvideTelemetry provider in providers)
                AddProvider(provider);
        }

        public void AddRangeFinder(RangeFinderLocation location, Models.RangeFinder rangefinder)
        {
            if (rangefinder == null)
                throw new ArgumentNullException("rangefinder");

            switch (location)
            {
                case RangeFinderLocation.Front:
                    if (front_rf == null)
                        front_rf = rangefinder;
                    else
                        throw new InvalidOperationException("Front rangefinder already set");
                    break;

                case RangeFinderLocation.Rear:
                    if (rear_rf == null)
                        rear_rf = rangefinder;
                    else
                        throw new InvalidOperationException("Rear rangefinder already set");
                    break;
            }
        }

        private void OnTelemetryFeedbackProcessed(TelemetryFeedbackArgs args)
        {
            if (TelemetryFeedbackProcessed != null)
                TelemetryFeedbackProcessed(this, args);
        }

        void provider_TelemetryUpdated(object sender, TelemetryEventArgs e)
        {
            try
            {
                foreach (Configuration.Telemetry telem in e.UpdatedTelemetry.Keys)
                {
                    switch (telem)
                    {
                        case Configuration.Telemetry.RoboteQ_1_MotorAmpsCh1:
                            robo1Mot1Amps = e.UpdatedTelemetry[telem];
                            UpdatePowerUsed(Robo1BatteryVoltage, robo1Mot1Amps / 10.0, e.Interval);
                            break;
                        case Configuration.Telemetry.RoboteQ_1_MotorAmpsCh2:
                            robo1Mot2Amps = e.UpdatedTelemetry[telem];
                            UpdatePowerUsed(Robo1BatteryVoltage, robo1Mot2Amps / 10.0, e.Interval);
                            break;
                        case Configuration.Telemetry.RoboteQ_1_HallCountCh1:
                            Robo1HallCount1 = Math.Abs(e.UpdatedTelemetry[telem]);
                            break;
                        case Configuration.Telemetry.RoboteQ_1_HallCountCh2:
                            Robo1HallCount2 = Math.Abs(e.UpdatedTelemetry[telem]);
                            break;
                        case Configuration.Telemetry.RoboteQ_1_BatteryVoltage:
                            robo1BatteryVoltage = e.UpdatedTelemetry[telem];
                            break;
                        case Configuration.Telemetry.RoboteQ_2_MotorAmpsCh1:
                            robo2Mot1Amps = e.UpdatedTelemetry[telem];
                            UpdatePowerUsed(Robo2BatteryVoltage, robo2Mot1Amps / 10.0, e.Interval);
                            break;
                        case Configuration.Telemetry.RoboteQ_2_MotorAmpsCh2:
                            robo2Mot2Amps = e.UpdatedTelemetry[telem];
                            UpdatePowerUsed(Robo2BatteryVoltage, robo2Mot2Amps / 10.0, e.Interval);
                            break;
                        case Configuration.Telemetry.RoboteQ_2_HallCountCh1:
                            Robo2HallCount1 = Math.Abs(e.UpdatedTelemetry[telem]);
                            break;
                        case Configuration.Telemetry.RoboteQ_2_HallCountCh2:
                            Robo2HallCount2 = Math.Abs(e.UpdatedTelemetry[telem]);
                            break;
                        case Configuration.Telemetry.RoboteQ_2_BatteryVoltage:
                            robo2BatteryVoltage = e.UpdatedTelemetry[telem];
                            break;

                        case Configuration.Telemetry.RoboteQ_3_MotorAmpsCh1:
                            UpdatePowerUsed(Robo3BatteryVoltage, e.UpdatedTelemetry[telem] / 10.0, e.Interval);
                            break;
                        case Configuration.Telemetry.RoboteQ_3_MotorAmpsCh2:
                            ScoopArmAmps = e.UpdatedTelemetry[telem];
                            UpdatePowerUsed(Robo3BatteryVoltage, ScoopArmAmps / 10.0, e.Interval);
                            break;

                        case Configuration.Telemetry.RoboteQ_3_BatteryVoltage:
                            robo3BatteryVoltage = e.UpdatedTelemetry[telem];
                            break;

                        case Configuration.Telemetry.RoboteQ_4_MotorAmpsCh1:
                            bucketLeftAmps = e.UpdatedTelemetry[telem];
                            UpdatePowerUsed(Robo4BatteryVoltage, bucketLeftAmps / 10.0, e.Interval);
                            break;
                        case Configuration.Telemetry.RoboteQ_4_MotorAmpsCh2:
                            bucketRightAmps = e.UpdatedTelemetry[telem];
                            UpdatePowerUsed(Robo4BatteryVoltage, bucketRightAmps / 10.0, e.Interval);
                            break;

                        case Configuration.Telemetry.RoboteQ_4_BatteryVoltage:
                            robo4BatteryVoltage = e.UpdatedTelemetry[telem];
                            break;

                        case Configuration.Telemetry.CleanPowerBatteryAmps:
                            UpdatePowerUsed(18.0, configuration.CleanPowerAmpsConversion.Convert(e.UpdatedTelemetry[telem]) / 20.0, e.Interval);
                            //UpdatePowerUsed(CleanBatteryVoltage, configuration.CleanPowerAmpsConversion.Convert(e.UpdatedTelemetry[telem]), e.Interval);
                            break;
                        case Configuration.Telemetry.CleanPowerBatteryVoltage:
                            //scaling by ten and storing as integer
                            cleanBatteryVoltage = (int)((e.UpdatedTelemetry[telem]/128.5) * 10);
                            break;

                        case Configuration.Telemetry.LeftScoopActuatorFeedback:
                            //Scoop Actuator Pot Voltage
                            //4965 at 0 degrees
                            //785 at 10 degrees
                            ScoopPitch = Math.Abs((int)((e.UpdatedTelemetry[telem] - 785) / 418) - 10);
                            break;
                        case Configuration.Telemetry.RightScoopActuatorFeedback:
                            break;
                        case Configuration.Telemetry.ScoopArmAngleRaw:
                            //Scoop Arm Pot Volatage 
                            //1145 at 145 degrees (top)
                            //5088 at 0 degrees (bottom)
                            ScoopArmAngle = Math.Abs((int)((e.UpdatedTelemetry[telem] - 1145) / 27.2) - 145);
                            break;
                        case Configuration.Telemetry.BucketAngleRaw:
                            //Bucket Angle Pot Voltage
                            //3700 at 0 degrees (bottom)
                            //2380 at 90 degrees (top)
                            BucketAngle = (int)((e.UpdatedTelemetry[telem] - 3700) / -14.6);
                            break;
                        case Configuration.Telemetry.ArmLowerLimitSwitch:
                            ScoopLowerLimitSwitch = (e.UpdatedTelemetry[telem] > 0 ? true : false);
                            break;

                        case Configuration.Telemetry.ArmUpperLimitSwitch:
                            ScoopUpperLimitSwitch = (e.UpdatedTelemetry[telem] > 0 ? true : false);
                            break;
                        case Configuration.Telemetry.RearRightIR:
                            rearProximityRight = Math.Min(80, (6787 / (e.UpdatedTelemetry[telem] - 3)) - 4);
                            break;
                        case Configuration.Telemetry.RearLeftIR:
                            rearProximityLeft = Math.Min(80, (6787 / (e.UpdatedTelemetry[telem] - 3)) - 4);
                            break;
                        case Configuration.Telemetry.LocalizationX:
                            x = e.UpdatedTelemetry[telem];
                            break;
                        case Configuration.Telemetry.LocalizationY:
                            y = e.UpdatedTelemetry[telem];
                            break;
                        case Configuration.Telemetry.LocalizationPsi:
                            psi = e.UpdatedTelemetry[telem];
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (Exception) { }//fail silently
        }

        #region accessors

        //tracks Watt hours
        public double PowerUsed
        {
            get
            {
                return power_used / WATT_SCALAR;
            }
        }

        //averages roboteq readings
        public double DriveBatteryVoltage
        {
            get
            {
                return (robo1BatteryVoltage + robo2BatteryVoltage + robo3BatteryVoltage + robo4BatteryVoltage) / 40.0;
            }
        }

        public double CleanBatteryVoltage
        {
            get
            {
                return cleanBatteryVoltage / 10.0;
            }
        }

        public double Robo1BatteryVoltage
        {
            get
            {
                return robo1BatteryVoltage / 10.0;
            }
        }

        public double Robo2BatteryVoltage
        {
            get
            {
                return robo2BatteryVoltage / 10.0;
            }
        }

        public double Robo3BatteryVoltage
        {
            get
            {
                return robo3BatteryVoltage / 10.0;
            }
        }

        public double Robo4BatteryVoltage
        {
            get
            {
                return robo4BatteryVoltage / 10.0;
            }
        }

        public double Robo1Mot1Amps
        {
            get
            {
                return robo1Mot1Amps / 10;
            }
        }

        public double Robo1Mot2Amps
        {
            get
            {
                return robo1Mot2Amps / 10;
            }
        }

        public double Robo2Mot1Amps
        {
            get
            {
                return robo2Mot1Amps / 10;
            }
        }

        public double Robo2Mot2Amps
        {
            get
            {
                return robo2Mot2Amps / 10;
            }
        }

        public double BucketPivotAngle
        {
            get { return BucketAngle; }
        }

        public double ScoopPitchAngle
        {
            get { return ScoopPitch; }
        }

        public double ScoopPivotAngle
        {
            get { return ScoopArmAngle; }
        }

        public double X
        {
            get { return x; }
        }

        public double Y
        {
            get { return y; }
        }

        public double Psi
        {
            get { return psi; }
        }

        public int State
        {
            get { return state; }
        }
        #endregion

        #region Processing

        //adds power consumed based on voltage, amperage, and time (ms)
        private void UpdatePowerUsed(double voltage, double amps, int time)
        {
            //may be negative, depending on movement of motors (negative voltage when in reverse). have to use abs. value
            double additional_power = voltage * amps * (time / MS_IN_HOUR);
            power_used += (uint)Math.Abs(additional_power * WATT_SCALAR);
        }

        #endregion

    }
}
