using lunabotics.Comms.CommandEncoding;
using lunabotics.Comms.TelemetryEncoding;
using lunabotics.Configuration;
using lunabotics.RCU.Telemetry;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Timers;
using System.Threading;
using lunabotics.RCU.Autonomy.Algorithms;
using lunabotics.RCU.Models;
using lunabotics.RCU.Hokuyo;

namespace lunabotics.RCU.Autonomy
{
    public class AutonomyHandler : IProvideTelemetry
    {
        #region Enums
        public enum State
        {
            Manual,
            InitializeAutonomy,
            TraverseWithObstacleAvoidance,
            TraverseClearPath,
            ReturnToDeposition,
            Mining,
            Deposition,
            TemporaryTesting
        }
        public enum Zone
        {
            Start,
            Obstacle,
            Mining,
            Unknown
        }
        #endregion

        #region Events
        public event EventHandler<AutonomyArgs> AutonomyUpdated;
        public event EventHandler<Telemetry.TelemetryEventArgs> TelemetryUpdated;
        public int[] EthernetSensorData;
        #endregion

        #region Fields
        //Initialize motor powers (Note forward power is negative)
        public short forwardPower = -500;
        public short reversePower = 500;

        private bool active; // Thread started
        private bool started; // State set to run
        private readonly object timerSync = new object();
        private AutonomyConfiguration configuration;
        private Deposition depositionAlgorithm;
        private Dictionary<CommandFields, short> staticOutput;
        private Robot robot;
        private State state;
        private get_distance_ethernet utm = new get_distance_ethernet();

        private Stopwatch stopwatch;
        private System.Timers.Timer timer;
        private Zone expectedZone;
        // Create output state
        private Dictionary<CommandFields, short> outputState = new Dictionary<CommandFields, short>();

        #endregion

        #region Constructors
        public AutonomyHandler(AutonomyConfiguration configuration, ref TelemetryHandler telemetryHandler)
        {
            // Assign fields
            this.configuration = configuration;

            // Create robot
            robot = new Robot(configuration);

            

            // Create stopwatch
            stopwatch = new Stopwatch();

            // Add telemetry callback
            telemetryHandler.TelemetryFeedbackProcessed += telemetryHandler_TelemetryFeedbackProcessed;

            // Create timer
            timer = new System.Timers.Timer(configuration.Interval);
            timer.Elapsed += timer_Elapsed;

            // Create static state
            staticOutput = new Dictionary<CommandFields, short>();
            staticOutput[Comms.CommandEncoding.CommandFields.Mode] = 1;
            staticOutput[Comms.CommandEncoding.CommandFields.FrontCameraState] = 0;
            staticOutput[Comms.CommandEncoding.CommandFields.BucketCameraState] = 0;
            staticOutput[Comms.CommandEncoding.CommandFields.RearCameraState] = 0;                    
        }
        #endregion
        
        #region Methods
        public void Activate()
        {
            // Start timer
            timer.Enabled = true;

            // Set flag
            active = true;

        }

        public void Deactivate()
        {
            if (active)
            {
                // Disable timer
                timer.Enabled = false;

                //Set flag
                active = false;
            }
        }

        private Dictionary<CommandFields, short> MergeStates(Dictionary<CommandFields, short> left, Dictionary<CommandFields, short> right)
        {
            foreach (CommandFields commandField in right.Keys)
            {
                left[commandField] = right[commandField];
            }

            return left;
        }

        private void OnAutonomyUpdated(AutonomyArgs args)
        {
            if (AutonomyUpdated != null)
                AutonomyUpdated(this, args);
        }

        private void OnTelemetryUpdated(Telemetry.TelemetryEventArgs args)
        {
            if (TelemetryUpdated != null)
                TelemetryUpdated(this, args);
        }

        public void Start()
        {
            // Reset stopwatch
            stopwatch.Restart();
            // Set state
            state = State.TemporaryTesting;
            // Set flag
            started = true;
            // Set zone
            expectedZone = Zone.Start;

        }

        public void Stop()
        {
            // Set flags
            started = false;
            state = State.Manual;
            // Stop stopwatch
            stopwatch.Stop();

            Console.WriteLine("Stop");
        }

        void telemetryHandler_TelemetryFeedbackProcessed(object sender, Comms.TelemetryEncoding.TelemetryFeedbackArgs e)
        {
            // Obtain measurements and copy to robot object
            robot.TelemetryFeedback = (TelemetryFeedback) e.UpdatedState.Clone();
        }

        private void timer_Elapsed(object sender, ElapsedEventArgs e)
        {
                // Reentry protection
            if (System.Threading.Monitor.TryEnter(timerSync))
            {
                try
                {
                    // Process state
                    Thread.CurrentThread.Name = "AutonomyHandlerThread";
                    //Console.WriteLine(stopwatch.Elapsed);
                    switch (state)
                    {
                        case State.Manual:
                            // Should not be here, but just in case
                            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPitch] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RangeFinderServo] = 90;
                            break;

                        case State.TemporaryTesting:
                            EthernetSensorData = utm.EthernetScan();
                            for (int j = 0; j < 760; j++)
                            {
                                Console.WriteLine(EthernetSensorData[j].ToString());
                            }
                            //Thread.Sleep(200);
                            //tankTurnRight(1200);
                            //Thread.Sleep(200);
                            //tankTurnLeft(1200);
                            //Thread.Sleep(200);
                            //state = State.Mining;
                            break;

                        case State.Mining:
                            MoveReverse(1200);
                            Thread.Sleep(200);
                            MoveReverse(1200);
                            Thread.Sleep(200);
                            state = State.TemporaryTesting;
                            break;
                         
                        default:
                            break;
                    }
                
                    // Store robot velocities
                    //*******John - See if this works while it is commented out
                    //robot.SetVelocities(
                    //    Kinematics.GetTranslationalVelocity(outputState[CommandFields.TranslationalVelocity]),
                    //    Kinematics.GetRotationalVelocity(outputState[CommandFields.RotationalVelocity]));
                    


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
                    System.Threading.Monitor.Exit(timerSync);
                }
            }            
        }

        private void UpdateState()
        {

        }

        //private void UpdateTelemetry(MonteCarloLocalization localization)
        //{
        //    // Update telemetry
        //    Dictionary<Configuration.Telemetry, int> dictionary = new Dictionary<Configuration.Telemetry, int>();
        //    //dictionary[Configuration.Telemetry.LocalizationX] = (int)(localization.Tracker.X * 10.0d); // Convert for double to int -> Like RoboteQ
        //    //dictionary[Configuration.Telemetry.LocalizationY] = (int)(localization.Tracker.Y * 10.0d); // Convert for double to int -> Like RoboteQ
        //    //dictionary[Configuration.Telemetry.LocalizationPsi] = (int)(localization.Tracker.Angle * 10.0d); // Convert for double to int -> Like RoboteQ
        //    //dictionary[Configuration.Telemetry.LocalizationConfidence] = (int)(100.0 * localization.Confidence); // 0 = No confidence, 100 = Complete convergence/good confidence
        //    //dictionary[Configuration.Telemetry.LocalizationState] = (int)state;
        //    dictionary[Configuration.Telemetry.LocalizationX] = 10;
        //    dictionary[Configuration.Telemetry.LocalizationY] = 10;
        //    dictionary[Configuration.Telemetry.LocalizationPsi] = 90;
        //    dictionary[Configuration.Telemetry.LocalizationConfidence] = 100;
        //    dictionary[Configuration.Telemetry.LocalizationState] = (int)state;

        //    OnTelemetryUpdated(new TelemetryEventArgs((int)configuration.Interval, dictionary));
        //}


        //Send Hall count steps
        public void MoveForward(double steps)
        {
            //System.Diagnostics.Debug.WriteLine("Forward");
            while (
                Math.Abs(Telemetry.TelemetryHandler.Robo1HallCount2) < steps)
            {
                //Print Hall count for debugging
                System.Diagnostics.Debug.WriteLine(Math.Abs(Telemetry.TelemetryHandler.Robo1HallCount2));
                //Set Velocity of motor. Power between -1000 and 0
                outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = forwardPower;
                outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                //Set Output States
                outputState = MergeStates(outputState, staticOutput);
                //Make calls to event handler to move motors
                OnAutonomyUpdated(new AutonomyArgs(outputState));
            }
            //Stop motors
            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
            outputState = MergeStates(outputState, staticOutput);
            OnAutonomyUpdated(new AutonomyArgs(outputState));
            //Reset Hall count for next motor command
            ResetCounters();
        }


        //Send Hall count steps
        public void MoveReverse(double steps)
        {
            //System.Diagnostics.Debug.WriteLine("Reverse");
            while (
               Math.Abs(Telemetry.TelemetryHandler.Robo1HallCount2) < steps)
            {
                //Print Hall count for debugging
                //System.Diagnostics.Debug.WriteLine(Math.Abs(Telemetry.TelemetryHandler.Robo1HallCount2));

                //Set Velocity of motor. Power between 0 and 1000
                outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = reversePower;
                outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                //Set Output States
                outputState = MergeStates(outputState, staticOutput);
                //Make calls to event handler to move motors
                OnAutonomyUpdated(new AutonomyArgs(outputState));
            }
            //Stop motors
            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
            outputState = MergeStates(outputState, staticOutput);
            OnAutonomyUpdated(new AutonomyArgs(outputState));
            //Reset Hall count for next motor command
            ResetCounters();
        }


        public void tankTurnRight(double steps)
        {

            while (
                Math.Abs(Telemetry.TelemetryHandler.Robo1HallCount2) < steps)
            {
                //System.Diagnostics.Debug.WriteLine(Math.Abs(Telemetry.TelemetryHandler.Robo1HallCount2));
                outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                //Velocity must be Positive for right turn
                outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 500;
                outputState = MergeStates(outputState, staticOutput);
                OnAutonomyUpdated(new AutonomyArgs(outputState));
            }
            //Break
            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
            outputState = MergeStates(outputState, staticOutput);
            OnAutonomyUpdated(new AutonomyArgs(outputState));
            ResetCounters();
        }


        public void tankTurnLeft(double steps)

        {

            while (
                Math.Abs(Telemetry.TelemetryHandler.Robo1HallCount2) < steps)
            {
                System.Diagnostics.Debug.WriteLine(Math.Abs(Telemetry.TelemetryHandler.Robo1HallCount2));
                outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                //Velocity must be negative for left turn
                outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = -500;
                outputState = MergeStates(outputState, staticOutput);
                OnAutonomyUpdated(new AutonomyArgs(outputState));
            }
            //Break
            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
            outputState = MergeStates(outputState, staticOutput);
            OnAutonomyUpdated(new AutonomyArgs(outputState));
            ResetCounters();
        }


        //Call each time to reset the Hall Count for all of the Roboteqs in config
        public void ResetCounters()
        {
            foreach (Controllers.RoboteQ roboteq in lunabotics.RCU.Program.roboteqs)
            {
                roboteq.ClearHallSensorCounts();
            }
        }

        #endregion

        #region Properties
        public bool Active
        {
            get { return active; }
        }
        public bool Started
        {
            get { return started; }
        }
        #endregion
    }

    /// <summary>
    /// Autonomy state feedback
    /// </summary>
    public class AutonomyArgs : EventArgs
    {
        private Dictionary<CommandFields, short> updatedState;

        public AutonomyArgs(Dictionary<CommandFields, short> updatedState)
        {
            if (updatedState == null)
                throw new ArgumentNullException("Updated state is null");

            this.updatedState = updatedState;
        }

        public Dictionary<CommandFields, short> UpdatedState
        {
            get { return updatedState; }
        }
    }
}
