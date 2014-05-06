using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace lunabotics.OCU.Visual_Controls
{
    public abstract class CustomVisualHost : FrameworkElement
    {
        private VisualCollection _children;

        public CustomVisualHost()
        {
            _children = new VisualCollection(this);

            ///*could add some initial children here*/

            ///*could install mouse handlers here*/

            //DrawingVisual dv = new DrawingVisual();
            //var context = dv.RenderOpen();
            //context.DrawRectangle(Brushes.LightBlue, (Pen)null, new Rect(new Point(50, 50), new Size(25, 25)));
            //context.DrawEllipse(Brushes.Yellow, (Pen)null, new Point(50, 50), 5, 5);
            //context.Close();

            //_children.Add(dv);
        }

        // Provide a required override for the VisualChildrenCount property.
        protected override int VisualChildrenCount
        {
            get { return _children.Count; }
        }

        // Provide a required override for the GetVisualChild method.
        protected override Visual GetVisualChild(int index)
        {
            if (index < 0 || index >= _children.Count)
            {
                throw new ArgumentOutOfRangeException();
            }

            return _children[index];
        }

    }

    /*Basic way to create an object to be drawn...
     * 
     * DrawingVisual drawingVisual = new DrawingVisual();
     * DrawingContext drawingContext = drawingVisual.RenderOpen();
     * 
     * drawingContext.DrawRectangle(blah blah);
     * drawingContext.Close();
     * 
     * Add the DrawingVisual to _children
     */
}