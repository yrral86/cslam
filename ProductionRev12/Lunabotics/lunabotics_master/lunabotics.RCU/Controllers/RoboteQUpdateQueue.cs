using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace lunabotics.RCU.Controllers
{
    public class RoboteQUpdateQueue
    {
        private volatile int dequeueTimeout;
        private readonly object queueSync = new object();

        private Dictionary<Configuration.Devices, int> storage;

        ManualResetEventSlim resetEvent;

        public RoboteQUpdateQueue(int opt_timeout = 1000)
        {
            //-1 for timeout is wait infinite
            if (opt_timeout <= 0 && opt_timeout != -1)
                throw new ArgumentOutOfRangeException("opt_timeout");

            dequeueTimeout = opt_timeout;
            resetEvent = new ManualResetEventSlim(false);
            storage = null;
        }

        public void Enqueue(Dictionary<Configuration.Devices, int> newItem)
        {
            if (newItem == null)
                throw new ArgumentNullException("newItem");

            Monitor.Enter(queueSync);
            try
            {
                if (storage != null)
                { //merge, in case the newItem contains different keys than the current
                    //thus, the old values will not be tossed out
                    foreach (Configuration.Devices key in newItem.Keys)
                        storage[key] = newItem[key];
                }
                else
                    storage = newItem;

                resetEvent.Set();
            }
            finally
            {
                Monitor.Exit(queueSync);
            }
        }

        public int DequeueTimeout
        {
            get { return dequeueTimeout; }
            set
            {
                if (value <= 0 && value != -1)
                    throw new ArgumentOutOfRangeException("value");
                else
                    dequeueTimeout = value;
            }
        }

        public Dictionary<Configuration.Devices, int> Dequeue()
        {
            if (resetEvent.Wait(dequeueTimeout))
            {
                Monitor.Enter(queueSync);
                try
                {
                    var toReturn = storage;
                    storage = null;
                    return toReturn;
                }
                finally
                {
                    resetEvent.Reset();
                    Monitor.Exit(queueSync);
                }
            }
            else
                throw new TimeoutException("Dequeue operation timed out");
        }

        public Dictionary<Configuration.Devices, int> Dequeue(CancellationToken token)
        {
            if (resetEvent.Wait(dequeueTimeout, token))
            {
                Monitor.Enter(queueSync);
                try
                {
                    var toReturn = storage;
                    storage = null;
                    return toReturn;
                }
                finally
                {
                    resetEvent.Reset();
                    Monitor.Exit(queueSync);
                }
            }
            else
                throw new TimeoutException("Dequeue operation timed out");
        }
    }
}
