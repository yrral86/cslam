using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using lunabotics.Comms.CommandEncoding;

namespace lunabotics.Comms.CommandEncoding
{
    public class FieldValuePair
    {
        public FieldValuePair()
        {
        }

        public FieldValuePair(CommandFields f, short val)
        {
            if (val < -2048 || val > 2047)
                throw new ArgumentException("Value is out of allowable range (-2048:2047)");
            this._field = f;
            this._value = val;
        }

        private CommandFields _field;
        public CommandFields field
        {
            get { return _field; }
            set { _field = value; }
        }

        private short _value;
        public short value
        {
            get { return _value; }
            set
            {
                if (value < -2048 || value > 2047)
                    throw new ArgumentException("Value is out of allowable range (-2048:2047)");
                _value = value;
            }
        }


        public override bool Equals(object obj)
        {
            var other = obj as FieldValuePair;

            if (other == null)
                return false;

            if (other._field != this._field)
                return false;

            if (other._value != this._value)
                return false;

            return true;
        }

        public override int GetHashCode()
        {
            //works fine if class is used properly (val is between -2048:2047)
            return ((short)this._field | this._value);
        }

        public override string ToString()
        {
            return this._field.ToString() + " " + this._value.ToString();
        }
    }

    // todo : add encoding/decode for multiple items

    public static class Encoder
    {
        /// <summary>
        /// Encodes a field and accompanying value into a short (16-bit) value
        /// </summary>
        /// <remarks>
        /// Bits 12-15 represent the field. Bits 0-11 represent the value (range between -2048 to 2047)
        /// </remarks>
        /// <param name="field"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public static short Encode(CommandFields field, short value)
        {
            if (value < -2048 || value > 2047)
                throw new ArgumentException("Value is outside allowable range (-2048:2047)");

            if (value < 0) //sign bit is one (0x0800)
            {
                return (short)((short)field | 0x0800 | (0x07FF & value));
            }
            else //sign bit (bit 11) is zero
            {
                //should be able to simply OR together
                return (short)((short)field | value);
            }
        }

        public static FieldValuePair Decode(short fv)
        {
            FieldValuePair toReturn = new FieldValuePair();
            toReturn.field = (CommandFields)(fv & 0xF000);

            short temp_val = 0x0000;
            //copy sign bit to positions 11-15
            if ((fv & 0x0800) == 0x0800) //has sign bit!
            {
                temp_val = (short)(temp_val | 0xF800);
            }

            //copy rest of the bits
            temp_val |= (short)(fv & 0x07FF);

            toReturn.value = temp_val;

            return toReturn;
        }

        public static Dictionary<CommandFields, short> Decode(byte[] bytes)
        {
            if (bytes == null)
                throw new ArgumentNullException("bytes cannot be null");
            if (bytes.Length % 2 > 0)
                throw new ArgumentException("byte array must have even number of bytes");

            var toReturn = new Dictionary<CommandFields, short>();

            for (int i = 0; i < bytes.Length; i += 2)
            {
                FieldValuePair fv = Decode(BitConverter.ToInt16(bytes, i));
                toReturn.Add(fv.field, fv.value);
            }

            return toReturn;
        }

        public static byte[] Encode(Dictionary<CommandFields, short> fvDict)
        {
            byte[] toReturn = new byte[fvDict.Count * 2];

            int index = 0;
            foreach (CommandFields key in fvDict.Keys)
            {
                short temp = Encode(key, fvDict[key]);
                Array.Copy(BitConverter.GetBytes(temp), 0, toReturn, index, 2);
                index += 2;
            }

            return toReturn;
        }

        public static byte[] Encode(IEnumerable<FieldValuePair> pairs)
        {
            byte[] toReturn = new byte[pairs.Count()*2];
            int index = 0;
            foreach (FieldValuePair pair in pairs)
            {
                short temp = Encode(pair.field, pair.value);
                Array.Copy(BitConverter.GetBytes(temp), 0, toReturn, index, 2);
                index += 2;
            }

            return toReturn;
        }
    }
}
