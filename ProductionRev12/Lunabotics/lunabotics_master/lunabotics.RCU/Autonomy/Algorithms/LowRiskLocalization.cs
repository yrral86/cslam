using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using lunabotics.Comms.CommandEncoding;
using lunabotics.Configuration;

namespace lunabotics.RCU.Autonomy.Algorithms
{
    public class LowRiskLocalization
    {
        #region Enums
        public enum AlgorithmState
        {
            LiftingScoop,
            Spinning,
            Done
        }

        private enum TurnDirection
        {
            Left = 500,
            Right = -500
        }        
        #endregion

        #region Fields
        private bool started;
        private long startTime;
        private AutonomyConfiguration configuration;
        private AlgorithmState state;
        private TurnDirection turnDirection;        
        #endregion

        #region Constructors
        public LowRiskLocalization(AutonomyConfiguration configuration)
        {
            this.configuration = configuration;
            this.turnDirection = TurnDirection.Right;
        }        
        #endregion

        #region Methods
        public Dictionary<CommandFields, short> Update(Robot robot, MonteCarloLocalization localization, Stopwatch stopwatch)
        {
            Dictionary<CommandFields, short> outputState = new Dictionary<CommandFields, short>();

            // Check for start pulse
            if (!started)
            {
                // Set flag
                started = true;
                // Record start time
                startTime = stopwatch.ElapsedMilliseconds;
                // Set state
                state = AlgorithmState.LiftingScoop;
            }

            switch (state)
            {
                case AlgorithmState.LiftingScoop:
                    // Check arm swing limit
                    if (robot.TelemetryFeedback.ScoopUpperLimitSwitchDepressed)
                    {
                        Console.WriteLine("Lifting scoop");
                        // Move to next stage
                        state = AlgorithmState.Spinning;
                        // Save time
                        startTime = stopwatch.ElapsedMilliseconds;
                        // Set state
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                    }
                    else
                    {
                        Console.WriteLine("Scoop lifted");
                        // Set state
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 1000;
                    }
                    break;
                case AlgorithmState.Spinning:
                    long elapsedTime = stopwatch.ElapsedMilliseconds - startTime;
                    if (elapsedTime > configuration.LocalizationTurnDuration)
                    {
                        // Change direction
                        if (turnDirection == TurnDirection.Left)
                            turnDirection = TurnDirection.Right;
                        else
                            turnDirection = TurnDirection.Left;

                        // Reset start timer
                        startTime = stopwatch.ElapsedMilliseconds;
                    }
            
                    // Check convergence of MCL
                    if (localization.Confidence > configuration.DesiredLocalizationConfidence)
                    {
                        Console.WriteLine("MCL converged");
                        // Set flag
                        state = AlgorithmState.Done;

                        // Set state
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                    }
                    else
                    {
                        Console.WriteLine("Spinning in place, MCL confidence: " + localization.Confidence);

                        // Set static state
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = (short)turnDirection;
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;

                        // Check reinitialize
                        if (elapsedTime > configuration.ConvergenceAllowance)
                        {
                            // Reinitialize the MCL algorithm
                            localization.Initialize(robot);
                            // Reset timer
                            startTime = stopwatch.ElapsedMilliseconds;
                        }
                    } 
                    break;
                case AlgorithmState.Done:
                    // Set state
                    outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 1000;
                    break;
                default:
                    break;
            }
            // Static state
            outputState[Comms.CommandEncoding.CommandFields.BucketPitch] = 0;
            outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
            outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
            outputState[Comms.CommandEncoding.CommandFields.RangeFinderServo] = 90;                

            return outputState;
        }
        #endregion

        #region Properties
        public AlgorithmState State
        {
            get { return state; }
        }
        #endregion

    }
}
