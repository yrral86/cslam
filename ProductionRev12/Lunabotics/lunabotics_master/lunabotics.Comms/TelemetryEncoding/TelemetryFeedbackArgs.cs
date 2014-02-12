using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace lunabotics.Comms.TelemetryEncoding
{
    public class TelemetryFeedbackArgs : EventArgs
    {
        private TelemetryFeedback state;

        public TelemetryFeedbackArgs(TelemetryFeedback updatedState)
        {
            if (updatedState == null)
                throw new ArgumentNullException("Updated state is null");

            this.state = updatedState;
        }

        public TelemetryFeedback UpdatedState
        {
            get { return state; }
        }
    }
}
