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
            var state = GamePad.GetState(ControllerSettings.Default.Player);

            if(!state.IsConnected)
                throw new Exception("Controller " + ControllerSettings.Default.Player + " is Disconnected!");

            //process state and build output
            Dictionary<lunabotics.Comms.CommandEncoding.CommandFields, short> outputState = new Dictionary<Comms.CommandEncoding.CommandFields, short>();

            //speed & radius - thumbsticks
            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = Convert.ToInt16(state.ThumbSticks.Left.Y * 1000.0);
            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = Convert.ToInt16(state.ThumbSticks.Right.X * 1000.0);
            //outputState[Comms.CommandEncoding.CommandFields.Radius] = 0;
            
            //pivot - right trigger for lower, left trigger to raise
            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = GetPivot(state);

            //pitch - right bumber to lower, left bumber to raise
            outputState[Comms.CommandEncoding.CommandFields.BucketPitch] = GetPitch(state);
            
            //belt - Y to excavate
            outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = GetActuator(state);
            outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = GetActuator(state);

            // Set servo state
            //outputState[Comms.CommandEncoding.CommandFields.RangeFinderServo] = (short)((45.0 * state.ThumbSticks.Right.X) + 45.0);
            outputState[Comms.CommandEncoding.CommandFields.RangeFinderServo] = 0;

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
            if (state.Buttons.A == ButtonState.Pressed)
                mode = Mode.BinLowerMacro;
            if (state.Buttons.Y == ButtonState.Pressed)
                mode = Mode.BinRaiseMacro;
            if (state.Buttons.X == ButtonState.Pressed)
                mode = Mode.CollectScoopMacro;

            return (short)mode;
        }

        internal static double GetRelativeSpeed(double input)
        {
            return (Math.Pow(Math.E, ControllerSettings.Default.SpeedSensitivity * input) - 1) /
                (Math.Pow(Math.E, ControllerSettings.Default.SpeedSensitivity * input) + 1);
        }

        //Radius uses left-stick x-value
        internal static short GetRadius(double input)
        {
            //input is between -1 to 1

            //scaled to -1000 to 1000...
            int scaled_input = (int)Math.Round(input * 1000.0);

            if (scaled_input == 0) //no steering
                return 2047;
            else if (scaled_input == -1000) //counter-clockwise spin
            {
                return -2048;
            }
            else if (scaled_input == 1000) //clockwise spin
            {
                return 0;
            }
            else
            { //general case

                double input_magnitude = Math.Abs(input);

                double rad = (1 - input_magnitude) / (ControllerSettings.Default.SteeringSensitivity / 100.0 * input_magnitude);


                return Convert.ToInt16((Math.Min(2047, rad) * Math.Sign(input)));
            }
        }

        private static short GetPivot(GamePadState state)
        {
            if (state.Triggers.Left > 0 && state.Triggers.Right > 0) //both triggers depressed
                return 0;
            else if (state.Triggers.Left > 0) //left trigger only
            {
                return (short)Math.Round(state.Triggers.Left * 1000.0);
            }
            else //right trigger only
            {
                return (short)Math.Round(state.Triggers.Right * -1000.0);
            }
        }

        private static short GetPitch(GamePadState state)
        {
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

        private static short GetActuator(GamePadState state)
        {
            if (state.DPad.Up == ButtonState.Pressed)
                return 1000;
            else if (state.DPad.Down == ButtonState.Pressed)
                return -1000;
            else
                return 0;
        }
    }
}
