using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace lunabotics.Comms
{
    public class UDP_Sender
    {
        private IPEndPoint endpoint;
        private Socket sendSocket;

        public UDP_Sender(string ip_address, int port, int opt_timeout = 0)
        {
            endpoint = new IPEndPoint(IPAddress.Parse(ip_address), port);
            sendSocket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            sendSocket.SendTimeout = opt_timeout;
        }

        public UDP_Sender(IPAddress ipAddress, int port, int opt_timeout = 0)
        {

            endpoint = new IPEndPoint(ipAddress, port);
            sendSocket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            sendSocket.SendTimeout = opt_timeout;
        }

        /// <summary>
        /// Gets or sets a value that specifies the amount of time after which a Send call should time out.
        /// </summary>
        public int Timeout
        {
            get { return sendSocket.SendTimeout; }
            set { sendSocket.SendTimeout = value; }
        }

        public void Send(byte[] data)
        {
            if (data.Length > 64000)
                throw new DataTooLargeException("Data is in excess of 64000 bytes");
            sendSocket.SendTo(data, endpoint);
            //Console.WriteLine("End of: UDP_Sender.cs Send()");
        }
    }

    public class DataTooLargeException : Exception
    {
        public DataTooLargeException(string message)
            : base(message)
        {
        }
    }
}
