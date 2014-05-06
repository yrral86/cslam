using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Configuration;
using lunabotics.Comms.CommandEncoding;

namespace lunabotics.RCU.Autonomy.Algorithms
{
    public class MiningAlgorithm
    {
        private int scoops;
        private int regolith;  //???
        private Robot robot;
        private Dictionary<AutonomyConfiguration,short> configuration;
        public enum AlgorithmState
        {
            //FindGroundLevel
            LowerScoop,
            DigIn,
            DigMore,
            DepositScoop,
            Reposition,
            Done
        }
        private AlgorithmState state;
        public MiningAlgorithm(Robot robot)
        {
        }

        public Dictionary<CommandFields, short> MiningUpdate()
        {
                        Dictionary<CommandFields, short> outputState = new Dictionary<CommandFields, short>();
            if (robot.ExpectedZone == lunabotics.RCU.Autonomy.Robot.Zone.Mining)
            {
                switch (state)
                {
                    case AlgorithmState.LowerScoop:
                        if (robot.TelemetryFeedback.ArmSwingAngle > -0.6279659846) //Get pitch telemetry
                        {
                            outputState[CommandFields.BucketPivot] = -1000;
                            outputState[CommandFields.BucketPitch] = 1000;
                        }
                        else
                            state = AlgorithmState.DigIn;
                        break;
                    case AlgorithmState.DigIn:
                        if (!robot.TelemetryFeedback.ScoopLowerLimitSwitchDepressed)
                        {
                            outputState[CommandFields.BucketPivot] = -1000;
                            outputState[CommandFields.BucketPitch] = 1000;
                            outputState[CommandFields.TranslationalVelocity] = 500;
                        }
                        else
                            state = AlgorithmState.DigMore;
                        break;
                    case AlgorithmState.DigMore:
                        if (robot.TelemetryFeedback.ArmSwingAngle < -0.6279659846)
                        {
                            outputState[CommandFields.BucketPivot] = 1000;
                            outputState[CommandFields.TranslationalVelocity] = 500;
                        }
                        break;
                    case AlgorithmState.DepositScoop:
                        if (!robot.TelemetryFeedback.ScoopUpperLimitSwitchDepressed)
                            outputState[CommandFields.BucketPivot] = 1000;
                        else
                        {
                            outputState[CommandFields.TranslationalVelocity] = 500;
                            if (scoops<3)
                                state=AlgorithmState.Reposition;
                            else
                            state=AlgorithmState.Done;
                        }
                        break;
                    case AlgorithmState.Reposition:
                            outputState[CommandFields.RotationalVelocity]=1000;
                            break;
                    case AlgorithmState.Done:
                        outputState[CommandFields.RotationalVelocity]=0;
                        outputState[CommandFields.TranslationalVelocity] = 500;
                        outputState[CommandFields.BucketPitch] = 0;
                        outputState[CommandFields.BucketPivot] = 0;
                        break;
                    default:
                        break;
                }
            }
            return outputState;
        }
    }
}
