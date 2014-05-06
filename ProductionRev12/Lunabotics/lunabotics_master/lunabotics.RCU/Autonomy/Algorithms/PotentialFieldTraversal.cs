using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using lunabotics.Configuration;
using lunabotics.Comms.CommandEncoding;
using Utility;

namespace lunabotics.RCU.Autonomy.Algorithms
{
    public class PotentialFieldTraversal
    {
        #region Enums
        public enum AlgorithmState
        {
            ParkScoop,
            MoveToHeadingInitial,
            MoveToPosition,
            MoveToHeadingFinal,
            Done
        }        
        #endregion

        #region Constants
        private readonly double GainAttractive = 1.0;
        private readonly double GainPFC = 1.0;
        private readonly double AngleThreshold = 10.0;
        private readonly double PositionThreshold = 20.0; // cm
        private readonly double TraverseVelocity = 700; // command
        #endregion

        #region Fields
        private AutonomyConfiguration configuration;
        private AlgorithmState state;
        #endregion

        #region Constructors
        public PotentialFieldTraversal(AutonomyConfiguration configuration)
        {
            // Assign fields
            this.configuration = configuration;
            // Set state

        }
        #endregion

        #region Methods
        public Dictionary<CommandFields, short> Update(Robot robot, Stopwatch stopwatch, Position position)
        {
            Dictionary<CommandFields, short> outputState = new Dictionary<CommandFields, short>();

            switch (state)
            {
                case AlgorithmState.ParkScoop:
                    if (!robot.TelemetryFeedback.ScoopUpperLimitSwitchDepressed)
                    {
                        // Sit still and raise scoop
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 1000;
                    }
                    else
                    {
                        // Set state to next
                        state = AlgorithmState.MoveToHeadingInitial;
                        // Stop
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                    }

                    break;
                case AlgorithmState.MoveToHeadingInitial:
                    // Calculate angle error (normalized -180,180)
                    double error = (robot.Angle - position.Angle) % 180.0d;

                    // Check error
                    if (error > AngleThreshold)
                    {
                        if (error < 0)
                        {
                            // Move left
                            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 500;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                        }
                        else
                        {
                            // Move right
                            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = -500;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                        }
                    }
                    else
                    {
                        // Move to next state
                        state = AlgorithmState.MoveToPosition;

                        // Converged, set state
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                    }                

                    break;
                case AlgorithmState.MoveToPosition:
                    // Calculate distance from goal
                    double distance = Math.Sqrt(
                        Math.Pow((robot.X - position.X), 2.0d) +
                        Math.Pow((robot.Y - position.Y), 2.0d));

                    // Check threshold
                    if (distance > PositionThreshold)
                    {
                        // Not converged, so move toward position
                        double F_att_x = (robot.X - position.X) * GainAttractive;
                        double F_att_y = (robot.Y - position.Y) * GainAttractive;

                        // Calculate angle
                        double angle = MathHelper.RadianToDegree(Math.Atan2(F_att_y, F_att_x));

                        // Calculate mixed commands
                        double omega = angle * GainPFC;

                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 500;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = Convert.ToInt16(angle * GainPFC);
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                    }
                    break;
                case AlgorithmState.MoveToHeadingFinal:
                    // Calculate angle error (normalized -180,180)
                    double error = (robot.Angle - position.Angle) % 180.0d;

                    // Check error
                    if (error > AngleThreshold)
                    {
                        if (error < 0)
                        {
                            // Move left
                            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 500;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                        }
                        else
                        {
                            // Move right
                            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = -500;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                        }
                    }
                    else
                    {
                        // Move to next state
                        state = AlgorithmState.Done;

                        // Converged, set state
                        outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                        outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                    }

                    break;
                case AlgorithmState.Done:
                    break;
                default:
                    break;
            }

            // Static output
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
            get { return this.state; }
        }
        #endregion
    }
}
