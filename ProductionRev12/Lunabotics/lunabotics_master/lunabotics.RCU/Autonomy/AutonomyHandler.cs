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
using lunabotics.RCU.Autonomy.Algorithms;
using lunabotics.RCU.Models;

namespace lunabotics.RCU.Autonomy
{
    public class AutonomyHandler : IProvideTelemetry
    {
        #region Enums
        public enum State
        {
            Manual,
            LowRiskLocalization,
            TraverseToMining,
            TraverseToDeposition,
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
        #endregion

        #region Fields
        private bool active; // Thread started
        private bool started; // State set to run
        private readonly object timerSync = new object();
        private AutonomyConfiguration configuration;
        private Deposition depositionAlgorithm;
        private Dictionary<CommandFields, short> staticOutput;
        private LowRiskLocalization lowRiskLocalizationAlgorithm;
        private MonteCarloLocalization localization;
        private Robot robot;
        private State state;
        private Stopwatch stopwatch;
        private Timer timer;
        private Zone expectedZone;
        #endregion

        #region Constructors
        public AutonomyHandler(AutonomyConfiguration configuration, ref TelemetryHandler telemetryHandler)
        {
            // Assign fields
            this.configuration = configuration;

            // Create localization logic
            localization = new MonteCarloLocalization(configuration);

            // Create robot
            robot = new Robot(configuration);

            // Create stopwatch
            stopwatch = new Stopwatch();

            // Add telemetry callback
            telemetryHandler.TelemetryFeedbackProcessed += telemetryHandler_TelemetryFeedbackProcessed;

            // Create timer
            timer = new Timer(configuration.Interval);
            timer.Elapsed += timer_Elapsed;

            // Create algorithms
            depositionAlgorithm = new Deposition(configuration);
            lowRiskLocalizationAlgorithm = new LowRiskLocalization(configuration);
            
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

            // Initialize MCL
            localization.Initialize(robot);
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
            state = State.Deposition;
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
                    // Create output state
                    Dictionary<CommandFields, short> outputState = new Dictionary<CommandFields, short>();

                    // Process state
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
                            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 1000;
                            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPitch] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RangeFinderServo] = 90;
                            break;
                        case State.LowRiskLocalization:
                            // Perform Monte Carlo localization
                            localization.Update(configuration.Interval / 1000.0d, robot, expectedZone, true);
                            robot.SetState(localization.Tracker);
                            // Calculate new output state
                            outputState = lowRiskLocalizationAlgorithm.Update(robot, localization, stopwatch);
                            
                            // If done, move on to the next step
                            if (lowRiskLocalizationAlgorithm.State == LowRiskLocalization.AlgorithmState.Done)
                            {
                                // Move to the next state
                                if (robot.Regolith < configuration.MinRegolithPerRun)
                                {
                                    state = State.TraverseToMining;
                                }
                                else
                                {
                                    state = State.TraverseToDeposition;
                                }
                            }

                            break;
                        case State.TraverseToMining:
                            // Perform Monte Carlo localization
                            localization.Update(configuration.Interval / 1000.0d, robot, expectedZone, true);
                            robot.SetState(localization.Tracker);
                            // Calculate new output state


                            break;
                        case State.TraverseToDeposition:
                            break;
                        case State.Mining:
                            break;
                        case State.Deposition:
                            // Continue localization
                            //localization.Update(configuration.Interval / 1000.0d, robot, expectedZone, true);
                            //robot.SetState(localization.Tracker);
                            // Perform deposition algorithm
                            outputState = depositionAlgorithm.Update(robot, stopwatch);
                            // Check converged
                            if (depositionAlgorithm.State == Deposition.AlgorithmState.Done)
                            {

                            }
                            break;
                        default:
                            break;
                    }

                    // Update output state
                    outputState = MergeStates(outputState, staticOutput);
                    OnAutonomyUpdated(new AutonomyArgs(outputState));

                    // Store robot velocities
                    robot.SetVelocities(
                        Kinematics.GetTranslationalVelocity(outputState[CommandFields.TranslationalVelocity]),
                        Kinematics.GetRotationalVelocity(outputState[CommandFields.RotationalVelocity]));

                    // Update telemetry
                    UpdateTelemetry(localization);
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

        private void UpdateTelemetry(MonteCarloLocalization localization)
        {
            // Update telemetry
            Dictionary<Configuration.Telemetry, int> dictionary = new Dictionary<Configuration.Telemetry, int>();
            //dictionary[Configuration.Telemetry.LocalizationX] = (int)(localization.Tracker.X * 10.0d); // Convert for double to int -> Like RoboteQ
            //dictionary[Configuration.Telemetry.LocalizationY] = (int)(localization.Tracker.Y * 10.0d); // Convert for double to int -> Like RoboteQ
            //dictionary[Configuration.Telemetry.LocalizationPsi] = (int)(localization.Tracker.Angle * 10.0d); // Convert for double to int -> Like RoboteQ
            //dictionary[Configuration.Telemetry.LocalizationConfidence] = (int)(100.0 * localization.Confidence); // 0 = No confidence, 100 = Complete convergence/good confidence
            //dictionary[Configuration.Telemetry.LocalizationState] = (int)state;
            dictionary[Configuration.Telemetry.LocalizationX] = 10;
            dictionary[Configuration.Telemetry.LocalizationY] = 10;
            dictionary[Configuration.Telemetry.LocalizationPsi] = 90;
            dictionary[Configuration.Telemetry.LocalizationConfidence] = 100;
            dictionary[Configuration.Telemetry.LocalizationState] = (int)state;

            OnTelemetryUpdated(new TelemetryEventArgs((int)configuration.Interval, dictionary));
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
