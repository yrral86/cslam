using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace lunabotics.RCU.Utility
{
    /// <summary>
    /// This is thread-safe blocking queue with a maximum size of 1.
    /// </summary>
    /// <remarks>
    /// This queue provides simple threadsafe enqueue/dequeue operations.
    /// The queue has a maximum size of 1; an enqueue while the queue is full will cause the old
    /// item to be dropped. This way, the queue always holds the most up to date item.
    /// This should only be used when all that matters is the freshness of the data, and not completeness.
    /// </remarks>
    /// <typeparam name="T"></typeparam>
    public class UpdateQueue<T>
    {
        private volatile int dequeueTimeout;
        private readonly object queueSync = new object();

        private T storage;

        ManualResetEventSlim resetEvent;

        public UpdateQueue(int opt_timeout = 1000)
        {
            //-1 for timeout is wait infinite
            if (opt_timeout <= 0 && opt_timeout != -1)
                throw new ArgumentOutOfRangeException("opt_timeout");

            dequeueTimeout = opt_timeout;
            resetEvent = new ManualResetEventSlim(false);
        }

        public void Enqueue(T newItem)
        {
            if (newItem == null)
                throw new ArgumentNullException("newItem");

            Monitor.Enter(queueSync);
            try
            {
                storage = newItem;
                resetEvent.Set();
            }
            finally
            {
                Monitor.Exit(queueSync);
            }
        }

        public T Dequeue()
        {
            if (resetEvent.Wait(dequeueTimeout))
            {
                Monitor.Enter(queueSync);
                try
                {
                    T temp = storage;
                    storage = default(T);
                    return temp;
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

        public int DequeueTimeout
        {
            get { return dequeueTimeout; }
            set
            {
                if (value <= 0 && value != -1)
                    throw new ArgumentOutOfRangeException("value");
                dequeueTimeout = value; 
            }
        }
    }
}
