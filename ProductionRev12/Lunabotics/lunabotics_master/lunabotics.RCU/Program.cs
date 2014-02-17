using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Configuration;
using System.Xml.Serialization;
using System.IO;
using System.Threading;
using lunabotics.RCU.Autonomy;
using lunabotics.RCU.Models;
using lunabotics.Comms;
using lunabotics.Comms.CommandEncoding;
using lunabotics.RCU.Controllers;
using lunabotics.Comms.TelemetryEncoding;
using System.Diagnostics;

namespace lunabotics.RCU
{
    class Program
    {
        static bool autonomous = false;
        static bool useAutonomy = true;
        static bool useRangeFinders = true;
        static bool useRoboteQs = true;
        //static bool useServoController = true;
        static bool useWebcams = true;
        static AutonomyHandler autonomy;
        
        static RCUConfiguration configuration;

        static List<Controllers.RoboteQ> roboteqs = new List<Controllers.RoboteQ>();
        static Mode mode;
        static RangeFinder frontRangeFinder;
        static RangeFinder rearRangeFinder;
        //static PhidgetServo servo;
        static Stopwatch stopwatch;
        static TelemetryFeedback feedback;
        static Webcam bucketCam, frontCam, rearCam;
        static Comms.UDP_Receiver receiver;

        static Telemetry.TelemetryHandler telemetryHandler;
        //static Kinematics.Kinematics kinematics = new Kinematics.Kinematics(1000);

        static CancellationTokenSource tokenSource = new CancellationTokenSource();
        static Thread stateProcessor;
        static Utility.UpdateQueue<Dictionary<Comms.CommandEncoding.CommandFields, short>> stateQueue = new Utility.UpdateQueue<Dictionary<Comms.CommandEncoding.CommandFields, short>>(-1);

        static void Main(string[] args)
        {
            //load config file from arguments
            foreach (string str in args)
            {
                char[] separator = { '=' };
                string[] arg_pair = str.Split(separator, 2, StringSplitOptions.RemoveEmptyEntries);

                switch (str)
                {
                    case "--noAutonomy":
                        useAutonomy = false;
                        break;
                    case "--noRangeFinder":
                        useRangeFinders = false;
                        break;
                    case "--noRoboteQ":
                        useRoboteQs = false;
                        break;
                    case "--noWebcam":
                        useWebcams = false;
                        break;
                        /*
                    case "--noServoController":
                        useServoController = false;
                        break;
                         */
                    default:
                        break;
                }

                if (arg_pair.Length < 2)
                    continue;
                
                switch (arg_pair[0])
                {
                    case "--config":
                        //load file...
                        XmlSerializer deserializer = new System.Xml.Serialization.XmlSerializer(typeof(RCUConfiguration));
                        try
                        {
                            using (StreamReader file_reader = new StreamReader(arg_pair[1]))
                            {
                                configuration = (RCUConfiguration)deserializer.Deserialize(file_reader);
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Error opening configuration file: " + ex.Message);
                        }
                        break;
                    default:
                        break;
                }
            }

            try
            {
                if (configuration == null)
                    throw new Exception("None or invalid configuration specified");

                mode = Mode.Manual;
                feedback = new TelemetryFeedback();
                stopwatch = new Stopwatch();

                telemetryHandler = new Telemetry.TelemetryHandler(configuration.TelemetryConfiguration, configuration.OCU_IP_Address);
                telemetryHandler.Activate();

                // Load RoboteQs
                if (useRoboteQs)
                {
                    foreach (var roboteq_config in configuration.RoboteqConfigurations)
                        roboteqs.Add(new Controllers.RoboteQ(roboteq_config));

                    foreach (Controllers.RoboteQ roboteq in roboteqs)
                    {
                        telemetryHandler.AddProvider(roboteq);
                        roboteq.Activate();
                    }
                }

                // Load rangefinders
                if (useRangeFinders)
                {
                    frontRangeFinder = new RangeFinder(configuration.FrontRangeFinder);
                    frontRangeFinder.Activate();

                    rearRangeFinder = new RangeFinder(configuration.RearRangeFinder);
                    rearRangeFinder.Activate();

                    telemetryHandler.AddRangeFinder(Telemetry.TelemetryHandler.RangeFinderLocation.Front, frontRangeFinder);
                    telemetryHandler.AddRangeFinder(Telemetry.TelemetryHandler.RangeFinderLocation.Rear, rearRangeFinder);
                }

                if (useWebcams)
                {
                    
                    frontCam = new Webcam(configuration.OCU_IP_Address, configuration.FrontCameraConfiguration);
                    Thread.Sleep(200);
                    frontCam.Activate();
                    Thread.Sleep(200);
                    
                    bucketCam = new Webcam(configuration.OCU_IP_Address, configuration.BucketCameraConfiguration);
                    Thread.Sleep(200);
                    bucketCam.Activate();
                    Thread.Sleep(200);
                    
                    rearCam = new Webcam(configuration.OCU_IP_Address, configuration.RearCameraConfiguration);
                    Thread.Sleep(200);
                    rearCam.Activate();
                    Thread.Sleep(200);
                    
                }
                /*
                // Create servo controller
                if (useServoController)
                {
                    servo = new PhidgetServo(configuration.PhidgetConfiguration);
                    servo.Activate();
                }
                */
                if (useAutonomy)
                {
                    // Start autonomy logic
                    autonomy = new AutonomyHandler(configuration.AutonomyConfiguration, ref telemetryHandler);
                    autonomy.AutonomyUpdated += autonomy_AutonomyUpdated;
                    autonomy.Activate();
                    telemetryHandler.AddProvider(autonomy);
                }

                //start listening...
                receiver = new Comms.UDP_Receiver(configuration.CommandPort);
                receiver.DataReceived += new EventHandler<Comms.DataArgs>(receiver_DataReceived);
                receiver.Activate();

                telemetryHandler.TelemetryFeedbackProcessed += telemetryHandler_TelemetryFeedbackProcessed;

                //packet handler
                stateProcessor = new Thread(new ThreadStart(StateProcessorDoWork));
                stateProcessor.Start();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error during startup: " + ex.Message);
                Teardown();
            }

            while (true)
            {
                try
                {
                    Console.WriteLine("Enter 'exit' to shutdown");
                    string input = Console.ReadLine().ToLower();
                    if (input.Contains("exit"))
                    {
                        break;
                    }
                }
                catch (Exception) { }
            }
            Teardown();
            // Pause
            Console.WriteLine("Press any key to close");
            Console.ReadKey();
        }


        static void telemetryHandler_TelemetryFeedbackProcessed(object sender, Comms.TelemetryEncoding.TelemetryFeedbackArgs e)
        {
            // Obtain measurements and copy to feedback object
            feedback = (TelemetryFeedback)e.UpdatedState.Clone();
        }

        static void autonomy_AutonomyUpdated(object sender, AutonomyArgs e)
        {
            // Enqueue state if not autonomous
            if (mode == Mode.Autonomous)
            {
                // Start autonomy if not already started
                if (!autonomy.Started)
                    autonomy.Start();

                // Queue commands from autonomy
                stateQueue.Enqueue(e.UpdatedState);
            }
        }

        private static void Teardown()
        {
            tokenSource.Cancel();
            //cleanup logic
            if (autonomy != null)
                autonomy.Deactivate();
            if (receiver != null)
                receiver.Deactivate();

            foreach (Controllers.RoboteQ r in roboteqs)
                r.Deactivate();
            /*
            if (servo != null)
                servo.Deactivate();
             */
            if (telemetryHandler != null)
                telemetryHandler.Deactivate();
            if (frontRangeFinder != null)
                frontRangeFinder.DeActivate();
            if (rearRangeFinder != null)
                rearRangeFinder.DeActivate();
            if (frontCam != null)
                frontCam.Deactivate();
            if (bucketCam != null)
                bucketCam.Deactivate();
            if (rearCam != null)
                rearCam.Deactivate();
        }

        static void receiver_DataReceived(object sender, Comms.DataArgs e)
        {
            try
            {
                Dictionary<Comms.CommandEncoding.CommandFields, short> state = Comms.CommandEncoding.Encoder.Decode(e.Data);
                if (useAutonomy)
                {
                    // Read mode
                    if (state.ContainsKey(CommandFields.Mode))
                        if ((Mode)state[CommandFields.Mode] != Mode.Null)
                        {
                            // Reset timer
                            stopwatch.Restart();
                            // Switch mode
                            mode = (Mode)state[CommandFields.Mode];
                            Console.WriteLine("Switched to: " + mode.ToString());
                        }

                    // Enqueue state if not autonomous
                    switch (mode)
                    {
                        case Mode.Manual:
                            // Stop autonomy if active
                            if (autonomy.Started)
                                autonomy.Stop();
                            // Enqueue current state from controller
                            stateQueue.Enqueue(state);
                            break;
                        case Mode.Autonomous:
                            // Do nothing
                            break;
                        case Mode.BinLowerMacro:
                            if (!feedback.BinLowerSwitchDepressed)
                            {
                                // Check limit switch
                                state[CommandFields.LeftBucketActuator] = -1000;
                                state[CommandFields.RightBucketActuator] = -1000;
                            }
                            else
                            {
                                mode = Mode.Manual;
                            }

                            // Enqueue current state from controller
                            stateQueue.Enqueue(state);

                            break;
                        case Mode.BinRaiseMacro:
                            if (!feedback.BinUpperSwitchDepressed)
                            {
                                // Check limit switch
                                state[CommandFields.LeftBucketActuator] = 1000;
                                state[CommandFields.RightBucketActuator] = 1000;
                            }
                            else
                            {
                                mode = Mode.Manual;
                            }

                            // Enqueue current state from controller
                            stateQueue.Enqueue(state);
                            break;
                        case Mode.CollectScoopMacro:
                            if (!feedback.BinUpperSwitchDepressed)
                            {
                                // Move actuators in and raise scoop
                                state[CommandFields.BucketPitch] = -1000;
                                state[CommandFields.BucketPivot] = 1000;
                                stopwatch.Restart();
                            }
                            else if (stopwatch.ElapsedMilliseconds < 700)
                            {
                                state[CommandFields.TranslationalVelocity] = 1000;
                            }
                            else
                            {
                                mode = Mode.Manual;
                            }

                            // Enqueue current state from controller
                            stateQueue.Enqueue(state);
                            break;
                        case Mode.DockRobotMacro:
                            // Calcuate orientation error
                            double orientationError = feedback.RearProximityLeft - feedback.RearProximityRight;

                            // Check orientation, TODO: un-hard-code
                            if (orientationError > 30)
                            {
                                double gain = 1.0; // TODO: tune this

                                state[CommandFields.RotationalVelocity] = (short)(orientationError * gain);
                                state[CommandFields.TranslationalVelocity] = 0;
                            }
                            else
                            {
                                // Calculate average distance
                                double distanceError = (feedback.RearProximityLeft + feedback.RearProximityRight) / 2.0d;

                                // Check distance error, TODO
                                if (distanceError > 15)
                                {
                                    // Backup slowly
                                    state[CommandFields.RotationalVelocity] = 0;
                                    state[CommandFields.TranslationalVelocity] = -500;
                                }
                                else
                                {
                                    // Close enough, move scoop all the way down
                                    state[CommandFields.BucketPivot] = -1000;
                                    // Check for done
                                    if (feedback.BinLowerSwitchDepressed)
                                    {
                                        // All done
                                        mode = Mode.Manual;
                                    }
                                }
                            }

                            // Enqueue current state from controller
                            stateQueue.Enqueue(state);
                            break;
                        default:

                            break;
                    }
                }
                else
                {
                    // Just load as usual
                    stateQueue.Enqueue(state);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error deserializing state: " + ex.Message);
            }
        }

        static void StateProcessorDoWork()
        {
            while (!tokenSource.Token.IsCancellationRequested)
            {
                try
                {
                    Dictionary<Comms.CommandEncoding.CommandFields, short> robotState = stateQueue.Dequeue(tokenSource.Token);

                    Dictionary<Configuration.Devices, int> deviceStates = new Dictionary<Devices, int>();

                    //first, retrieve wheel states
                    if (robotState.ContainsKey(CommandFields.TranslationalVelocity) && robotState.ContainsKey(CommandFields.RotationalVelocity))
                    {
                        // Get velocities
                        double translationalVelocity = Kinematics.GetTranslationalVelocity(robotState[Comms.CommandEncoding.CommandFields.TranslationalVelocity]);
                        double rotationalVelocity = Kinematics.GetRotationalVelocity(robotState[Comms.CommandEncoding.CommandFields.RotationalVelocity]);

                        // Calculate kinematics
                        deviceStates = MergeStates(deviceStates, Kinematics.GetWheelStates(
                            translationalVelocity, rotationalVelocity));
                    }

                    // Get servo command
                    if (robotState.ContainsKey(CommandFields.RangeFinderServo))
                        deviceStates[Devices.RangeFinderServo] = robotState[CommandFields.RangeFinderServo];
                    
                    //TEMPORARY KLUDGE:
                    if (robotState.ContainsKey(CommandFields.BucketPitch))
                        deviceStates[Devices.Actuators] = robotState[Comms.CommandEncoding.CommandFields.BucketPitch];
                    if (robotState.ContainsKey(CommandFields.BucketPivot))
                        deviceStates[Devices.Pivot] = robotState[Comms.CommandEncoding.CommandFields.BucketPivot];
                    if (robotState.ContainsKey(CommandFields.LeftBucketActuator))
                        deviceStates[Devices.LeftBucketActuator] = robotState[Comms.CommandEncoding.CommandFields.LeftBucketActuator];
                    if (robotState.ContainsKey(CommandFields.RightBucketActuator))
                        deviceStates[Devices.RightBucketActuator] = robotState[Comms.CommandEncoding.CommandFields.RightBucketActuator];

                    foreach (Controllers.RoboteQ roboteq in roboteqs)
                        roboteq.SetValues(deviceStates);
                    /*
                    if (servo != null)
                        servo.SetValues(deviceStates);
                    */
                    if (frontCam != null)
                        if (robotState.ContainsKey(Comms.CommandEncoding.CommandFields.FrontCameraState))
                            frontCam.UpdateState(Comms.States.VideoState.DecodeVideoState(robotState[CommandFields.FrontCameraState]));

                    if (bucketCam != null)
                        if (robotState.ContainsKey(Comms.CommandEncoding.CommandFields.BucketCameraState))
                            bucketCam.UpdateState(Comms.States.VideoState.DecodeVideoState(robotState[CommandFields.BucketCameraState]));
                }
                catch (OperationCanceledException ex)
                {
                    Console.WriteLine(ex.Message);
                    break;
                }
                catch (Exception ex)
                {
                    Console.WriteLine("StateProcessor: unhandled exception: " + ex.Message);
                }
            }
            Console.WriteLine("StateProcessor exiting...");
        }

        /// <summary>
        /// Copies all the keys from the right dictionary into the left dictionary.  If the left dictionary already contains a key in the right, 
        /// it is overwritten.
        /// </summary>
        /// <param name="left"></param>
        /// <param name="right"></param>
        /// <returns></returns>
        private static Dictionary<Configuration.Devices, int> MergeStates(Dictionary<Configuration.Devices, int> left, Dictionary<Configuration.Devices, int> right)
        {
            foreach (Configuration.Devices device in right.Keys)
            {
                left[device] = right[device];
            }

            return left;
        }

        private Dictionary<CommandFields, short> MergeCommands(Dictionary<CommandFields, short> left, Dictionary<CommandFields, short> right)
        {
            foreach (CommandFields commandField in right.Keys)
            {
                left[commandField] = right[commandField];
            }

            return left;
        }
    }
}
