using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace lunabotics.OCU.Models
{
    public class ObservableNotifiableCollection<T> :
                ObservableCollection<T> where T : INotifyPropertyChanged
    {
        public ItemPropertyChangedEventHandler ItemPropertyChanged;
        public EventHandler CollectionCleared;

        protected override void OnCollectionChanged(NotifyCollectionChangedEventArgs args)
        {
            base.OnCollectionChanged(args);

            if (args.NewItems != null)
                foreach (INotifyPropertyChanged item in args.NewItems)
                    item.PropertyChanged += OnItemPropertyChanged;

            if (args.OldItems != null)
                foreach (INotifyPropertyChanged item in args.OldItems)
                    item.PropertyChanged -= OnItemPropertyChanged;
        }

        void OnItemPropertyChanged(object sender, PropertyChangedEventArgs args)
        {
            if (ItemPropertyChanged != null)
                ItemPropertyChanged(this, new ItemPropertyChangedEventArgs(sender, args.PropertyName));
        }

        protected override void ClearItems()
        {
            foreach (INotifyPropertyChanged item in Items)
                item.PropertyChanged -= OnItemPropertyChanged;

            if (CollectionCleared != null)
                CollectionCleared(this, EventArgs.Empty);

            base.ClearItems();
        }
    }

    public class ItemPropertyChangedEventArgs : PropertyChangedEventArgs
    {
        object item;

        public ItemPropertyChangedEventArgs(object item,
                                            string propertyName)
            : base(propertyName)
        {
            this.item = item;
        }

        public object Item
        {
            get { return item; }
        }
    }

    public delegate void ItemPropertyChangedEventHandler(object sender,
                                        ItemPropertyChangedEventArgs args);
}
