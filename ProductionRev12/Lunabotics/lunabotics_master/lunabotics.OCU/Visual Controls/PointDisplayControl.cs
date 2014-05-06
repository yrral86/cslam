using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using lunabotics.OCU.Models;
using System.Collections;

namespace lunabotics.OCU.Visual_Controls
{
    public class PointDisplayControl : FrameworkElement
    {
        VisualCollection visualChildren;

        public static readonly DependencyProperty PointSourceProperty =
            DependencyProperty.Register("PointSource",
                typeof(ObservableNotifiableCollection<Models.Point>),
                typeof(PointDisplayControl),
                new PropertyMetadata(OnPointSourceChanged));

        public static readonly DependencyProperty MaxRangeProperty =
            DependencyProperty.Register("MaxRange",
            typeof(double),
            typeof(PointDisplayControl),
            new PropertyMetadata(1000.0));

        public PointDisplayControl()
            : base()
        {
            this.Height = 200; this.Width = 400;
            visualChildren = new VisualCollection(this);
        }

        public ObservableNotifiableCollection<Models.Point> PointSource
        {
            set { SetValue(PointSourceProperty, value); }
            get { return (ObservableNotifiableCollection<Models.Point>)GetValue(PointSourceProperty); }
        }

        public double MaxRange
        {
            set { SetValue(MaxRangeProperty, value); }
            get { return (double)GetValue(MaxRangeProperty); }
        }

        static void OnPointSourceChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            (obj as PointDisplayControl).OnPointSourceChanged(args);
        }

        void OnPointSourceChanged(DependencyPropertyChangedEventArgs args)
        {
            visualChildren.Clear();

            if (args.OldValue != null)
            {
                ObservableNotifiableCollection<Models.Point> coll = args.OldValue as ObservableNotifiableCollection<Models.Point>;
                coll.CollectionCleared -= OnCollectionCleared;
                coll.CollectionChanged -= OnCollectionChanged;
                coll.ItemPropertyChanged -= OnItemPropertyChanged;
            }

            if (args.NewValue != null)
            {
                ObservableNotifiableCollection<Models.Point> coll = args.NewValue as ObservableNotifiableCollection<Models.Point>;
                coll.CollectionCleared += OnCollectionCleared;
                coll.CollectionChanged += OnCollectionChanged;
                coll.ItemPropertyChanged += OnItemPropertyChanged;

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

        /*moves an existing point on the plot when it's x or y value change*/
        void OnItemPropertyChanged(object sender, ItemPropertyChangedEventArgs args)
        {
            Models.Point dataPoint = args.Item as Models.Point;

            foreach (Visual child in visualChildren)
            {
                DrawingVisualPlus drawingVisual = child as DrawingVisualPlus;

                if (dataPoint == drawingVisual.DataPoint)
                {
                    // Assume only VariableX or VariableY are changing
                    TranslateTransform xform = drawingVisual.Transform as TranslateTransform;

                    if (args.PropertyName == "X")
                        xform.X = (RenderSize.Width / 2) + (RenderSize.Width / 2) * dataPoint.X / MaxRange;

                    else if (args.PropertyName == "Y")
                        xform.Y = (RenderSize.Height) - (RenderSize.Height) * dataPoint.Y / MaxRange;
                }
            }
        }

        /*Creates DrawingVisual to place point on plot*/
        void CreateVisualChildren(ICollection coll)
        {
            foreach (object obj in coll)
            {
                Models.Point dataPoint = obj as Models.Point;

                DrawingVisualPlus drawingVisual = new DrawingVisualPlus();
                drawingVisual.DataPoint = dataPoint;
                DrawingContext dc = drawingVisual.RenderOpen();

                dc.DrawEllipse(new SolidColorBrush(Color.FromArgb(255, 127, 255, 0)), null,
                    new System.Windows.Point(0, 0), 2, 2);

                drawingVisual.Transform = new TranslateTransform(
                    ((RenderSize.Width / 2) + (RenderSize.Width / 2) * dataPoint.X / MaxRange),
                    ((RenderSize.Height) - (RenderSize.Height) * dataPoint.Y / MaxRange));

                dc.Close();
                visualChildren.Add(drawingVisual);
            }
        }

        /*finds the matching visualchild for removed datapoint and removes it*/
        void RemoveVisualChildren(ICollection coll)
        {
            foreach (object obj in coll)
            {
                Models.Point dataPoint = obj as Models.Point;
                List<DrawingVisualPlus> removeList = new List<DrawingVisualPlus>();

                foreach (Visual child in visualChildren)
                {
                    DrawingVisualPlus drawingVisual = child as DrawingVisualPlus;
                    if (drawingVisual.DataPoint == dataPoint)
                    {
                        removeList.Add(drawingVisual);
                        break;
                    }
                }
                foreach (DrawingVisualPlus drawingVisual in removeList)
                    visualChildren.Remove(drawingVisual);
            }
        }

        /*if form is resized, moves the points accordingly*/
        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            foreach (Visual child in visualChildren)
            {
                DrawingVisualPlus drawingVisual = child as DrawingVisualPlus;
                TranslateTransform xform = drawingVisual.Transform as TranslateTransform;

                if (sizeInfo.WidthChanged)
                    xform.X = (RenderSize.Width / 2) + (RenderSize.Width / 2) * drawingVisual.DataPoint.X / MaxRange;

                if (sizeInfo.HeightChanged)
                    xform.Y = (RenderSize.Width) - (RenderSize.Height) * drawingVisual.DataPoint.Y / MaxRange;
            }
            base.OnRenderSizeChanged(sizeInfo);
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
            PathGeometry geometry = new PathGeometry();
            PathFigure figure = new PathFigure();

            geometry.Figures.Add(figure);
            figure.StartPoint = new System.Windows.Point(RenderSize.Width/2, RenderSize.Height);

            //first, draw a line from center outwards
            figure.Segments.Add(new LineSegment(new System.Windows.Point(RenderSize.Width/2 + RenderSize.Width/2*Math.Cos(Math.PI/4), RenderSize.Height - RenderSize.Height*Math.Sin(Math.PI/4)), true));
            figure.Segments.Add(new ArcSegment(new System.Windows.Point(RenderSize.Width/2 + RenderSize.Width/2*Math.Cos(3*Math.PI/4), RenderSize.Height - RenderSize.Height*Math.Sin(3*Math.PI/4)), new Size(RenderSize.Width / 2, RenderSize.Height), 90, false, SweepDirection.Counterclockwise, true));
            figure.Segments.Add(new LineSegment(new System.Windows.Point(RenderSize.Width / 2, RenderSize.Height), true));
            drawingContext.DrawGeometry(new SolidColorBrush(Color.FromArgb(176, 0, 0, 0)), new Pen(new SolidColorBrush(Color.FromArgb(255, 200, 200, 200)), 2), geometry);
       
        }

        class DrawingVisualPlus : DrawingVisual
        {
            public DrawingVisualPlus() { }

            public DrawingVisualPlus(Models.Point dp) { DataPoint = dp; }

            public Models.Point DataPoint { get; set; }
        }
        
    }
}
