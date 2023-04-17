using System;
using System.Text;
using System.Runtime.InteropServices;

namespace NStreamCom
{
    public struct NStreamData
    {
        public ushort ID;
        public object Data;
    
        public NStreamData(ushort newID, object newData)
        {
            ID = newID;
            Data = newData;
        }
    }

    public static class NStreamComParser
    {
        private const byte PROTOCOL_OVERHEAD = 6;
        private const byte BUFFER_SOH_IDX = 0;
        private const byte BUFFER_SIZE_IDX = 1;
        private const byte BUFFER_ID_IDX = 2;
        private const byte BUFFER_DATA_IDX = 4;

        private const byte ASCII_NULL = 0x00;
        private const byte ASCII_SOH = 0x01;
        private const byte ASCII_NEWLINE = 0x0A;
        private const byte ASCII_RETURN = 0x0D;

        private static void CombineBytes(ref byte[] Buffer, byte Size, ushort ID, byte[] DataBytes)
        {
            Buffer[0] = ASCII_SOH;
            Buffer[BUFFER_SIZE_IDX] = Size;

            byte[] IDBytes = BitConverter.GetBytes(ID);
            Buffer[BUFFER_ID_IDX] = IDBytes[0];
            Buffer[BUFFER_ID_IDX + 1] = IDBytes[1];

            Array.Copy(DataBytes, 0, Buffer, BUFFER_DATA_IDX, DataBytes.Length);

            Buffer[Buffer.Length - 2] = ASCII_RETURN;
            Buffer[Buffer.Length - 1] = ASCII_NEWLINE;
        }

        public static NStreamData? Parse(byte[] Buffer)
        {
            if (Buffer.Length <= PROTOCOL_OVERHEAD) return null;
            if ((Buffer[BUFFER_SOH_IDX] != ASCII_SOH) || (Buffer.Length - Buffer[BUFFER_SIZE_IDX] != PROTOCOL_OVERHEAD) || (Buffer[Buffer.Length - 2] != ASCII_RETURN) || (Buffer[Buffer.Length - 1] != ASCII_NEWLINE)) return null;
            byte Size = Buffer[BUFFER_SIZE_IDX];
            ushort ID = BitConverter.ToUInt16(Buffer, BUFFER_ID_IDX);
            byte[] DataBytes = new byte[Size];
            Array.Copy(Buffer, BUFFER_DATA_IDX, DataBytes, 0, Size);
            return new NStreamData(ID, DataBytes);
        }

        public static byte[] Parse(ushort ID, byte[] Buffer)
        {
            byte[] ParsedBuffer = new byte[Buffer.Length + PROTOCOL_OVERHEAD];
            CombineBytes(ref ParsedBuffer, ((byte)Buffer.Length), ID, Buffer);
            return ParsedBuffer;
        }

        public static byte[] Parse(ushort ID, uint Data)
        {
            byte[] Buffer = new byte[sizeof(uint) + PROTOCOL_OVERHEAD];
            byte[] DataBytes = BitConverter.GetBytes(Data);
            CombineBytes(ref Buffer, sizeof(uint), ID, DataBytes);
            return Buffer;
        }

        public static byte[] Parse(ushort ID, int Data)
        {
            byte[] Buffer = new byte[sizeof(int) + PROTOCOL_OVERHEAD];
            byte[] DataBytes = BitConverter.GetBytes(Data);
            CombineBytes(ref Buffer, sizeof(int), ID, DataBytes);
            return Buffer;
        }

        public static byte[] Parse(ushort ID, double Data)
        {
            byte[] Buffer = new byte[sizeof(double) + PROTOCOL_OVERHEAD];
            byte[] DataBytes = BitConverter.GetBytes(Data);
            CombineBytes(ref Buffer, sizeof(double), ID, DataBytes);
            return Buffer;
        }

        public static byte[] Parse(ushort ID, float Data)
        {
            byte[] Buffer = new byte[sizeof(float) + PROTOCOL_OVERHEAD];
            byte[] DataBytes = BitConverter.GetBytes(Data);
            CombineBytes(ref Buffer, sizeof(float), ID, DataBytes);
            return Buffer;
        }

        public static byte[] Parse(ushort ID, string Data)
        {
            byte[] Buffer = new byte[PROTOCOL_OVERHEAD + Data.Length + 1];
            byte[] DataBytes = new byte[Data.Length + 1];
            byte[] UnsafeDataBytes = Encoding.ASCII.GetBytes(Data);
            Array.Copy(UnsafeDataBytes, DataBytes, UnsafeDataBytes.Length);
            CombineBytes(ref Buffer, ((byte)DataBytes.Length), ID, DataBytes);
            return Buffer;
        }

        public static byte[] Parse<T>(ushort ID, T Structure) where T : struct
        {
            byte[] DataBytes = new byte[Marshal.SizeOf(typeof(T))];
            IntPtr Pointer = IntPtr.Zero;

            Pointer = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(T)));
            Marshal.StructureToPtr(Structure, Pointer, true);
            Marshal.Copy(Pointer, DataBytes, 0, Marshal.SizeOf(typeof(T)));
            Marshal.FreeHGlobal(Pointer);

            return Parse(ID, DataBytes);
        }

        public static byte[] Parse(NStreamData Data)
        {
            if (Data.Data is uint)
            {
                return Parse(Data.ID, (uint)Data.Data);
            }
            else if (Data.Data is int)
            {
                return Parse(Data.ID, (int)Data.Data);
            }
            else if (Data.Data is double)
            {
                return Parse(Data.ID, (double)Data.Data);
            }
            else if (Data.Data is float)
            {
                return Parse(Data.ID, (float)Data.Data);
            }
            else if (Data.Data is string)
            {
                return Parse(Data.ID, (string)Data.Data);
            }
            else
            {
                return null;
            }
        }
    }
}