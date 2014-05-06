using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace lunabotics.Comms
{
    // todo : should the events be fired using a threadpool to keep them from blocking?
    // this would require a user's delegate to be re-entrant.

    /// <summary>
    /// A simple class that provides asynchronous UDP read operations.
    /// </summary>
    public class UDP_Receiver : IDisposable
    {
        private IPEndPoint endpoint;
        private UdpClient receiver;
        int port;
        private Thread receive_thread;
        private volatile bool isActive;

        private bool disposed = false;

        private int opt_timeout;

        /// <summary>
        /// Creates a UDP Receiver that will listen on the specified port.
        /// </summary>
        /// <param name="port">Listening port</param>
        /// <param name="opt_timeout">Optional receive timeout value.  Default is 1000 ms.</param>
        /// <remarks>
        /// The receiver is set to not allow for datagram fragmenting.
        /// </remarks>
        public UDP_Receiver(int port, int opt_timeout = 1000)
        {
            this.port = port;
            endpoint = new IPEndPoint(IPAddress.Any, port);
            isActive = false;
            this.opt_timeout = opt_timeout;
        }

        ~UDP_Receiver()
        {
            Dispose(false);
        }

        /// <summary>
        /// Gets or sets a value that specifies the amount of time after which a Receive call should time out.
        /// </summary>
        /// <remarks>
        /// Will not be validated or go into effect until receiver is Activated
        /// </remarks>
        public int Timeout
        {
            get { return opt_timeout; }
            set { opt_timeout = value; }
        }

        /// <summary>
        /// Gets a value indicating whether the Receiver is active or not.
        /// </summary>
        public bool IsActive
        {
            get { return isActive; }
        }

        /// <summary>
        /// Activates asynchronous operation of the UDP Receiver.
        /// </summary>
        /// <remarks>
        /// The asynchronous operation is conducted in separate thread.
        /// To receive updates whenever a new datagram is received, subscribe to the DataReceived event.
        /// Subscribe to the ReceiverError method to be alerted of any SocketException occurrences.
        /// </remarks>
        public void Activate()
        {
            if (isActive)
                throw new Exception("Already active");

            receiver = new UdpClient(endpoint);
            receiver.Client.ReceiveTimeout = opt_timeout;

            try
            {
                receive_thread = new Thread(new ParameterizedThreadStart(p => DoReceive()));
                receive_thread.SetApartmentState(ApartmentState.STA);
                isActive = true;
                receive_thread.Start();
            }
            catch (Exception ex)
            {
                Deactivate();
                throw;
            }
        }

        /// <summary>
        /// Shutsdown asynchronous operation.
        /// </summary>
        public void Deactivate()
        {
            isActive = false;
            if (receiver != null)
                receiver.Close();

            if (receive_thread != null && receive_thread.ThreadState != ThreadState.Unstarted)
            {
                receive_thread.Join();
                receive_thread = null;
            }
        }

        /// <summary>
        /// Raised whenever a new datagram is received.
        /// </summary>
        /// <remarks>
        /// Delegates attached to this event are done synchronously with the receive polling, so ensure
        /// that they complete quickly.
        /// </remarks>
        public event EventHandler<DataArgs> DataReceived;
        /// <summary>
        /// Raised whenever a SocketException occurs.
        /// </summary>
        public event EventHandler<ErrorArgs> ReceiverError;

        private void DoReceive()
        {
            while (isActive)
            {
                byte[] buffer;
                try
                {
                    buffer = receiver.Receive(ref endpoint);
                    OnDataReceived(new DataArgs(buffer));
                }
                catch (SocketException ex)
                {
                    OnError(new ErrorArgs(ex));
                }
                catch (ObjectDisposedException ex)
                {
                    Console.WriteLine("Socket closed from another thread: " + ex.Message);
                    break;
                }
                catch (Exception ex) 
                {
                    //fail silently
                }
            }
        }
        private void OnDataReceived(DataArgs args)
        {
            if (DataReceived != null)
                DataReceived(this, args);
        }
        private void OnError(ErrorArgs args)
        {
            if (ReceiverError != null)
                ReceiverError(this, args);
        }


        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (disposing)
                {
                    this.Deactivate();
                }

                disposed = true;
            }
        }
    }

    public class ErrorArgs : EventArgs
    {
        private SocketException ex;

        public ErrorArgs(SocketException ex)
        {
            this.ex = ex;
        }

        public SocketException Exception
        {
            get { return this.ex; }
        }
    }
}
