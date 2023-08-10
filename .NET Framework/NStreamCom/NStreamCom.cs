using System;
using System.Collections;
using System.Runtime.InteropServices;

namespace NStreamCom
{
    public static class Extensions
    {
        public static byte[][] MessagePacketsBytes(this Packet[] PacketArray)
        {
            byte[][] StreamBytesMessage = new byte[PacketArray.Length][];
            for (int i = 0; i < StreamBytesMessage.Length; i++) StreamBytesMessage[i] = PacketArray[i].GetStreamBytes();
            return StreamBytesMessage;
        }

        public static bool Verify(this Packet[] PacketArray)
        {
            ushort TemporaryID = PacketArray.GetMessageID();
            ushort TemporaryMessageSize = PacketArray.GetMessageSize();
            ushort PacketDataSizeTotal = 0;

            for (ushort iPacket = 0; iPacket < PacketArray.Length; iPacket++)
            {
                if (PacketArray.GetMessageID() != TemporaryID)
                    throw new PacketException("Not all IDs match in the packet array", PacketException.ErrorCodes.ID_Mismatch);
                if (PacketArray.GetMessageSize() != TemporaryMessageSize)
                    throw new PacketException("Not all MessageSizes' match in the packet array", PacketException.ErrorCodes.MessageSize_Mismatch);
                if (PacketArray[iPacket].Data.Length != PacketArray[iPacket].PacketDataSize)
                    throw new PacketException($"Packet at index { iPacket } data buffer size did not match.", PacketException.ErrorCodes.PacketDataSizeMismatch);
                PacketDataSizeTotal += PacketArray[iPacket].PacketDataSize;
            }

            if (PacketDataSizeTotal != TemporaryMessageSize)
                throw new PacketException("Did not receive all packets.", PacketException.ErrorCodes.PacketDataSizeTotal_Mismatch);

            return true;
        }

        public static void GetMessageBytes(this Packet[] Packets, ref byte[] Buffer)
        {
            for (int iPacket = 0; iPacket < Packets.Length; iPacket++)
            {
                Array.Copy(Packets[iPacket].Data, 0, Buffer, iPacket * Packets[0].PacketDataSize, Packets[iPacket].PacketDataSize);
            }
        }

        public static byte[] GetMessageBytes(this Packet[] Packets)
        {
            byte[] Buffer = new byte[Packets.GetMessageSize()];
            GetMessageBytes(Packets, ref Buffer);
            return Buffer;
        }

        public static ushort GetMessageID(this Packet[] Packets)
        {
            return Packets[0].MessageID;
        }

        public static ushort GetMessageSize(this Packet[] Packets)
        {
            return Packets[0].MessageSize;
        }

        public static void GetPacketsFromMessagePacketsBytes(ref byte[][] MessagePacketsBytes, ref Packet[] Packets)
        {
            if (MessagePacketsBytes.Length > Packets.Length) Array.Resize(ref Packets, MessagePacketsBytes.Length);
            for (int iPacket = 0; iPacket < MessagePacketsBytes.Length; iPacket++) Packets[iPacket] = new Packet(MessagePacketsBytes[iPacket]);
        }

        public static Packet[] GetPacketsFromMessagePacketsBytes(this byte[][] MessagePacketsBytes)
        {
            Packet[] Packets = new Packet[MessagePacketsBytes.Length];
            GetPacketsFromMessagePacketsBytes(ref MessagePacketsBytes, ref Packets);
            return Packets;
        }

        public static void GetBytes<T>(this T Structure, ref byte[] Out)
        {
            IntPtr Pointer = IntPtr.Zero;
            Pointer = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(T)));
            Marshal.StructureToPtr(Structure, Pointer, true);
            Marshal.Copy(Pointer, Out, 0, Marshal.SizeOf(typeof(T)));
            Marshal.FreeHGlobal(Pointer);
        }   

        public static byte[] GetBytes<T>(this T Structure) where T: struct
        {
            byte[] Buffer = new byte[Marshal.SizeOf(typeof(T))];
            GetBytes<T>(Structure, ref Buffer);
            return Buffer;
        }

        public static void GetStructure<T>(this byte[] Buffer, ref T Structure) where T: struct
        {
            IntPtr Pointer = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(T)));
            Marshal.Copy(Buffer, 0, Pointer, Marshal.SizeOf(typeof(T)));
            Structure = (T)Marshal.PtrToStructure(Pointer, typeof(T));
        }

        public static T GetStructure<T>(this byte[] Buffer) where T : struct
        {
            T Structure = new T();
            GetStructure<T>(Buffer, ref Structure);
            return Structure;
        }
    }

    public class PacketException : Exception
    {
        public enum ErrorCodes
        {
            NA,
            ID_Mismatch,
            MessageSize_Mismatch,
            PacketDataSizeTotal_Mismatch,
            PacketDataSizeMismatch,
            BadStream
        }

        public ErrorCodes Code { get; }

        public PacketException()
            : base()
        {
            this.Code = ErrorCodes.NA;
        }

        public PacketException(string message, ErrorCodes Code = ErrorCodes.NA)
            : base(message)
        {
            this.Code = Code;
        }

        public PacketException(string message, Exception inner, ErrorCodes Code = ErrorCodes.NA)
            : base(message, inner)
        {
            this.Code = Code;
        }
    }

    public class PacketCollectorException : Exception
    {
        public PacketCollectorException()
            : base()
        {
        }

        public PacketCollectorException(string Message)
            : base(Message)
        {
        }

        public PacketCollectorException(string Message, Exception Inner)
            : base(Message, Inner)
        {
        }
    }

    public class PacketsReadyEventArgs : EventArgs, IDisposable
    {
        public Packet[] Packets { get; }

        public PacketsReadyEventArgs(Packet[] Packets)
        {
            this.Packets = Packets;
        }

        public void Dispose()
        {
        }
    }

    public class PacketCollector : IDisposable
    {
        System.Collections.Generic.List<Packet> PacketsCollected = null;
        ushort PacketDataBytesCollected;
        bool DiscardOnPacketException;

        public event EventHandler<PacketsReadyEventArgs> PacketsReady;

        public PacketCollector(bool DiscardOnPacketException = true)
        {
            PacketsCollected = new System.Collections.Generic.List<Packet>();
            this.DiscardOnPacketException = DiscardOnPacketException;
        }

        public void Collect(byte[] PacketBytes)
        {
            bool Recycle = false;
            try
            {
                Packet Collected = new Packet(PacketBytes);

                PacketDataBytesCollected += Collected.PacketDataSize;

                if (PacketsCollected.Count == 0)
                {
                    PacketsCollected.Add(Collected);
                    if (Collected.MessageSize == Collected.PacketDataSize) goto Ready;
                    return;
                }

                if (Collected.MessageID == PacketsCollected[PacketsCollected.Count - 1].MessageID)
                {
                    PacketsCollected.Add(Collected);
                    if (PacketsCollected[0].MessageSize == PacketDataBytesCollected) goto Ready;
                }
                else
                {
                    if (PacketDataBytesCollected != PacketsCollected[0].MessageSize)
                    {
                        PacketsCollected.Clear();
                        PacketDataBytesCollected = 0;
                        throw new PacketCollectorException($"Did not receive all packets of Message");
                    }
                    Recycle = true;
                    goto Ready;
                }

                return;
            }
            catch (PacketException E)
            {
                if (DiscardOnPacketException) goto Clear;
                else throw;
            }
            
        Ready:
            if (PacketsReady != null) PacketsReady(this, new PacketsReadyEventArgs(PacketsCollected.ToArray()));
        Clear:
            PacketsCollected.Clear();
            PacketDataBytesCollected = 0;
            if (Recycle) Collect(PacketBytes);
        }

        public void Dispose()
        {
            PacketsCollected.Clear();
        }
    }

    public struct Packet
    {
        readonly static byte StreamProtocolSize = 6;

        public ushort MessageID { get; }

        public ushort MessageSize { get; }

        public ushort PacketDataSize { get; }

        public byte[] Data { get; }

        public int StreamBytesArraySize { get { return Data.Length + StreamProtocolSize; } }

        public Packet(ushort MessageID, ushort MessageSize, ushort PacketDataSize, ref byte[] Data)
        {
            this.MessageID = MessageID;
            this.MessageSize = MessageSize;
            this.PacketDataSize = PacketDataSize;
            this.Data = Data;
        }

        public Packet(ushort MessageID, ushort MessageSize, ushort PacketDataSize, byte[] Data)
            : this(MessageID, MessageSize, PacketDataSize, ref Data)
        {
        }

        public Packet(ref byte[] Buffer)
        {
            if (!VerifyStream(Buffer)) throw new PacketException("Parameter: Buffer, The buffer is invalid", PacketException.ErrorCodes.BadStream);
            MessageID = BitConverter.ToUInt16(Buffer, 0);
            MessageSize = BitConverter.ToUInt16(Buffer, 2);
            PacketDataSize = BitConverter.ToUInt16(Buffer, 4);
            Data = new byte[PacketDataSize];
            Array.Copy(Buffer, 6, Data, 0, PacketDataSize);
        }

        public Packet(byte[] Buffer)
            : this(ref Buffer)
        {
        }

        public Packet(Message Message)
            : this(Message.MessageID, Message.MessageSize, Message.MessageSize, Message.Data)
        {
        }

        public static bool VerifyStream(byte[] Buffer)
        {
            if (Buffer == null) return false;
            if (Buffer.Length <= StreamProtocolSize) return false;
            if (Buffer.Length - StreamProtocolSize != BitConverter.ToUInt16(Buffer, 4)) return false;
            return true;
        }

        public void GetStreamBytes(ref byte[] Buffer)
        {
            byte[] IDBytes = BitConverter.GetBytes(MessageID);
            byte[] MessageSizeBytes = BitConverter.GetBytes(MessageSize);
            byte[] PacketDataSizeBytes = BitConverter.GetBytes(PacketDataSize);
            Array.Copy(IDBytes, 0, Buffer, 0, 2);
            Array.Copy(MessageSizeBytes, 0, Buffer, 2, 2);
            Array.Copy(PacketDataSizeBytes, 0, Buffer, 4, 2);
            Array.Copy(Data, 0, Buffer, 6, PacketDataSize);
        }

        public byte[] GetStreamBytes()
        {
            byte[] Buffer = new byte[StreamBytesArraySize];
            GetStreamBytes(ref Buffer);
            return Buffer;
        }
    }

    public struct Message
    {
        public ushort MessageID { get; }
        public ushort MessageSize { get; }
        public byte[] Data { get; }

        public Message(ushort MessageID, ref byte[] Data)
        {
            this.MessageID = MessageID;
            this.MessageSize = (ushort)Data.Length;
            this.Data = Data;
        }

        public Message(ushort MessageID, byte[] Data)
            : this(MessageID, ref Data)
        {
        }

        public Message(ref Packet[] Packets)
        {
            Packets.Verify();
            MessageID = Packets.GetMessageID();
            MessageSize = Packets.GetMessageSize();
            Data = new byte[Packets.GetMessageSize()];
            Data = Packets.GetMessageBytes();
        }

        public Message(Packet[] Packets)
            : this(ref Packets)
        {
        }

        public Packet[] GetPackets(ushort PacketSize)
        {
            if (PacketSize >= MessageSize) return new Packet[] { new Packet(this) };
            int PacketCount = (int)Math.Ceiling((double)MessageSize / PacketSize);
            Packet[] Packets = new Packet[PacketCount];

            for (int iPacket = 0; iPacket < PacketCount; iPacket++)
            {
                PacketSize = (ushort)((iPacket == PacketCount - 1) ? MessageSize - (iPacket * PacketSize) : PacketSize);
                byte[] PacketBytes = new byte[PacketSize];
                Array.Copy(Data, (iPacket == PacketCount - 1) ? MessageSize - PacketSize : iPacket * PacketSize, PacketBytes, 0, PacketSize);
                Packets[iPacket] = new Packet(MessageID, MessageSize, PacketSize, PacketBytes);
            }

            return Packets;
        }
    }

    namespace ClientServerModel
    {

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        struct ClientRequest
        {
            public byte RequestingMessage;
            public ushort RequestMessageID;

            public ClientRequest(bool RequestingMessage, ushort RequestMessageID)
            {
                this.RequestingMessage = (byte)(RequestingMessage ? 1 : 0);
                this.RequestMessageID = RequestMessageID;
            }
        }

        public static class Server
        {
            public const ushort InterpreterID = ushort.MaxValue;

            public static int GetRequestedID(byte[] ClientRequestBytes)
            {
                if (ClientRequestBytes.Length != Marshal.SizeOf<ClientRequest>())
                    return -1;

                ClientRequest Request = ClientRequestBytes.GetStructure<ClientRequest>();
                if (Request.RequestingMessage == 0)
                    return -2;

                return Request.RequestMessageID;
            }

            public static int GetRequestedID(Packet[] Packets)
            {
                return GetRequestedID(Packets.GetMessageBytes());
            }

            public static int GetRequestedID(Message Message)
            {
                return GetRequestedID(Message.Data);
            }
        }

        public static class Client
        {
            const ushort InterpreterID = Server.InterpreterID;

            public static Message MakeRequestMessage(ushort ID)
            {
                return new Message(InterpreterID, new ClientRequest(true, ID).GetBytes());
            }
        }
    }
}