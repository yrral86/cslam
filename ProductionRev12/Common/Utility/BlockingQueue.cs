using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Utility
{
    public class BlockingQueue<T>
    {
        private volatile int dequeueTimeout;

        private Queue<T> queue;
        private readonly object queueSync = new object();

        private ManualResetEventSlim resetEvent;

        public BlockingQueue(int opt_timeout = 1000)
        {
            //-1 for timeout is wait infinite
            if (opt_timeout <= 0 && opt_timeout != -1)
                throw new ArgumentOutOfRangeException("opt_timeout");

            dequeueTimeout = opt_timeout;
            resetEvent = new ManualResetEventSlim(false);
            queue = new Queue<T>();
        }

        public void Enqueue(T newItem)
        {
            if (newItem == null)
                throw new ArgumentNullException("newItem");

            Monitor.Enter(queueSync);
            try
            {
                queue.Enqueue(newItem);
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
                    return queue.Dequeue();
                }
                finally
                {
                    if (queue.Count <= 0)
                        resetEvent.Reset();
                    Monitor.Exit(queueSync);
                }
            }
            else
                throw new TimeoutException("Dequeue operation timed out");
        }

        public T Dequeue(CancellationToken token)
        {
            if (resetEvent.Wait(dequeueTimeout, token))
            {
                Monitor.Enter(queueSync);
                try
                {
                    return queue.Dequeue();
                }
                finally
                {
                    if (queue.Count <= 0)
                        resetEvent.Reset();
                    Monitor.Exit(queueSync);
                }
            }
            else
                throw new TimeoutException("Dequeue operation timed out");
        }

        public void Clear()
        {
            Monitor.Enter(queueSync);
            try
            {
                queue.Clear();
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
    }
}
