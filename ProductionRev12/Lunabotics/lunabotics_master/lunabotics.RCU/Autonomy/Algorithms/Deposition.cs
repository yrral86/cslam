using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Configuration;
using lunabotics.Comms.CommandEncoding;
using System.Diagnostics;

namespace lunabotics.RCU.Autonomy.Algorithms
{
    public class Deposition
    {
        #region Enums
        public enum AlgorithmState
        {
            FinePositioning,
            LiftingBin,
            WiggleRotational,
            WiggleTranslational,
            LoweringBin,
            Done
        }
        private enum TurnDirection
        {
            Left = -2048,
            Right = 0
        }     
        #endregion

        #region Fields
        private bool started;
        private int turnCount;
        private long startTime;
        private AlgorithmState state;
        private AutonomyConfiguration configuration;
        private TurnDirection turnDirection;  
        #endregion

        #region Constructors
        public Deposition(AutonomyConfiguration configuration)
        {
            // Assign fields
            this.configuration = configuration;
            this.turnDirection = TurnDirection.Right;
        }
        #endregion

        #region Methods
        public Dictionary<CommandFields, short> Update(Robot robot, Stopwatch stopwatch)
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
                state = AlgorithmState.LiftingBin;
            }

            switch (state)
            {
                case AlgorithmState.FinePositioning:
                    // Read proximity sensors
                    outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
                    // Calcualte error
                    double error = robot.TelemetryFeedback.RearProximityRight - robot.TelemetryFeedback.RearProximityLeft;
                    // Check for position converged
                    if ((robot.TelemetryFeedback.RearProximityLeft - robot.TelemetryFeedback.RearProximityRight) < 10)
                        state = AlgorithmState.LiftingBin;
                    break;
                case AlgorithmState.LiftingBin:
                    // Set state
                    outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 1000;
                    outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 1000;

                    // Check for bin fully raised
                    if (robot.TelemetryFeedback.BinUpperSwitchDepressed)
                    {
                        state = AlgorithmState.WiggleRotational;
                        // Store start time
                        startTime = stopwatch.ElapsedMilliseconds;
                    }
                    break;
                case AlgorithmState.WiggleRotational:
                    // Calculate time
                    long elapsedTime = stopwatch.ElapsedMilliseconds - startTime;

                    if (elapsedTime > 1500)
                    {
                        // Change direction
                        if (turnDirection == TurnDirection.Left)
                            turnDirection = TurnDirection.Right;
                        else
                            turnDirection = TurnDirection.Left;

                        // Increment counter
                        turnCount++;

                        // Reset start timer
                        startTime = stopwatch.ElapsedMilliseconds;
                    }

                    // Check convergence of MCL
                    if (turnCount > 3)
                    {
                        Console.WriteLine("Done wiggling");
                        // Set flag
                        state = AlgorithmState.LoweringBin;

                        // Set state
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
                    }
                    else
                    {
                        // Set static state
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = (short)turnDirection;
                        outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
                    } 

                    break;
                case AlgorithmState.LoweringBin:
                    Console.WriteLine("Lowering bin");
                    outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = -1000;
                    outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = -1000;

                    // Check for bin fully raised
                    if (robot.TelemetryFeedback.BinLowerSwitchDepressed)
                        state = AlgorithmState.Done;
                    break;
                case AlgorithmState.Done:
                    Console.WriteLine("Done");
                    // Set state
                    outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
                    outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
                    break;
                default:
                    break;
            }

            // Static outputs
            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
            outputState[Comms.CommandEncoding.CommandFields.BucketPitch] = 0;
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
