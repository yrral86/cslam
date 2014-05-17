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
            DepositionNoFeedback,
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
        private int[] EthernetSensorDepositionData;
        private int[] SerialSensorData;
        #endregion

        #region Fields
        //Mining Data
        private int scoopsMined = 0;
        private int hasMined = 0;

        //Initialize motor powers (Note forward power is negative)
        public short forwardPower = 800;
        public short reversePower = -800;
        public short rightPower = -400;
        public short leftPower = 400;

        public int lane_width = 750;

        private bool active; // Thread started
        private bool started; // State set to run
        private readonly object timerSync = new object();
        private AutonomyConfiguration configuration;
        //private Deposition depositionAlgorithm;
        private Dictionary<CommandFields, short> staticOutput;
        private Robot robot;
        private State state;

        private get_distance_serial urg = new get_distance_serial();
        private get_distance_ethernet utm = new get_distance_ethernet();
        //string ip_address = "192.168.1.8";
        //int port_number = 10940;
        //NetworkStream stream;
        //TcpClient urg = new TcpClient();

        private Stopwatch stopwatch;
        private Stopwatch dumpClock;
        private System.Timers.Timer timer;
        
        // Create output state
        private Dictionary<CommandFields, short> outputState = new Dictionary<CommandFields, short>();
        //Create telemetry state
        private Dictionary<Configuration.Telemetry, int> outputTelemetry = new  Dictionary<Configuration.Telemetry, int>();
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
            dumpClock = new Stopwatch();

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
            int i = 0;
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
            //Reset Hall Counts From previous runs
            ResetCounters();

            /*Test of Pose Telemetry
            while (true)
            {

                outputTelemetry[Configuration.Telemetry.LocalizationX] = i;
                outputTelemetry[Configuration.Telemetry.LocalizationY] = i + 1;
                outputTelemetry[Configuration.Telemetry.LocalizationPsi] = i - 1;
                OnTelemetryUpdated(new TelemetryEventArgs((int)configuration.Interval, outputTelemetry));
                Thread.Sleep(1000);
                i++;
            }
            */
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
                            outputState[Comms.CommandEncoding.CommandFields.ScoopPivot] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.ScoopPitch] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.LeftBucketActuator] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RightBucketActuator] = 0;
                            outputState[Comms.CommandEncoding.CommandFields.RangeFinderEnable] = 0;
                            break;

                        case State.InitializeAutonomy:
                            int tries = 0;

                            //Initialize position - temporary 
                            //StartingAutonomy();
                            //Initialize Particle Filter
                            Swarm.swarm_init(721, 180, 7300, 4100, 1940, (int)configuration.SensorRadius);
                            //Move(300, direction.forward);
                            //Move(300, direction.right);
                            //Move(300, direction.left);
                            //Move(300, direction.reverse);
                            //state = State.Manual;
                            //break;

                            EthernetSensorData = utm.EthernetScan();

                            //Loop to try and converge particles
                            while (Swarm.swarm_converged() == 0 && tries < 200)
                            {
                                Swarm.swarm_move(0, 0, 0);
                                Swarm.swarm_update(EthernetSensorData);
                                EthernetSensorData = utm.EthernetScan();
                                Swarm.swarm_update_finalize();
                                tries++;
                            }

                            if (tries == 200)
                            {
                                currentPose[Pose.Xpos] = 9999;
                                currentPose[Pose.Ypos] = 9999;
                                currentPose[Pose.Heading] = 9999;
                            }
                            else
                            {
                                currentPose[Pose.Xpos] = Swarm.swarm_get_best_x();
                                currentPose[Pose.Ypos] = Swarm.swarm_get_best_y();
                                currentPose[Pose.Heading] = Swarm.swarm_get_best_theta();
                            }
                            outputTelemetry[Configuration.Telemetry.LocalizationX] = (int)currentPose[Pose.Xpos];
                            outputTelemetry[Configuration.Telemetry.LocalizationY] = (int)currentPose[Pose.Ypos];
                            outputTelemetry[Configuration.Telemetry.LocalizationPsi] = (int)currentPose[Pose.Heading];
                            OnTelemetryUpdated(new TelemetryEventArgs((int)configuration.Interval, outputTelemetry));
                            Thread.Sleep(5000);
                            //turnToGivenHeading(0);
                            state = State.TraverseClearPath;
                            //state = State.TemporaryTesting;
                            break;

                        case State.TemporaryTesting:

                            forwardPower = 500;
                            reversePower = -500;

                            while (currentPose[Pose.Xpos] < 5100)
                            {
                                Move(300, direction.forward);

                            }

                            turnToGivenHeading(90);

                            while (currentPose[Pose.Ypos] < 2800)
                            {
                                Move(300, direction.forward);
                            }

                            turnToGivenHeading(180);

                            while (currentPose[Pose.Xpos] > 1900)
                            {
                                Move(300, direction.forward);
                            }

                            turnToGivenHeading(-90);

                            while (currentPose[Pose.Ypos] > 1500)
                            {
                                Move(300, direction.forward);
                            }
                            
                            turnToGivenHeading(0);

                            state = State.TemporaryTesting;
                            break;

                        case State.TraverseClearPath:
                            SetArmSwingAndScoopPitch(145, 0);
                            while (currentPose[Pose.Xpos] < 4440)
                            {
                                if (currentPose[Pose.Ypos] < configuration.RoverLane - lane_width / 2)
                                    turnToGivenHeading(20);
                                else if (currentPose[Pose.Ypos] > configuration.RoverLane + lane_width / 2)
                                    turnToGivenHeading(-20);
                                else
                                    turnToGivenHeading(0);

                                if (currentPose[Pose.Xpos] < 4000)
                                    Move(300, direction.forward);
                                else
                                    Move(MMToSteps((int)(5000 - currentPose[Pose.Xpos])), direction.forward);
                                
                            }

                            state = State.Mining;
                            //state = State.ReturnToDeposition;
                            break;

                        case State.Mining:
                            scoopsMined = 0;
                            if (hasMined == 1)
                            {
                                if (configuration.RoverLane < 1940)
                                {
                                    turnToGivenHeading(90);
                                    while (currentPose[Pose.Xpos] < (configuration.RoverLane + configuration.RoverWidth * 2))
                                        Move(MMToSteps((int)(configuration.RoverLane + configuration.RoverWidth * 2 - currentPose[Pose.Xpos])), direction.forward);
                                }
                                else
                                {

                                    turnToGivenHeading(-90);
                                    while (currentPose[Pose.Xpos] < (configuration.RoverLane - configuration.RoverWidth * 2))
                                        Move(MMToSteps((int)(currentPose[Pose.Xpos] - (configuration.RoverLane - configuration.RoverWidth * 2))), direction.forward);

                                }
                            }
                            else if (hasMined > 1)
                            {
                                Move(1200, direction.left);
                                Move(1200, direction.right);
                                state = State.Manual;
                                
                            }

                            while (currentPose[Pose.Xpos] < 5365 && scoopsMined < configuration.MaximumScoops)
                            {
                                turnToGivenHeading(0);
                                MineScoop();
                                scoopsMined++;
                                if (stopwatch.ElapsedMilliseconds > 45000)
                                {
                                    state = State.ReturnToDeposition;
                                    break;
                                }
                            }
                            hasMined++;
                            state = State.ReturnToDeposition;
                            break;

                        case State.ReturnToDeposition:

                            while (currentPose[Pose.Xpos] > 2000)
                            {
                                if (currentPose[Pose.Ypos] < configuration.RoverLane - lane_width / 2)
                                    turnToGivenHeading(-20);
                                else if (currentPose[Pose.Ypos] > configuration.RoverLane + lane_width / 2)
                                    turnToGivenHeading(20);
                                else
                                    turnToGivenHeading(0);

                                Move(300, direction.reverse);
                            }

                            state = State.Deposition;
                            break;

                        case State.Deposition:
                            short oldForwardPower, oldReversePower;
                            SetArmSwingAndScoopPitch(60, 0);
                            while (currentPose[Pose.Xpos] > configuration.SensorRadius + 3)
                            {
                                if (currentPose[Pose.Ypos] < 1940 - lane_width/2)
                                    turnToGivenHeading(-90);
                                else if (currentPose[Pose.Ypos] > 1940 + lane_width/2)
                                    turnToGivenHeading(90);
                                else
                                    turnToGivenHeading(0, 1);


                                if (Math.Abs(currentPose[Pose.Heading]) < 1)
                                {
                                    oldForwardPower = forwardPower;
                                    oldReversePower = reversePower;
                                    forwardPower = 200;
                                    reversePower = -200;
                                    if (robot.TelemetryFeedback.BucketPivotAngle < 30 && robot.TelemetryFeedback.ArmSwingAngle < 75)
                                    {
                                        outputState[CommandFields.LeftBucketActuator] = 1000;
                                        outputState[CommandFields.RightBucketActuator] = 1000;
                                    }
                                    else
                                    {

                                        outputState[CommandFields.LeftBucketActuator] = 0;
                                        outputState[CommandFields.RightBucketActuator] = 0;
                                    }
                                    Move(MMToSteps((int)(currentPose[Pose.Xpos] - configuration.SensorRadius)), direction.reverse);
                                    forwardPower = oldForwardPower;
                                    reversePower = oldReversePower;
                                }
                                else
                                    Move(MMToSteps((int)Math.Abs(currentPose[Pose.Ypos] - configuration.RoverLane)), direction.reverse);

                            }

                            SetBucketAngle(90);
                            Thread.Sleep(10000);
                            Move(50, direction.forward);
                            Move(50, direction.reverse);
                            SetBucketAngle(0);

                            state = State.TraverseClearPath;
                            break;
                        case State.DepositionNoFeedback:
                            while (currentPose[Pose.Xpos] > 600)
                            {
                                if (currentPose[Pose.Ypos] < 1940 - lane_width / 2)
                                    turnToGivenHeading(-90);
                                else if (currentPose[Pose.Ypos] > 1940 + lane_width / 2)
                                    turnToGivenHeading(90);
                                else
                                    turnToGivenHeading(0);


                                if (Math.Abs(currentPose[Pose.Heading]) < 5)
                                {
                                    forwardPower = 200;
                                    reversePower = -200;
                                    if (robot.TelemetryFeedback.BucketPivotAngle < 30)
                                    {
                                        outputState[CommandFields.LeftBucketActuator] = 1000;
                                        outputState[CommandFields.RightBucketActuator] = 1000;
                                    }
                                    else
                                    {
                                        outputState[CommandFields.LeftBucketActuator] = 0;
                                        outputState[CommandFields.RightBucketActuator] = 0;
                                    }
                                    Move(MMToSteps((int)(currentPose[Pose.Xpos] - 600)), direction.reverse);
                                    forwardPower = 800;
                                    reversePower = -800;
                                }
                                else
                                    Move(MMToSteps((int)Math.Abs(currentPose[Pose.Ypos] - 1940)), direction.reverse);

                            }

                            dumpClock.Start();
                            while (dumpClock.ElapsedMilliseconds < 70000)
                            {
                                outputState[CommandFields.LeftBucketActuator] = 1000;
                                outputState[CommandFields.RightBucketActuator] = 1000;
                                outputState = MergeStates(outputState, staticOutput);
                                //Make calls to event handler to move motors
                                OnAutonomyUpdated(new AutonomyArgs(outputState));
                            }
                            Thread.Sleep(5000);
                            dumpClock.Restart();
                            while (dumpClock.ElapsedMilliseconds < 70000)
                            {
                                outputState[CommandFields.LeftBucketActuator] = -1000;
                                outputState[CommandFields.RightBucketActuator] = -1000;
                                outputState = MergeStates(outputState, staticOutput);
                                //Make calls to event handler to move motors
                                OnAutonomyUpdated(new AutonomyArgs(outputState));
                            }
                            state = State.TraverseClearPath;
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

        public int detectObstacle()
        // Returns a int variable stating that there is/ is not an obstacle directly in front of the MMV.
        // 1 indicates obstacle on right
        // -1 indicates obstacle on left
        // 0 indicates no obstacle
        // 2 indicates obstacle on left and right
        {
            int obstacle = 0;
            int tolerance = 25;
            int repeats = 0;
            double test;
            double slope;
            double one, two, three;
            SerialSensorData = urg.SerialScan();

            one = (SerialSensorData[206] + SerialSensorData[207] + SerialSensorData[208]) / 3;
            two = (SerialSensorData[97] + SerialSensorData[98] + SerialSensorData[99]) / 3;
            three = (SerialSensorData[0] + SerialSensorData[1] + SerialSensorData[2]) / 3;

            slope = Math.Abs(three - two) / 94;
            test = three + slope * 2;
            for (int i = 289; i < 382 && obstacle == 0; i++)
            {
                test += slope;
                if (SerialSensorData[i] > (test - tolerance) && SerialSensorData[i] < (test + tolerance))
                    repeats = 0;
                else
                {
                    repeats++;
                    if (repeats > 5)
                        obstacle = -1;
                }
            }

            slope = Math.Abs(two - one) / 109;
            test = two + slope * 2;
            for (int i = 386; i < 491 && obstacle < 1; i++)
            {
                test += slope;
                if (SerialSensorData[i] > (test - tolerance) && SerialSensorData[i] < (test + tolerance))
                    repeats = 0;
                else
                {
                    repeats++;
                    if (repeats > 5)
                        if (obstacle == -1)
                            obstacle = 2;
                        else
                            obstacle = 1;
                }
            }

            return obstacle;
        }

        public int MMToSteps(int mm)
        {
            //return (int)(mm / 0.7023);
            return mm;
        }

        public bool MineScoop()
        {
            bool mined = false;
            if (currentPose[Pose.Xpos] < 6530)
            {
                Stopwatch st = new Stopwatch();
                SetArmSwingAndScoopPitch(5, 8);
                Move(150, direction.forward);
                SetArmSwingAndScoopPitch(145, 0);
                st.Start();
                while (st.ElapsedMilliseconds < 5000)
                {
                    outputState[Comms.CommandEncoding.CommandFields.ScoopPitch] = -1000;
                    //Set Output States
                    outputState = MergeStates(outputState, staticOutput);
                    //Make calls to event handler to move motors
                    OnAutonomyUpdated(new AutonomyArgs(outputState));
                }
                st.Stop();
                SetArmSwingAndScoopPitch(60, 8);
                mined = true;
            }
            return mined;
        }

        public void SetArmSwingAndScoopPitch(int arm_angle, int scoop_angle)
        {
            int arm_pivot_error, scoop_pitch_error;
            arm_pivot_error = arm_angle - (int)robot.TelemetryFeedback.ArmSwingAngle;
            scoop_pitch_error = scoop_angle - (int)robot.TelemetryFeedback.ScoopPitchAngle;
            while (Math.Abs(arm_pivot_error) > 0 && Math.Abs(scoop_pitch_error) > 0)
            {
                outputState[Comms.CommandEncoding.CommandFields.ScoopPitch] = (short)(1000 * scoop_pitch_error / Math.Abs(scoop_pitch_error));
                outputState[Comms.CommandEncoding.CommandFields.ScoopPivot] = (short)(1000 * arm_pivot_error / Math.Abs(arm_pivot_error));

                //Set Output States
                outputState = MergeStates(outputState, staticOutput);
                //Make calls to event handler to move motors
                OnAutonomyUpdated(new AutonomyArgs(outputState));

                arm_pivot_error = arm_angle - (int)robot.TelemetryFeedback.ArmSwingAngle;
                scoop_pitch_error = scoop_angle - (int)robot.TelemetryFeedback.ScoopPitchAngle;
                if (Math.Abs(arm_pivot_error) < 2)
                    arm_pivot_error = 0;
                if (Math.Abs(scoop_pitch_error) < 2)
                    scoop_pitch_error = 0;
            }

            //Stop motors
            outputState[Comms.CommandEncoding.CommandFields.ScoopPitch] = 0;
            outputState[Comms.CommandEncoding.CommandFields.ScoopPivot] = 0;
            outputState = MergeStates(outputState, staticOutput);
            OnAutonomyUpdated(new AutonomyArgs(outputState));
        }

        public void SetBucketAngle(int angle)
        {
            int angle_error;
            angle_error = angle - (int)robot.TelemetryFeedback.BucketPivotAngle;
            while (Math.Abs(angle_error) > 0)
            {
                outputState[CommandFields.LeftBucketActuator] = (short)(1000 * angle_error / Math.Abs(angle_error));
                outputState[CommandFields.RightBucketActuator] = (short)(1000 * angle_error / Math.Abs(angle_error));

                //Set Output States
                outputState = MergeStates(outputState, staticOutput);
                //Make calls to event handler to move motors
                OnAutonomyUpdated(new AutonomyArgs(outputState));
               
                angle_error = angle - (int)robot.TelemetryFeedback.BucketPivotAngle;
                if (Math.Abs(angle_error) < 2)
                    angle_error = 0;
            }

            //Stop motors
            outputState[CommandFields.RightBucketActuator] = 0;
            outputState[CommandFields.LeftBucketActuator] = 0;
            outputState = MergeStates(outputState, staticOutput);
            OnAutonomyUpdated(new AutonomyArgs(outputState));
        }

        public void Move(int steps, direction dir)
        {
            //475 Steps to move .333 meters
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
            if (dir == direction.reverse)
                steps *= -1;
            ParticleFiltering(steps, steps);
        }

        public void ParticleFiltering(int stepsR, int stepsL)
        {
            int tries = 0;

            //Estimate Current Pose
            changePose = Kinematics.UpdatePose(stepsR, stepsL, currentPose);
            //Particle filtering
            Swarm.swarm_move((int)changePose[Pose.Xpos], (int)changePose[Pose.Ypos], (int)changePose[Pose.Heading]);

            //Scan Hokuyo
            EthernetSensorData = utm.EthernetScan();
            while (Swarm.swarm_converged() == 0 && tries < 200)
            {
                Swarm.swarm_update(EthernetSensorData);
                EthernetSensorData = utm.EthernetScan();
                Swarm.swarm_update_finalize();
                tries++;
                if (Swarm.swarm_converged() == 0 && tries < 200)
                    Swarm.swarm_move(0, 0, 0);
            }

            //Update Current Pose to Particle filter outputs
            if (tries == 200)
            {
                currentPose[Pose.Xpos] = 9999;
                currentPose[Pose.Ypos] = 9999;
                currentPose[Pose.Heading] = 9999;
            }
            else
            {
                currentPose[Pose.Xpos] = Swarm.swarm_get_best_x();
                currentPose[Pose.Ypos] = Swarm.swarm_get_best_y();
                currentPose[Pose.Heading] = Swarm.swarm_get_best_theta();
            }

            outputTelemetry[Configuration.Telemetry.LocalizationX] = (int)currentPose[Pose.Xpos];
            outputTelemetry[Configuration.Telemetry.LocalizationY] = (int)currentPose[Pose.Ypos];
            outputTelemetry[Configuration.Telemetry.LocalizationPsi] = (int)currentPose[Pose.Heading];
            OnTelemetryUpdated(new TelemetryEventArgs((int)configuration.Interval, outputTelemetry));
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

        public void turnToGivenHeading(double desiredTheta, double tol = 5)
        {
            int angleError = (int)Math.Round(desiredTheta - currentPose[Pose.Heading]);
            double spd = 4.35;

	    if (angleError > 180)
	       angleError -= 360;
	    if (angleError < -180)
	       angleError += 360;

            while ((Math.Abs(angleError) > tol))
            {
                angleError = (int)Math.Round(desiredTheta - currentPose[Pose.Heading]);

		if (angleError > 180)
	       	   angleError -= 360;
	    	if (angleError < -180)
	       	   angleError += 360;

                Console.WriteLine("desired: " + desiredTheta + " current heading: " + currentPose[Pose.Heading] + " error: " + angleError);
            //Correct Left
            if (angleError > 0)
            {
                Console.WriteLine("Case 1, turning left " + angleError *spd + " steps");
                Move((int)(Math.Abs(angleError) * spd), direction.left);
            }
            //Correct Right    
            else if (angleError < 0)
            {
                Console.WriteLine("Case 2, turning right " + Math.Abs(angleError) * spd + " steps");

                Move((int)(Math.Abs(angleError) * spd), direction.right);
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
        public double X
        {
            get { return currentPose[Pose.Xpos]; }
        }
        public double Y
        {
            get { return currentPose[Pose.Ypos]; }
        }
        public double Psi
        {
            get { return currentPose[Pose.Heading]; }
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
