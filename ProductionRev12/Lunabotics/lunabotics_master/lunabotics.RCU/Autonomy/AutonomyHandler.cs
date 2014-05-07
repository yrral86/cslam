using lunabotics.Comms.CommandEncoding;
using lunabotics.Comms.TelemetryEncoding;
using lunabotics.Configuration;
using lunabotics.RCU.Telemetry;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Timers;
using System.Threading;
using lunabotics.RCU.Autonomy.Algorithms;
using lunabotics.RCU.Models;
using lunabotics.RCU.Hokuyo;
using lunabotics.RCU.Localization;
using System.Net.Sockets;


namespace lunabotics.RCU.Autonomy
{
    public class AutonomyHandler : IProvideTelemetry
    {
        #region Enums
        public enum State
        {
            Manual,
            InitializeAutonomy,
            TraverseWithObstacleAvoidance,
            TraverseClearPath,
            ReturnToDeposition,
            Mining,
            Deposition,
            TemporaryTesting,
            SafeShutdown
        }
        public enum Zone
        {
            Start,
            Obstacle,
            Mining,
            Unknown
        }

        public enum direction
        {
            forward,
            right,
            left,
            reverse
        }
        #endregion

        #region Events
        public event EventHandler<AutonomyArgs> AutonomyUpdated;
        public event EventHandler<Telemetry.TelemetryEventArgs> TelemetryUpdated;
        //Array to hold sensor values
        private int[] EthernetSensorData;
        #endregion

        #region Fields
        //Initialize motor powers (Note forward power is negative)
        public short forwardPower = 800;
        public short reversePower = -800;
        public short rightPower = -400;
        public short leftPower = 400;

        private bool active; // Thread started
        private bool started; // State set to run
        private readonly object timerSync = new object();
        private AutonomyConfiguration configuration;
        //private Deposition depositionAlgorithm;
        private Dictionary<CommandFields, short> staticOutput;
        private Robot robot;
        private State state;

        private get_distance_ethernet utm = new get_distance_ethernet();
        //string ip_address = "192.168.1.8";
        //int port_number = 10940;
        //NetworkStream stream;
        //TcpClient urg = new TcpClient();

        private Stopwatch stopwatch;
        private System.Timers.Timer timer;
        private Zone expectedZone;
        // Create output state
        private Dictionary<CommandFields, short> outputState = new Dictionary<CommandFields, short>();
        //Create current pose
        private Dictionary<Pose, double> currentPose = new Dictionary<Pose, double>();
        //Create pose to hold changes in pose after moving
        private Dictionary<Pose, double> changePose = new Dictionary<Pose, double>();

        #endregion

        #region Constructors
        public AutonomyHandler(AutonomyConfiguration configuration, ref TelemetryHandler telemetryHandler)
        {
            // Assign fields
            this.configuration = configuration;

            // Create robot
            robot = new Robot(configuration);



            // Create stopwatch
            stopwatch = new Stopwatch();

            // Add telemetry callback
            telemetryHandler.TelemetryFeedbackProcessed += telemetryHandler_TelemetryFeedbackProcessed;

            // Create timer
            timer = new System.Timers.Timer(configuration.Interval);
            timer.Elapsed += timer_Elapsed;

            // Create static state
            staticOutput = new Dictionary<CommandFields, short>();
            staticOutput[Comms.CommandEncoding.CommandFields.Mode] = 1;
            staticOutput[Comms.CommandEncoding.CommandFields.FrontCameraState] = 0;
            staticOutput[Comms.CommandEncoding.CommandFields.BucketCameraState] = 0;
            staticOutput[Comms.CommandEncoding.CommandFields.RearCameraState] = 0;
        }
        #endregion

        #region Methods
        public void Activate()
        {
            // Start timer
            timer.Enabled = true;

            // Set flag
            active = true;

        }

        public void Deactivate()
        {
            if (active)
            {
                // Disable timer
                timer.Enabled = false;

                //Set flag
                active = false;
            }
        }

        private Dictionary<CommandFields, short> MergeStates(Dictionary<CommandFields, short> left, Dictionary<CommandFields, short> right)
        {
            foreach (CommandFields commandField in right.Keys)
            {
                left[commandField] = right[commandField];
            }

            return left;
        }

        private void OnAutonomyUpdated(AutonomyArgs args)
        {
            if (AutonomyUpdated != null)
                AutonomyUpdated(this, args);
        }

        private void OnTelemetryUpdated(Telemetry.TelemetryEventArgs args)
        {
            if (TelemetryUpdated != null)
                TelemetryUpdated(this, args);
        }

        public void Start()
        {
            ////hokuyo initialization
            //urg.Connect(ip_address, port_number);
            //stream = urg.GetStream();
            Console.WriteLine("Autonomy.Start()");
            // Reset stopwatch
            stopwatch.Restart();
            // Set state
            state = State.InitializeAutonomy;
            // Set flag
            started = true;
            // Set zone
            expectedZone = Zone.Start;
            //Reset Hall Counts From previous runs
            ResetCounters();

        }

        public void Stop()
        {
            ////close hokuyo
            //utm.quit(stream, urg); 
            // Set flags
            started = false;
            state = State.Manual;
            // Stop stopwatch
            stopwatch.Stop();

            Console.WriteLine("Stop");
        }

        void telemetryHandler_TelemetryFeedbackProcessed(object sender, Comms.TelemetryEncoding.TelemetryFeedbackArgs e)
        {
            // Obtain measurements and copy to robot object
            robot.TelemetryFeedback = (TelemetryFeedback)e.UpdatedState.Clone();
        }

        private void timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            // Reentry protection
            if (System.Threading.Monitor.TryEnter(timerSync))
            {

                try
                {
                    // Process state
                    Thread.CurrentThread.Name = "AutonomyHandlerThread";
                    //Console.WriteLine(stopwatch.Elapsed);
                    switch (state)
                    {
                        case State.Manual:
                            // Should not be here, but just in case
                            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPivot] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.BucketPitch] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RangeFinderServo] = 90;
                            break;

                        case State.InitializeAutonomy:

                            //Initialize position - temporary 
                            StartingAutonomy();
                            //Initialize Particle Filter
                            Swarm.swarm_init(721, 180, 7300, 4100, 1940);
                            //Move(300, direction.forward);
                            //Move(300, direction.right);
                            //Move(300, direction.left);
                            //Move(300, direction.reverse);
                            //state = State.Manual;
                            //break;

                            //Loop to try and converge particles
                            for (int j = 0; j < 5; j++)
                            {
                                EthernetSensorData = utm.EthernetScan();
                                Swarm.swarm_move(0, 0, 0);
                                Swarm.swarm_update(EthernetSensorData);
                            }
                            currentPose[Pose.Xpos] = Swarm.swarm_get_best_x();
                            currentPose[Pose.Ypos] = Swarm.swarm_get_best_y();
                            currentPose[Pose.Heading] = Swarm.swarm_get_best_theta();

                            //turnToGivenHeading(-90);
                            //tankTurnLeft(450);
                            state = State.TemporaryTesting;
                            break;

                        case State.TemporaryTesting:

                            while (currentPose[Pose.Xpos] < 5100)
                            {
                                //MoveForward(300);
                                Move(300, direction.forward);
                                Thread.Sleep(100);

                            }

                            state = State.Mining;
                            break;
                        case State.Mining:

                            turnToGivenHeading(90);
                            Thread.Sleep(100);

                            while (currentPose[Pose.Ypos] < 2800)
                            {
                                //MoveForward(300);
                                Move(300, direction.forward);
                                Thread.Sleep(100);
                            }

                            turnToGivenHeading(180);
                            Thread.Sleep(100);

                            while (currentPose[Pose.Xpos] > 1900)
                            {
                                //MoveForward(300);
                                Move(300, direction.forward);
                                Thread.Sleep(100);
                            }

                            turnToGivenHeading(-90);
                            Thread.Sleep(100);

                            while (currentPose[Pose.Ypos] > 1500)
                            {
                                Move(300, direction.forward);
                                Thread.Sleep(100);
                            }
                            
                            turnToGivenHeading(0);
                            Thread.Sleep(100);

                            state = State.TemporaryTesting;
                            break;

                        case State.SafeShutdown:

                            Stop();
                            break;

                        default:
                            break;
                    }

                }
                catch (TimeoutException ex)
                {
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
                finally
                {
                    System.Threading.Monitor.Exit(timerSync);
                }
            }
        }

        //Moving from Starting position
        public void StartingAutonomy()
        {
            currentPose[Pose.Heading] = 0; //Initital heading in degrees
            currentPose[Pose.Xpos] = 970; //Initial X in mm
            currentPose[Pose.Ypos] = 970; //Initial Y in mm
        }

        public void Move(int steps, direction dir)
        {
            ResetCounters();
            short transPower = 0, rotPower = 0;
            //Set Powers
            if (dir == direction.forward)
                transPower = forwardPower;
            else if (dir == direction.reverse)
                transPower = reversePower;
            else if (dir == direction.right)
                rotPower = rightPower;
            else if (dir == direction.left)
                rotPower = leftPower;

            while (
                 Telemetry.TelemetryHandler.Robo1HallCount1 < steps || Telemetry.TelemetryHandler.Robo2HallCount1 < steps)
            {

                //Set Velocity of motor. -Power between -1000 and 0
                outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = transPower;
                outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = rotPower;
                //Set Output States
                outputState = MergeStates(outputState, staticOutput);
                //Make calls to event handler to move motors
                OnAutonomyUpdated(new AutonomyArgs(outputState));
            }
            //Stop motors
            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
            outputState = MergeStates(outputState, staticOutput);
            OnAutonomyUpdated(new AutonomyArgs(outputState));
            //Reset Hall count for next motor command
            ResetCounters();
            ParticleFiltering(steps, steps);
        }

        public void ParticleFiltering(int stepsR, int stepsL)
        {
            //Scan Hokuyo
            EthernetSensorData = utm.EthernetScan();
            //Estimate Current Pose
            changePose = Kinematics.UpdatePose(stepsR, stepsL, currentPose);
            //Particle filtering
            Swarm.swarm_move((int)changePose[Pose.Xpos], (int)changePose[Pose.Ypos], (int)changePose[Pose.Heading]);
            Swarm.swarm_update(EthernetSensorData);
            for (int j = 0; j < 5; j++)
            {
                EthernetSensorData = utm.EthernetScan();
                Swarm.swarm_move(0, 0, 0);
                Swarm.swarm_update(EthernetSensorData);
            }
            //Update Current Pose to Particle filter outputs
            currentPose[Pose.Xpos] = Swarm.swarm_get_best_x();
            currentPose[Pose.Ypos] = Swarm.swarm_get_best_y();
            currentPose[Pose.Heading] = Swarm.swarm_get_best_theta();
        }

        //Call each time to reset the Hall Count for all of the Roboteqs in config
        public void ResetCounters()
        {

            //foreach (Controllers.RoboteQ roboteq in lunabotics.RCU.Program.roboteqs)
            //{
            //    roboteq.ClearHallSensorCounts();
            //}
            for (int i = 0; i < 2; i++)
            {
                Controllers.RoboteQ roboteq = lunabotics.RCU.Program.roboteqs[i];
                roboteq.ClearHallSensorCounts();
                Thread.Sleep(110);
            }
        }

        public void turnToThetaZero()
        {
            EthernetSensorData = utm.EthernetScan();
            while (((EthernetSensorData[0] + EthernetSensorData[720]) > 4000 || (EthernetSensorData[0] + EthernetSensorData[720]) < 3760) && EthernetSensorData[361] > 400)
            {
                outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
                //Velocity must be negative for left turn
                outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 300;
                outputState = MergeStates(outputState, staticOutput);
                OnAutonomyUpdated(new AutonomyArgs(outputState));
                EthernetSensorData = utm.EthernetScan();
            }
            outputState[Comms.CommandEncoding.CommandFields.TranslationalVelocity] = 0;
            outputState[Comms.CommandEncoding.CommandFields.RotationalVelocity] = 0;
            outputState = MergeStates(outputState, staticOutput);
            OnAutonomyUpdated(new AutonomyArgs(outputState));
        }

        public void turnToGivenHeading(double desiredTheta)
        {
            int angleError;
            double spd = 4.35;
            while ((Math.Abs(desiredTheta - currentPose[Pose.Heading]) > 5))
            {
                angleError = (int)Math.Round(desiredTheta - currentPose[Pose.Heading]);
                Console.WriteLine("desired: " + desiredTheta + " current heading: " + currentPose[Pose.Heading] + " error: " + angleError);
            //Correct Left
            if (angleError > 0 && Math.Abs(angleError) < 180)
            {
                Console.WriteLine("Case 1, turning left " + angleError *spd + " steps");
                Move((int)(Math.Abs(angleError) * spd), direction.left);
            }
            //Correct Right
            else if (angleError > 0 && Math.Abs(angleError) >= 180)
            {
                Console.WriteLine("Case 2, turning right " + (180 - Math.Abs(angleError)) * spd + " steps");

                Move((180 - (int)(Math.Abs(angleError) * spd)), direction.right);
            }
            //Correct Right    
            else if (angleError < 0 && Math.Abs(angleError) < 180)
            {
                Console.WriteLine("Case 3, turning right " + Math.Abs(angleError) * spd + " steps");

                Move((int)(Math.Abs(angleError) * spd), direction.right);
            }
            //Correct Left
            else if (angleError < 0 && Math.Abs(angleError) >= 180)
            {
                Console.WriteLine("Case 4, turning left " + (180 - Math.Abs(angleError)) * spd + " steps");
                Move((180 - (int)(Math.Abs(angleError) * spd)), direction.left);
            }
            Thread.Sleep(100);
            }

        }
        #endregion


        #region Properties
        public bool Active
        {
            get { return active; }
        }
        public bool Started
        {
            get { return started; }
        }
        #endregion
    }

    /// <summary>
    /// Autonomy state feedback
    /// </summary>
    public class AutonomyArgs : EventArgs
    {
        private Dictionary<CommandFields, short> updatedState;

        public AutonomyArgs(Dictionary<CommandFields, short> updatedState)
        {
            if (updatedState == null)
                throw new ArgumentNullException("Updated state is null");

            this.updatedState = updatedState;
        }

        public Dictionary<CommandFields, short> UpdatedState
        {
            get { return updatedState; }
        }
    }
}
