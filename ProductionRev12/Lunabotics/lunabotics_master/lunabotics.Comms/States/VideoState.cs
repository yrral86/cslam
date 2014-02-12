using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Comms.States
{
    public class VideoState
    {
        public int FrameRate;
        //quality is scaled down to 1-20 on OCU, must be scaled back to cover 1-100 for RCU
        public int Quality;

        public static short EncodeVideoState(int quality, int frame_rate)
        {
            if (quality > 20 || quality < 1)
                throw new ArgumentOutOfRangeException("quality", "Must be between 1-20");

            if (frame_rate < 0 || frame_rate > 30)
                throw new ArgumentOutOfRangeException("frame_rate", "Must be between 0-30");

            //allowable range of values are 2047 to -2048
            return (short)(quality*100 + frame_rate);
        }

        public static VideoState DecodeVideoState(short encoded_state)
        {
            VideoState toReturn = new VideoState();

            toReturn.Quality = encoded_state / 100;
            toReturn.FrameRate = encoded_state%100;

            return toReturn;
        }
    }
}
