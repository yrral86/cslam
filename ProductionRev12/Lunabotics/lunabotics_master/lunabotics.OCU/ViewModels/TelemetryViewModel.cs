using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using lunabotics.OCU.Components;

namespace lunabotics.OCU.ViewModels
{
    public sealed class TelemetryViewModel : Base.BaseINotifyPropertyChanged
    {
        private static readonly int MAX_VOLTAGE_GRAPH_DATA = 120;
        private static readonly int MAX_POWER_GRAPH_DATA = 1000;

        private static TelemetryViewModel instance;
        private static readonly object instanceSync = new object();

        private int lastSecond = -1;
        private double dbVoltageSum = 0, cbVoltageSum = 0;
        private int dbVoltageCount = 0, cbVoltageCount = 0;
        private double beltMotorAmps = 0;

        //Jason Added 5-19-13-9:44
        private double tiltX = 0, tiltY = 0;
        private double binLeftMotorAmps = 0, binRightMotorAmps = 0;
        private bool binLowerSwitchDepressed = false, binUpperSwitchDepressed = false;
        private double x = 0, y = 0, phi = 0, confidence = 0;
        private int state = 0;
        private double rearProximityLeft = 0, rearProximityRight = 0,proximityBarLeft=0,proximityBarRight=0;

        private double pivotAngle = 0, pitchAngle = 0;
        private bool bucketUpperLimitSwitch = false, bucketLowerLimitSwitch = false;

        private Models.TelemetryReceiver telemetryReceiver;
        private bool isActive = false;

        private Models.ObservableNotifiableCollection<Models.Point> frontRFPoints, rearRFPoints;

        private ObservableCollection<KeyValuePair<double,double>> driveBatteryData;
        private ObservableCollection<KeyValuePair<double,double>> cleanBatteryData;
        private ObservableCollection<KeyValuePair<double, double>> powerUsedData;

        private BatteryInfo driveBatteryInfo = new BatteryInfo();
        private BatteryInfo cleanBatteryInfo = new BatteryInfo();

        /*drive battery levels*/
        private static double DB_GOOD_VOLTAGE = 38.5;
        private static double DB_OK_VOLTAGE = 37;

        /*clean battery levels*/
        private static double CB_GOOD_VOLTAGE = 19.5;
        private static double CB_OK_VOLTAGE = 18.5;

        private double powerUsed = 0;

        private TelemetryViewModel()
        {
            driveBatteryData = new ObservableCollection<KeyValuePair<double, double>>();
            cleanBatteryData = new ObservableCollection<KeyValuePair<double, double>>();
            powerUsedData = new ObservableCollection<KeyValuePair<double, double>>();

            frontRFPoints = new Models.ObservableNotifiableCollection<Models.Point>();
            rearRFPoints = new Models.ObservableNotifiableCollection<Models.Point>();
            Activate();
        }

        public static TelemetryViewModel Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (instanceSync)
                    {
                        if (instance == null)
                            instance = new TelemetryViewModel();
                    }
                }
                return instance;
            }
        }

        public ObservableCollection<KeyValuePair<double,double>> DriveBatteryData
        {
            get { return driveBatteryData; }
        }

        public ObservableCollection<KeyValuePair<double, double>> CleanBatteryData
        {
            get { return cleanBatteryData; }
        }

        public ObservableCollection<KeyValuePair<double, double>> PowerUsedData
        {
            get { return powerUsedData; }
        }

        public Models.ObservableNotifiableCollection<Models.Point> FrontRFPoints
        {
            get { return frontRFPoints; }
            private set
            {
                frontRFPoints = value;
                OnPropertyChanged("FrontRFPoints");
            }
        }

        public Models.ObservableNotifiableCollection<Models.Point> RearRFPoints
        {
            get { return rearRFPoints; }
            private set
            {
                rearRFPoints = value;
                OnPropertyChanged("RearRFPoints");
            }
        }

        public double ProximityBar1
        {
            get { return proximityBarLeft; }
            set
            {
                if (proximityBarLeft != value)
                {
                    proximityBarLeft = value;
                    OnPropertyChanged("ProximityBar1");
                }
            }
        }

        public double ProximityBar2
        {
            get { return proximityBarRight; }
            set
            {
                if (proximityBarRight != value)
                {
                    proximityBarRight = value;
                    OnPropertyChanged("ProximityBar2");
                }
            }
        }



        public double BeltMotorAmps
        {
            get { return beltMotorAmps; }
            set
            {
                if (beltMotorAmps != value)
                {
                    beltMotorAmps = value;
                    OnPropertyChanged("BeltMotorAmps");
                }
            }
        }

        public double TiltX
        {
            get { return tiltX; }
            set
            {
                if (tiltX != value)
                {
                    tiltX= value;
                    OnPropertyChanged("TiltX");
                }
            }
        }


        public double TiltY
        {
            get { return tiltY; }
            set
            {
                if (tiltY != value)
                {
                    tiltY = value;
                    OnPropertyChanged("TiltY");
                }
            }
        }


        public double BinLeftMotorAmps
        {
            get { return binLeftMotorAmps; }
            set
            {
                if (binLeftMotorAmps != value)
                {
                    binLeftMotorAmps = value;
                    OnPropertyChanged("BinLeftMotorAmps");
                }
            }
        }

        public double BinRightMotorAmps
        {
            get { return binRightMotorAmps; }
            set
            {
                if (binRightMotorAmps != value)
                {
                    binRightMotorAmps = value;
                    OnPropertyChanged("BinRightMotorAmps");
                }
            }
        }

        public bool BinLowerSwitchDepressed
        {
            get { return binLowerSwitchDepressed; }
            set
            {
                if (binLowerSwitchDepressed != value)
                {
                    binLowerSwitchDepressed = value;
                    OnPropertyChanged("BinLowerSwitchDepressed");
                }
            }
        }

        public bool BinUpperSwitchDepressed
        {
            get { return binUpperSwitchDepressed; }
            set
            {
                if (binUpperSwitchDepressed != value)
                {
                    binUpperSwitchDepressed = value;
                    OnPropertyChanged("BinUpperSwitchDepressed");
                }
            }
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

        public double Phi
        {
            get { return phi; }
            set
            {
                if (phi != value)
                {
                    phi = value;
                    OnPropertyChanged("Phi");
                }
            }
        }

        public double Confidence
        {
            get { return confidence; }
            set
            {
                if (confidence != value)
                {
                    confidence = value;
                    OnPropertyChanged("Confidence");
                }
            }
        }

        public int State
        {
            get { return state; }
            set
            {
                if (state != value)
                {
                    state = value;
                    OnPropertyChanged("State");
                }
            }
        }

        public double RearProximityLeft
        {
            get { return rearProximityLeft; }
            set
            {
                if (rearProximityLeft != value)
                {
                    rearProximityLeft = value;
                    OnPropertyChanged("RearProximityLeft");
                }
            }
        }


        public double RearProximityRight
        {
            get { return rearProximityRight; }
            set
            {
                if (rearProximityRight != value)
                {
                    rearProximityRight = value;
                    OnPropertyChanged("RearProximityRight");
                }
            }
        }

        public double BucketPivotAngle
        {
            get { return pivotAngle; }
            private set
            {
                if (pivotAngle != value)
                {
                    pivotAngle = Math.Min(130,Math.Max(0,Math.Round(value, 0)));
                    OnPropertyChanged("BucketPivotAngle");
                    OnPropertyChanged("BucketGraphicPivotAngle");
                }
            }
        }

        public double BucketGraphicPivotAngle
        {
            get { return 130 - BucketPivotAngle; }
        }

        public bool BucketUpperLimitSwitch
        {
            get { return bucketUpperLimitSwitch; }
            private set
            {
                if (bucketUpperLimitSwitch != value)
                {
                    bucketUpperLimitSwitch = value;
                    OnPropertyChanged("BucketUpperLimitSwitch");
                }
            }
        }

        public bool BucketLowerLimitSwitch
        {
            get { return bucketLowerLimitSwitch; }
            private set
            {
                if (bucketLowerLimitSwitch != value)
                {
                    bucketLowerLimitSwitch = value;
                    OnPropertyChanged("BucketLowerLimitSwitch");
                }
            }
        }

        public double BucketPitchAngle
        {
            get { return pitchAngle; }
            private set
            {
                if (pitchAngle != value)
                {
                    pitchAngle = Math.Min(15, Math.Max(0, Math.Round(value, 0)));
                    OnPropertyChanged("BucketPitchAngle");
                }
            }
        }

        public BatteryInfo DriveBatteryInfo
        {
            get { return driveBatteryInfo; }
        }

        public BatteryInfo CleanBatteryInfo
        {
            get { return cleanBatteryInfo; }
        }

        public double PowerUsedRecent
        {
            get { return powerUsed; }
            set
            {
                if (powerUsed != value)
                {
                    powerUsed = value;
                    OnPropertyChanged("PowerUsedRecent");
                    Console.WriteLine(powerUsed);
                }
            }
        }

        public bool IsActive
        {
            get { return isActive; }
            private set
            {
                if (isActive != value)
                {
                    isActive = value;
                    OnPropertyChanged("IsActive");
                }
            }
        }

        public void Activate()
        {
            if (IsActive)
                return;

            telemetryReceiver = new Models.TelemetryReceiver(Properties.Settings.Default.TelemetryPort);
            telemetryReceiver.TelemetryReceived += new EventHandler<Models.TelemetryReceivedEventArgs>(telemetryReceiver_TelemetryReceived);
            telemetryReceiver.Activate();
            IsActive = true;
        }

        public void DeActivate()
        {
            if (!IsActive)
                return;

            telemetryReceiver.TelemetryReceived -= telemetryReceiver_TelemetryReceived;
            telemetryReceiver.Deactivate();
            telemetryReceiver.Dispose();
            telemetryReceiver = null;
            IsActive = false;
        }

        void telemetryReceiver_TelemetryReceived(object sender, Models.TelemetryReceivedEventArgs e)
        {
            try
            {
                //track battery voltages
                dbVoltageSum += e.Telemetry.DriveBatteryVoltage;
                cbVoltageSum += e.Telemetry.CleanBatteryVoltage;
                ++dbVoltageCount;
                ++cbVoltageCount;
                
                double seconds = System.DateTime.Now.TimeOfDay.TotalSeconds;
                int currentSecond = (int)Math.Floor(seconds);
                if (currentSecond != lastSecond) //only track graph data once per second
                { //add to graph data
                    App.Current.Dispatcher.Invoke((Action)(() =>
                    {
                        driveBatteryInfo.Voltage = e.Telemetry.DriveBatteryVoltage;
                        cleanBatteryInfo.Voltage = e.Telemetry.CleanBatteryVoltage;
                        PowerUsedRecent = e.Telemetry.PowerUsed;

                        if (e.Telemetry.DriveBatteryVoltage > DB_GOOD_VOLTAGE)
                            driveBatteryInfo.Status = BatteryStatus.Good;
                        else if (e.Telemetry.DriveBatteryVoltage > DB_OK_VOLTAGE)
                            driveBatteryInfo.Status = BatteryStatus.Ok;
                        else
                            driveBatteryInfo.Status = BatteryStatus.Critical;

                        if (driveBatteryData.Count >= MAX_VOLTAGE_GRAPH_DATA)
                            driveBatteryData.RemoveAt(0);
                        driveBatteryData.Add(new KeyValuePair<double, double>(seconds, e.Telemetry.DriveBatteryVoltage));

                        if (e.Telemetry.CleanBatteryVoltage > CB_GOOD_VOLTAGE)
                            cleanBatteryInfo.Status = BatteryStatus.Good;
                        else if (e.Telemetry.CleanBatteryVoltage > CB_OK_VOLTAGE)
                            cleanBatteryInfo.Status = BatteryStatus.Ok;
                        else
                            cleanBatteryInfo.Status = BatteryStatus.Critical;

                        if (cleanBatteryData.Count >= MAX_VOLTAGE_GRAPH_DATA)
                            cleanBatteryData.RemoveAt(0);
                        cleanBatteryData.Add(new KeyValuePair<double, double>(seconds, e.Telemetry.CleanBatteryVoltage));
                        if (powerUsedData.Count >= MAX_POWER_GRAPH_DATA)
                            powerUsedData.RemoveAt(0);
                        powerUsedData.Add(new KeyValuePair<double, double>(seconds, e.Telemetry.PowerUsed));
                    }), new TimeSpan(0,0,1));

                    lastSecond = currentSecond;
                    //reset averaging
                    dbVoltageCount = 0;
                    dbVoltageSum = 0;
                    cbVoltageCount = 0;
                    cbVoltageSum = 0;
                }

                //*other telemetry*/
                BeltMotorAmps = e.Telemetry.ArmMotorAmps;
                BucketPitchAngle = e.Telemetry.ScoopPitchAngle;
                BucketPivotAngle = e.Telemetry.ArmSwingAngle;
                BucketLowerLimitSwitch = e.Telemetry.ScoopLowerLimitSwitchDepressed;
                BucketUpperLimitSwitch = e.Telemetry.ScoopUpperLimitSwitchDepressed;

                
                
                // Tilt Telemetry //
                TiltX = e.Telemetry.TiltX;
                TiltY = e.Telemetry.TiltY;
                
                // Localization //
                X = e.Telemetry.X;
                Y = e.Telemetry.Y;
                Phi = e.Telemetry.Psi;
                State = e.Telemetry.State;

                // Rear Proximity Sensors //
                
                
                RearProximityLeft = e.Telemetry.RearProximityLeft;
                RearProximityRight = e.Telemetry.RearProximityRight;

                ProximityBar1 = e.Telemetry.RearProximityLeft;
                ProximityBar2 = e.Telemetry.RearProximityRight;


                // Collection Bin States // 
                BinLeftMotorAmps = e.Telemetry.BinLeftMotorAmps; // From 'driver' perspective
                BinRightMotorAmps = e.Telemetry.BinRightMotorAmps;
                BinLowerSwitchDepressed = e.Telemetry.BinLowerSwitchDepressed;
                BinUpperSwitchDepressed = e.Telemetry.BinUpperSwitchDepressed;                 
                 
                                

                //process range finder
                //System.Diagnostics.Debug.WriteLine("Rear RF Data Length: " + e.Telemetry.RearRangeFinderDataLength);
                if (e.Telemetry.RearRangeFinderDataLength != 0)
                {
                    double angle = Properties.RangeFinderSettings.Default.RearStartAngle;
                    Models.ObservableNotifiableCollection<Models.Point> newPoints = new Models.ObservableNotifiableCollection<Models.Point>();

                    for (int i = 0; i < e.Telemetry.RearRangeFinderDataLength && i < Properties.RangeFinderSettings.Default.RearDataLength; ++i, angle += Properties.RangeFinderSettings.Default.RearStepAngle)
                    {
                        double dist = Math.Min(1000, e.Telemetry.RearRangeFinderData[i]);
                        newPoints.Add(new Models.Point()
                        {
                            X = dist * Math.Cos(angle * Math.PI / 180),
                            Y = dist * Math.Sin(angle * Math.PI / 180)
                        });
                    }

                    App.Current.Dispatcher.Invoke((Action)(() => { RearRFPoints = newPoints; }), new TimeSpan(0,0,1));
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }
    }

    public class BatteryInfo : Base.BaseINotifyPropertyChanged
    {
        private BatteryStatus status;
        private double voltage;

        public BatteryInfo()
        {
            this.status = BatteryStatus.Unknown;
            this.voltage = 0;
        }

        public BatteryStatus Status
        {
            get { return status; }
            internal set
            {
                if (status != value)
                {
                    status = value;
                    OnPropertyChanged("Status");
                }
            }
        }
        public double Voltage
        {
            get { return voltage; }
            internal set
            {
                if (voltage != value)
                {
                    voltage = value;
                    OnPropertyChanged("Voltage");
                }
            }
        }
    }
}
