using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using lunabotics.OCU.Models;

namespace lunabotics.OCU.Visual_Controls
{
    public class LineDisplayControl : FrameworkElement
    {

        VisualCollection visualChildren;

        public static readonly DependencyProperty LineSourceProperty =
            DependencyProperty.Register("LineSource",
                typeof(ObservableNotifiableCollection<Line>),
                typeof(LineDisplayControl),
                new PropertyMetadata(OnLineSourceChanged));


        public static readonly DependencyProperty MaxRangeProperty =
            DependencyProperty.Register("MaxRange",
                typeof(double),
                typeof(LineDisplayControl),
                new PropertyMetadata(4000.0));

        public static readonly DependencyProperty OverlayModeProperty =
            DependencyProperty.Register("OverlayMode",
                typeof(bool),
                typeof(LineDisplayControl),
                new PropertyMetadata(false));
        public LineDisplayControl()
            : base()
        {
            this.Height = 200; this.Width = 400;
            visualChildren = new VisualCollection(this);
        }

        public ObservableNotifiableCollection<Line> LineSource
        {
            set { SetValue(LineSourceProperty, value); }
            get { return (ObservableNotifiableCollection<Line>)GetValue(LineSourceProperty); }
        }

        public double MaxRange
        {
            set { SetValue(MaxRangeProperty, value); }
            get { return (double)GetValue(MaxRangeProperty); }
        }

        public bool OverlayMode
        {
            set { SetValue(OverlayModeProperty, value); }
            get { return (bool)GetValue(OverlayModeProperty); }
        }

        static void OnLineSourceChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            (obj as LineDisplayControl).OnLineSourceChanged(args);
        }

        void OnLineSourceChanged(DependencyPropertyChangedEventArgs args)
        {
            visualChildren.Clear();

            if (args.OldValue != null)
            {
                ObservableNotifiableCollection<Line> coll = args.OldValue as ObservableNotifiableCollection<Line>;
                coll.CollectionCleared -= OnCollectionCleared;
                coll.CollectionChanged -= OnCollectionChanged;
            }

            if (args.NewValue != null)
            {
                ObservableNotifiableCollection<Line> coll = args.NewValue as ObservableNotifiableCollection<Line>;
                coll.CollectionCleared += OnCollectionCleared;
                coll.CollectionChanged += OnCollectionChanged;

                CreateVisualChildren(coll);
            }
        }

        /*if collection is emptied, remove all visual children*/
        void OnCollectionCleared(object sender, EventArgs args)
        {
            RemoveVisualChildren(visualChildren);
        }

        /*handles when a member is added or removed from collection*/
        void OnCollectionChanged(object sender, NotifyCollectionChangedEventArgs args)
        {
            if (args.OldItems != null)
                RemoveVisualChildren(args.OldItems);

            if (args.NewItems != null)
                CreateVisualChildren(args.NewItems);
        }

        void CreateVisualChildren(ICollection coll)
        {
            foreach (object obj in coll)
            {
                Line line = obj as Line;

                DrawingVisualPlus drawingVisual = new DrawingVisualPlus();
                drawingVisual.Line = line;
                DrawingContext dc = drawingVisual.RenderOpen();

                PathGeometry geometry = new PathGeometry();
                PathFigure figure = new PathFigure();

                geometry.Figures.Add(figure);

                double dist = GetDistance(line.Start.X, line.End.X, line.Start.Y, line.End.Y);

                double scalar = 1;
                if (dist > MaxRange)
                    scalar = dist / MaxRange;

                figure.StartPoint = new System.Windows.Point(
                    (RenderSize.Width / 2) + (RenderSize.Width / 2) * line.Start.X/MaxRange,
                    (RenderSize.Height) - (RenderSize.Height) * line.Start.Y/MaxRange);

                figure.Segments.Add(new LineSegment(new System.Windows.Point(
                    (RenderSize.Width / 2) + (RenderSize.Width / 2) * line.End.X/MaxRange,
                    (RenderSize.Height) - (RenderSize.Height) * line.End.Y/MaxRange), true));

                dc.DrawGeometry(new SolidColorBrush(Color.FromArgb(255, 69, 255, 0)), new Pen(new SolidColorBrush(Color.FromArgb(255, 69, 255, 0)), 3), geometry);

                dc.Close();
                visualChildren.Add(drawingVisual);
            }
        }

        private double GetDistance(double x1, double x2, double y1, double y2)
        {
            return Math.Sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
        }

        /*finds the matching visualchild for removed datapoint and removes it*/
        void RemoveVisualChildren(ICollection coll)
        {
            foreach (object obj in coll)
            {
                Line line = obj as Line;
                List<DrawingVisualPlus> removeList = new List<DrawingVisualPlus>();

                foreach (Visual child in visualChildren)
                {
                    DrawingVisualPlus drawingVisual = child as DrawingVisualPlus;
                    if (drawingVisual.Line == line)
                    {
                        removeList.Add(drawingVisual);
                        break;
                    }
                }
                foreach (DrawingVisualPlus drawingVisual in removeList)
                    visualChildren.Remove(drawingVisual);
            }
        }

        /*required*/
        protected override int VisualChildrenCount
        {
            get
            {
                return visualChildren.Count;
            }
        }
        /*required*/
        protected override Visual GetVisualChild(int index)
        {
            if (index < 0 || index >= visualChildren.Count)
                throw new ArgumentOutOfRangeException("index");

            return visualChildren[index];
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
            //drawingContext.DrawRectangle(new SolidColorBrush(Color.FromArgb(255, 55, 55, 55)), null, new Rect(RenderSize));

            PathGeometry geometry = new PathGeometry();
            PathFigure figure = new PathFigure();

            geometry.Figures.Add(figure);
            figure.StartPoint = new System.Windows.Point(RenderSize.Width/2, RenderSize.Height);

            //first, draw a line from center outwards
            figure.Segments.Add(new LineSegment(new System.Windows.Point(RenderSize.Width, RenderSize.Height), true));
            figure.Segments.Add(new ArcSegment(new System.Windows.Point(0, RenderSize.Height), new Size(RenderSize.Width / 2, RenderSize.Height), 180, false, SweepDirection.Counterclockwise, true));
            figure.Segments.Add(new LineSegment(new System.Windows.Point(RenderSize.Width / 2, RenderSize.Height), true));
            drawingContext.DrawGeometry(new SolidColorBrush(Color.FromArgb(255, 0, 0, 0)), new Pen(new SolidColorBrush(Color.FromArgb(255, 200, 200, 200)), 2), geometry);
        }


        class DrawingVisualPlus : DrawingVisual
        {
            public DrawingVisualPlus() { }

            public DrawingVisualPlus(Models.Line line)
            {
                Line = line;
            }

            public Line Line { get; set; }
        }
    }
}
