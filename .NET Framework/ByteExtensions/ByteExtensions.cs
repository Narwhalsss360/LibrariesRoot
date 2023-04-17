namespace ByteExtensions
{
    public static class ByteExtensions
    {
        public static int BitMask(int Bit)
        {
            return (1 << Bit);
        }

        public static bool GetBit(this byte Byte, int Bit)
        {
            if (Bit < 0 || Bit > 7)
            {
                throw new System.ArgumentOutOfRangeException("Bit", $"Argument must be in the range 0 - 7. {Bit} was given.");
            }

            return (Byte & BitMask(Bit)) > 0;
        }

        public static void SetBit(ref this byte Byte, int Bit)
        {
            if (Bit < 0 || Bit > 7)
            {
                throw new System.ArgumentOutOfRangeException("Bit", $"Argument must be in the range 0 - 7. {Bit} was given.");
            }
            Byte |= (byte)BitMask(Bit);
        }

        public static void ClearBit(ref this byte Byte, int Bit)
        {
            if (Bit < 0 || Bit > 7)
            {
                throw new System.ArgumentOutOfRangeException("Bit", $"Argument must be in the range 0 - 7. {Bit} was given.");
            }

            Byte &= (byte)~BitMask(Bit);
        }

        public static void BitWrite(ref this byte Byte, int Value)
        {
            if (Value > 0)
            {
                SetBit(ref Byte, Value);
            }
            else
            {
                ClearBit(ref Byte, Value);
            }
        }

        public static void FlipBit(ref this byte Byte, int Bit)
        {
            if (Bit < 0 || Bit > 7)
            {
                throw new System.ArgumentOutOfRangeException("Bit", $"Argument must be in the range 0 - 7. { Bit } was given.");
            }

            Byte ^= (byte)BitMask(Bit);
        }
    }
}