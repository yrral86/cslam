using lunabotics.Configuration;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Utility;


namespace lunabotics.RCU.Autonomy
{
    public class MonteCarloLocalization
    {
        #region Enums
        public enum ExpectedZone
        {
            Starting,
            Mining,
            Any
        }
        #endregion

        #region Constants
        private readonly double DigX = 444.0; // cm
        private readonly double FieldHeight = 388.0; // cm
        private readonly double FieldWidth = 738.0; // cm
        private readonly double ObstacleX = 150.0; // cm
        #endregion

        #region Fields
        private double confidence; // 0 = No confidence, 100 = Complete convergence/good confidence
        private AutonomyConfiguration configuration;
        private Field field;
        private List<Particle> particles;
        private Tracker tracker;
        #endregion

        #region Constructor
        public MonteCarloLocalization(AutonomyConfiguration configuration)
        {
            // Assign fields
            this.configuration = configuration;

            // Initialize random number generator
            MathHelper.random = new Random((int)DateTime.Now.Ticks);

            // Initialize field
            field = new Field(FieldWidth, FieldHeight, DigX, ObstacleX, configuration.WallMargin);
        }
        #endregion

        #region Methods
        public void Initialize(Robot robot)
        {
            // Initialize particles
            particles = new List<Particle>(configuration.NumberOfParticles);
            for (int i = 0; i < configuration.NumberOfParticles; i++)
            {
                switch (robot.ExpectedZone)
                {
                    case Robot.Zone.Starting:
                        particles.Add(new Particle(field,
                            configuration.WallMargin + MathHelper.random.NextDouble() * (ObstacleX - 2.0 * configuration.WallMargin),
                            configuration.WallMargin + MathHelper.random.NextDouble() * (FieldHeight - 2.0 * configuration.WallMargin),
                            MathHelper.random.NextDouble() * 360.0));
                        break;
                    case Robot.Zone.Mining:
                        particles.Add(new Particle(field,
                            DigX + configuration.WallMargin + MathHelper.random.NextDouble() * (DigX - 2.0 * configuration.WallMargin),
                            configuration.WallMargin + MathHelper.random.NextDouble() * (FieldHeight - 2.0 * configuration.WallMargin),
                            MathHelper.random.NextDouble() * 360.0));
                        break;
                    case Robot.Zone.Unknown:
                        particles.Add(new Particle(field,
                            configuration.WallMargin + MathHelper.random.NextDouble() * (FieldWidth - 2.0 * configuration.WallMargin),
                            configuration.WallMargin + MathHelper.random.NextDouble() * (FieldHeight - 2.0 * configuration.WallMargin),
                            MathHelper.random.NextDouble() * 360.0));
                        break;
                    default:
                        break;
                }
            }

            // Update tracker
            tracker = new Tracker();
            tracker.FindCenter(particles);
        }

        public void Update(double elapsedTime, Robot robot, lunabotics.RCU.Autonomy.AutonomyHandler.Zone zone, bool useFrontRangeFinder)
        {
            // Move particles
            particles.ForEach(p =>
            {
                p.SetVelocities(robot.TranslationalVelocity, robot.RotationalVelocity);
                p.Move(elapsedTime);
            });

            // Blast
            for (int i = 0; i < configuration.NumberOfParticles * configuration.BlastPercent; i++)
            {
                double magnitude = configuration.BlastRadius * MathHelper.random.NextDouble();
                double angle = MathHelper.random.NextDouble() * 2.0d * Math.PI;

                particles[MathHelper.random.Next(configuration.NumberOfParticles)].Set(
                    tracker.X + magnitude * Math.Cos(angle),
                    tracker.Y + magnitude * Math.Sin(angle),
                    MathHelper.random.NextDouble() * 360.0);
            }

            // Re-initialize
            for (int i = 0; i < configuration.NumberOfParticles * configuration.RandomReInitPercent; i++)
            {
                particles[MathHelper.random.Next(configuration.NumberOfParticles)].Set(
                    configuration.WallMargin + MathHelper.random.NextDouble() * (FieldWidth - 2.0 * configuration.WallMargin),
                    configuration.WallMargin + MathHelper.random.NextDouble() * (FieldHeight - 2.0 * configuration.WallMargin),
                    MathHelper.random.NextDouble() * 360.0);
            }

            // Kill lost particles
            foreach (Particle particle in particles)
            {
                if (!field.RectangleStarting.Contains((int)particle.X, (int)particle.Y))
                {
                    // Regenerate particle
                    switch (zone)
                    {
                        case AutonomyHandler.Zone.Start:
                            particle.Set(
                                configuration.WallMargin + MathHelper.random.NextDouble() * (ObstacleX - 2.0 * configuration.WallMargin),
                                configuration.WallMargin + MathHelper.random.NextDouble() * (FieldHeight - 2.0 * configuration.WallMargin),
                                MathHelper.random.NextDouble() * 360.0);
                            break;
                        case AutonomyHandler.Zone.Obstacle:
                            particle.Set(
                                ObstacleX + configuration.WallMargin + MathHelper.random.NextDouble() * ((DigX - ObstacleX) - 2.0 * configuration.WallMargin),
                                configuration.WallMargin + MathHelper.random.NextDouble() * (FieldHeight - 2.0 * configuration.WallMargin),
                                MathHelper.random.NextDouble() * 360.0);
                            break;
                        case AutonomyHandler.Zone.Mining:
                            particle.Set(
                                DigX + configuration.WallMargin + MathHelper.random.NextDouble() * ((FieldWidth - DigX) - 2.0 * configuration.WallMargin),
                                configuration.WallMargin + MathHelper.random.NextDouble() * (FieldHeight - 2.0 * configuration.WallMargin),
                                MathHelper.random.NextDouble() * 360.0);
                            break;
                        case AutonomyHandler.Zone.Unknown:
                            particle.Set(
                                configuration.WallMargin + MathHelper.random.NextDouble() * (FieldWidth - 2.0 * configuration.WallMargin),
                                configuration.WallMargin + MathHelper.random.NextDouble() * (FieldHeight - 2.0 * configuration.WallMargin),
                                MathHelper.random.NextDouble() * 360.0);
                            break;
                        default:
                            break;
                    }
                }
            }

            // Measure robot sensors
            double[] z = robot.ReadSensors(useFrontRangeFinder);

            // Initialize weight sum
            double weightSum = 0.0;

            // Update particle weights
            particles.ForEach(p => weightSum += p.UpdateWeight(z, useFrontRangeFinder));

            // Store elite
            Particle elite = particles.OrderByDescending(p => p.Weight).First();

            // Re-weight particles
            double sum = 0.0;
            particles.ForEach(p =>
            {
                double normalizedWeight = p.Weight / weightSum;
                p.NormalizedWeight = sum + normalizedWeight;
                sum += normalizedWeight;
            });

            // Generate new population
            List<Particle> newParticles = new List<Particle>(configuration.NumberOfParticles);
            for (int i = 0; i < configuration.NumberOfParticles; i++)
            {
                // Get random
                double random = MathHelper.random.NextDouble();

                for (int j = 0; j < configuration.NumberOfParticles; j++)
                {
                    if (random < particles[j].NormalizedWeight)
                    {
                        //Debug.WriteLine(random + ":" + j);
                        newParticles.Add((Particle)particles[j].Clone());
                        break;
                    }
                }
            }

            // Put elite back into the population
            newParticles[MathHelper.random.Next(configuration.NumberOfParticles)] = (Particle)elite.Clone();

            // Calculate confidence
            double confSum = 0.0d;
            particles.ForEach(p => confSum += p.Weight);
            this.confidence = confSum / (double)configuration.NumberOfParticles;

            // Overwrite particles
            particles = newParticles;

            // Update tracker position
            tracker.FindCenter(particles);
        }
        #endregion

        #region Properties
        public double Confidence
        {
            get { return confidence; }
        }
        public Tracker Tracker
        {
            get { return tracker; }
        }
        #endregion
    }
}
