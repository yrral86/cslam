using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Comms;
using Microsoft.Xna.Framework.Input;
using lunabotics.OCU.Properties;
using lunabotics.Configuration;

namespace lunabotics.OCU.Models
{
    public static class ControllerInput
    {
        
        static Mode mode = Mode.Manual;

        public static Dictionary<lunabotics.Comms.CommandEncoding.CommandFields, short> GetLunabotState()
        {
            var state = GamePad.GetState(ControllerSettings.Default.Player, GamePadDeadZone.Circular);

            if(!state.IsConnected)
                throw new Exception("Controller " + ControllerSettings.Default.Player + " is Disconnected!");

            //process state and build output
            Dictionary<lunabotics.Comms.CommandEncoding.CommandFields, short> outputState = new Dictionary<Comms.CommandEncoding.CommandFields, short>();

            //Translational and rotaional velocities - left thumbstick
            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = Convert.ToInt16(state.ThumbSticks.Left.Y * 1000.0);
            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = Convert.ToInt16(state.ThumbSticks.Left.X * 1000.0);

            //Scoop Arms - raise arms with up on right stick, lower with down
            outputState[Comms.CommandEncoding.CommandFields.ScoopPivot] = GetArmPivot(state); 

            //Scoop Actuators left trigger to retract actuators, right trigger to extend
            outputState[Comms.CommandEncoding.CommandFields.ScoopPitch] = GetPitch(state);
            
            //Bucket - left bumper to lower, right bumper to raise
            outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = GetActuator(state);
            outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = GetActuator(state);

            // Set autonomy state
            outputState[Comms.CommandEncoding.CommandFields.Mode] = GetMode(state);

            return outputState;
        }

        private static short GetMode(GamePadState state)
        {
            mode = Mode.Null;

            if (state.Buttons.Start == ButtonState.Pressed)
                mode = Mode.Autonomous;
            if (state.Buttons.Back == ButtonState.Pressed)
                mode = Mode.Manual;
            /*
            if (state.Buttons.A == ButtonState.Pressed)
            {
                //if(Scoop.Angle >=120)
                mode = Mode.BinLowerMacro;
            }
            if (state.Buttons.Y == ButtonState.Pressed)
            {
                //if(Scoop.Angle >=120)
                mode = Mode.BinRaiseMacro;
            }
            */
            if (state.Buttons.X == ButtonState.Pressed)
            {
                //if(Bucket.Angle = 0)
                mode = Mode.CollectScoopMacro;
            }
            return (short)mode;
        }

        internal static double GetRelativeSpeed(double input)
        {
            return (Math.Pow(Math.E, ControllerSettings.Default.SpeedSensitivity * input) - 1) /
                (Math.Pow(Math.E, ControllerSettings.Default.SpeedSensitivity * input) + 1);
        }

        private static short GetPitch(GamePadState state)
        {
            if (state.Triggers.Left > 0 && state.Triggers.Right > 0) //both triggers depressed
                return 0;
            else if (state.Triggers.Left > 0) //left trigger only
            {
                return (short)Math.Round(state.Triggers.Left * -1000.0);
            }
            else if (state.Triggers.Right > 0)//right trigger only
            {
                return (short)Math.Round(state.Triggers.Right * 1000.0);
            }
            else if (state.Buttons.A == ButtonState.Pressed)
            {
                return 1000;
            }
            else if (state.Buttons.Y == ButtonState.Pressed)
            {
                return -1000;
            }
            else
                return 0;
        }

        private static short GetArmPivot(GamePadState state)
        {
            if (Math.Abs(state.ThumbSticks.Right.Y) > 0)
            {
                return Convert.ToInt16(state.ThumbSticks.Right.Y * 1000.0);
            }
            else if (state.Buttons.A == ButtonState.Pressed)
            {
                return -1000;
            }
            else if (state.Buttons.Y == ButtonState.Pressed)
            {
                return 1000;
            }
            else
                return 0;
        }

        private static short GetActuator(GamePadState state)
        {
            //if (state.DPad.Up == ButtonState.Pressed)
            //    return 1000;
            //else if (state.DPad.Down == ButtonState.Pressed)
            //    return -1000;
            //else
            //    return 0;
            if (state.Buttons.RightShoulder == ButtonState.Pressed &&
                state.Buttons.LeftShoulder == ButtonState.Pressed)
                return 0;
            else if (state.Buttons.LeftShoulder == ButtonState.Pressed)
                return -1000;
            else if (state.Buttons.RightShoulder == ButtonState.Pressed)
                return 1000;
            else
                return 0;

        }
    }
}
