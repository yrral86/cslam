using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;

namespace lunabotics.RCU.Autonomy
{
    public class Field
    {
        #region Fields
        private double height;
        private double width;
        private double obstacleX;
        private double digX;
        private Rectangle rectangleField;
        private Rectangle rectangleStarting;
        #endregion

        #region Constructor
        public Field(double width, double height, double digX, double obstacleX, double margin)
        {
            this.width = width;
            this.height = height;
            this.digX = digX;
            this.obstacleX = obstacleX;

            this.rectangleField = new Rectangle((int)margin, (int)margin, (int)(width - margin), (int)(height - margin));
            this.rectangleStarting = new Rectangle((int)margin, (int)margin, (int)(obstacleX - margin), (int)(height - margin));
        }
        #endregion

        #region Properties
        public double DigX
        {
            get { return digX; }
            set { digX = value; }
        }
        public double Height
        {
            get { return height; }
            set { height = value; }
        }
        public double ObstacleX
        {
            get { return obstacleX; }
            set { obstacleX = value; }
        }
        public double Width
        {
            get { return width; }
            set { width = value; }
        }
        public Rectangle RectangleField
        {
            get { return rectangleField; }
            set { rectangleField = value; }
        }
        public Rectangle RectangleStarting
        {
            get { return rectangleStarting; }
            set { rectangleStarting = value; }
        }
        #endregion
    }
}
