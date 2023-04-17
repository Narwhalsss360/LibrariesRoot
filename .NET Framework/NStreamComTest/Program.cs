using System;
using System.Text;
using System.IO.Ports;
using NStreamCom;
using System.Threading;

namespace NStreamComTest
{
    #region Creation
    static class CreationTestPacketArrayExtension
    {
        public static byte[][] MessagePacketsBytes(this CreationTest.Packet[] PacketArray)
        {
            byte[][] StreamBytesMessage = new byte[PacketArray.Length][];
            for (int i = 0; i < StreamBytesMessage.Length; i++) StreamBytesMessage[i] = PacketArray[i].GetStreamBytes();
            return StreamBytesMessage;
        }

        public static bool Verify(this CreationTest.Packet[] PacketArray)
        {
            ushort TemporaryID = PacketArray[0].ID;
            ushort TemporaryMessageSize = PacketArray[0].MessageSize;
            ushort PacketDataSizeTotal = 0;

            for (ushort iPacket = 0; iPacket < PacketArray.Length; iPacket++) if (PacketArray[iPacket].ID != TemporaryID) throw new CreationTestPacketException("Not all IDs match in the packet array", CreationTestPacketException.ErrorCodes.ID_Mismatch);
            for (ushort iPacket = 0; iPacket < PacketArray.Length; iPacket++) if (PacketArray[iPacket].MessageSize != TemporaryMessageSize) throw new CreationTestPacketException("Not all MessageSizes' match in the packet array", CreationTestPacketException.ErrorCodes.MessageSize_Mismatch);
            for (ushort iPacket = 0; iPacket < PacketArray.Length; iPacket++) PacketDataSizeTotal += PacketArray[iPacket].PacketDataSize;
            if (PacketDataSizeTotal != TemporaryMessageSize) throw new CreationTestPacketException("Did not receive all packets.", CreationTestPacketException.ErrorCodes.PacketDataSizeTotal_Mismatch);

            return true;
        }

        public static void GetMessageBytes(this CreationTest.Packet[] Packets, ref byte[] Buffer)
        {
            for (int iPacket = 0; iPacket < Packets.Length; iPacket++) Array.Copy(Packets[iPacket].Data, 0, Buffer, iPacket * Packets[0].PacketDataSize, Packets[0].PacketDataSize);
        }

        public static byte[] GetMessageBytes(this CreationTest.Packet[] Packets)
        {
            byte[] Buffer = new byte[Packets[0].MessageSize];
            GetMessageBytes(Packets, ref Buffer);
            return Buffer;
        }
    }

    public class CreationTestPacketException : Exception
    {
        public enum ErrorCodes
        {
            NA,
            ID_Mismatch,
            MessageSize_Mismatch,
            PacketDataSizeTotal_Mismatch
        }

        public ErrorCodes Code { get; }

        public CreationTestPacketException()
        {
            this.Code = ErrorCodes.NA;
        }

        public CreationTestPacketException(string message, ErrorCodes Code = ErrorCodes.NA)
            : base(message)
        {
            this.Code = Code;
        }

        public CreationTestPacketException(string message, Exception inner, ErrorCodes Code = ErrorCodes.NA)
            : base(message, inner)
        {
            this.Code = Code;
        }
    }

    public static class CreationTest
    {
        public struct Packet
        {
            readonly static int StreamSizeNoData = 6;

            public ushort ID;
            public ushort MessageSize { get; }
            public ushort PacketDataSize { get; }
            public byte[] Data { get; }

            public Packet(ushort ID, ushort MessageSize, ushort PacketSize, byte[] Data)
            {
                this.ID = ID;
                this.MessageSize = MessageSize;
                this.PacketDataSize = PacketSize;
                this.Data = Data;
            }

            public Packet(ushort ID, ushort MessageSize, ushort PacketSize, ref byte[] Data)
            {
                this.ID = ID;
                this.MessageSize = MessageSize;
                this.PacketDataSize = PacketSize;
                this.Data = Data;
            }

            public Packet(Message M)
            {
                this.ID = M.ID;
                this.MessageSize = M.Size;
                this.PacketDataSize = M.Size;
                this.Data = M.Data;
            }

            public Packet(ref Message M)
            {
                this.ID = M.ID;
                this.MessageSize = M.Size;
                this.PacketDataSize = M.Size;
                this.Data = M.Data;
            }

            public Packet(ref byte[] Bytes)
            {
                ID = BitConverter.ToUInt16(Bytes, 0);
                MessageSize = BitConverter.ToUInt16(Bytes, 2);
                PacketDataSize = BitConverter.ToUInt16(Bytes, 4);
                Data = new byte[PacketDataSize];
                Array.Copy(Bytes, 6, Data, 0, PacketDataSize);
            }

            public Packet(byte[] Bytes)
                : this(ref Bytes)
            {
            }

            public int GetStreamBytesArraySize()
            {
                return StreamSizeNoData + PacketDataSize;
            }

            public void GetStreamBytes(ref byte[] Bytes)
            {
                byte[] IDBytes = BitConverter.GetBytes(ID);
                byte[] MessageSizeBytes = BitConverter.GetBytes(MessageSize);
                byte[] PacketSizeBytes = BitConverter.GetBytes(PacketDataSize);
                Array.Copy(IDBytes, 0, Bytes, 0, 2);
                Array.Copy(MessageSizeBytes, 0, Bytes, 2, 2);
                Array.Copy(PacketSizeBytes, 0, Bytes, 4, 2);
                Array.Copy(Data, 0, Bytes, 6, PacketDataSize);
            }

            public byte[] GetStreamBytes()
            {
                byte[] Buffer = new byte[GetStreamBytesArraySize()];
                GetStreamBytes(ref Buffer);
                return Buffer;
            }
        }

        public struct Message
        {
            public ushort ID;
            public byte[] Data;
            public ushort Size;

            public Message(ushort ID, byte[] Data)
            {
                this.ID = ID;
                this.Data = Data;
                Size = (ushort)Data.Length;
            }

            public Message(ushort ID, ref byte[] Data)
            {
                this.ID = ID;
                this.Data = Data;
                Size = (ushort)Data.Length;
            }

            public Message(Packet[] Packets)
            {
                Packets.Verify();
                ID = Packets[0].ID;
                Size = Packets[0].MessageSize;
                Data = Packets.GetMessageBytes();
            }

            public Message(ref byte[][] MessagePacketsBytes)
            {
                Packet[] Packets = new Packet[MessagePacketsBytes.Length];
                for (int iPacket = 0; iPacket < MessagePacketsBytes.Length; iPacket++) Packets[iPacket] = new Packet(MessagePacketsBytes[iPacket]);
                this = new Message(Packets);
            }

            public Message(byte[][] MessagePacketsBytes)
                : this(ref MessagePacketsBytes)
            {

            }

            public Packet[] GetPackets(ushort PacketSize)
            {
                if (PacketSize >= Size) return new Packet[] { new Packet(this) };
                int PacketCount = (int)Math.Ceiling((double)Size / PacketSize);
                Packet[] Packets = new Packet[PacketCount];

                for (int iPacket = 0; iPacket < PacketCount; iPacket++)
                {
                    byte[] PacketBytes = new byte[PacketSize];
                    Array.Copy(Data, iPacket * PacketSize, PacketBytes, 0, PacketSize);
                    Packets[iPacket] = new Packet(ID, Size, PacketSize, PacketBytes);
                }
                return Packets;
            }
        }

        public static void Main(string[] args)
        {
            byte[] A1 = new byte[16];
            for (int i = 0; i < 16; i++) A1[i] = (byte)i;
            Message M1 = new Message(255, A1);
            Packet[] PA1 = M1.GetPackets(4);
            byte[][] MessagePacketsBytes = PA1.MessagePacketsBytes();
            Packet p1 = new Packet(MessagePacketsBytes[0]);
            Message recv = new Message(MessagePacketsBytes);
        }
    }
    #endregion

    interface ITester
    {
        String TestName();
        bool Test();
        bool TestResult();
    }

    public static class ProgramExtensions
    {
        public static string ItemizedString<T>(this T[] Items)
        {
            string Out = "[";

            for (int Index = 0; Index < Items.Length; Index++)
            {
                Out += Items[Index].ToString();
                if (Index != Items.Length - 1) Out += ", ";
            }
            Out += "]";
            return Out;
        }
    }

    public class SerializeDeserializeTest : ITester
    {
        public static bool Pass = false;

        struct DataContainer
        {
            public double x;
            public double y;

            public static bool Matches(ref DataContainer One, ref DataContainer Other) => (One.x == Other.x && One.y == Other.y);
        }

        public String TestName() => "Serialize Deserialize";

        public bool Test()
        {
            DataContainer Data = new DataContainer();
            Data.x = 2;
            Data.y = 4;
            ushort Data1ID = 1;

            Message Message = new Message(Data1ID, Data.GetBytes());
            Packet[] MPackets = Message.GetPackets(4);
            byte[][] MPacketsBytes = MPackets.MessagePacketsBytes();
            
            Packet[] MRPackets = MPacketsBytes.GetPacketsFromMessagePacketsBytes();
            Message MessageR = new Message(ref MPackets);
            DataContainer Data1R = MessageR.Data.GetStructure<DataContainer>();

            Pass = DataContainer.Matches(ref Data, ref Data1R);
            return Pass;
        }

        public bool TestResult() => Pass;
    }

    public class SerialPortTest : ITester
    {
        /*
         * Expects an echo from reciever 2 seconds later.
         */
        bool Pass = false;
        int Read = 0;
        byte[][] Data = new byte[4][];

        const string TextMessage = "This is the text message to be sent through the serial port";
        SerialPort Port = new SerialPort("COM15", 1000000);

        public String TestName() => "Serial Port";

        void WriteBytePackets(ref byte[][] Packets, int delay = 4)
        {
            Console.WriteLine($"[Serial Port Test] Writing...");
            bool first = true;
            foreach (byte[] Buffer in Packets)
            {
                Port.DiscardOutBuffer();
                if (!first) System.Threading.Thread.Sleep(delay);
                else first = false;
                Port.Write(Buffer, 0, Buffer.Length);
            }
            System.Threading.Thread.Sleep(delay);
        }

        void WriteBytePackets(byte[][] Packets, int delay = 4)
        {
            WriteBytePackets(ref Packets, delay);
        }

        public bool Test()
        {
            Port.DataReceived += Port_DataReceived;
            Port.Open();
            Port.DiscardInBuffer();
            Port.DiscardOutBuffer();

            Message PortMessage = new Message(1, Encoding.ASCII.GetBytes(TextMessage));
            Packet[] Packets = PortMessage.GetPackets(16);
            WriteBytePackets(Packets.MessagePacketsBytes(), 12);
            Port.DiscardInBuffer();

            DateTime Start = DateTime.Now;
            while (Read < 4)
            {
                if ((DateTime.Now - Start).TotalSeconds > 30)
                {
                    Console.WriteLine("[Serial Port Test] Timed out.");
                    goto Fail;
                }
            }
            Port.DataReceived -= Port_DataReceived;

            Message Recieved = new Message(Data.GetPacketsFromMessagePacketsBytes());
            string RecievedString = Encoding.ASCII.GetString(Recieved.Data);

            Pass = RecievedString == TextMessage;
            Fail:
            Port.Close();
            Port.Dispose();
            return Pass;
        }

        private void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            Console.WriteLine($"[Serial Port Test] Read: { Read }");
            SerialPort EventPort = (SerialPort)sender;
            byte[] Buffer = new byte[EventPort.BytesToRead];
            EventPort.Read(Buffer, 0, Buffer.Length);
            EventPort.DiscardInBuffer();
            Data[Read] = Buffer;
            Read++;
        }

        public bool TestResult() => Pass;
    }

    public class VerificationTest : ITester
    {
        bool Pass = false;

        public String TestName() => "Verification Test";

        public bool Test()
        {
            Packet[] Packets = new Packet[] { new Packet(1, 0, 0, new byte[] { 1 }), new Packet(1, 0, 0, new byte[] { 1 }) };

            try
            {
                Packets.Verify();
            }
            catch (PacketException E)
            {
                Pass = (E.Code == PacketException.ErrorCodes.ID_Mismatch);
            }

            Packets[0] = new Packet(1, 5, 3, new byte[] { 0, 0, 0});
            Packets[1] = new Packet(1, 5, 2, new byte[] { 0, 0 });

            try
            {
                Packets.Verify();
            }
            catch (PacketException E)
            {
                Pass = E.Code == PacketException.ErrorCodes.MessageSize_Mismatch;
            }

            Packets[0] = new Packet(1, 5, 3, new byte[] { 0, 0, 0 });
            Packets[1] = new Packet(1, 5, 3, new byte[] { 0, 0, 0 });

            try
            {
                Packets.Verify();
            }
            catch (PacketException E)
            {
                Pass = E.Code == PacketException.ErrorCodes.PacketDataSizeTotal_Mismatch;
            }

            Packets[0] = new Packet(1, 4, 4, new byte[] { 0, 0, 0 });
            Packets[1] = new Packet();

            try
            {
                Packets.Verify();
            }
            catch (PacketException E)
            {
                Pass = E.Code == PacketException.ErrorCodes.PacketDataSizeMismatch;
            }

            return Pass;
        }

        public bool TestResult() => Pass;
    }

    public class SerialPortReceiverTest : ITester
    {
        bool Pass;

        const int ReadTo = 90000; // milliseconds

        SerialPort Port = new SerialPort("COM15", 1000000);

        PacketCollector Collector = new PacketCollector();

        System.Timers.Timer ConsoleTimer = new System.Timers.Timer();

        bool GarbageDataIndicator = false;
        uint GarbageDataCount;

        uint ErrorCount;

        UInt32 Uptime = 1;
        UInt32 Micros;

        public String TestName() => "Serial Port Receive Test";

        public bool Test()
        {
            Port.Open();
            Thread.Sleep(600);
            
            Port.DataReceived += Port_DataReceived;
            Collector.PacketsReady += Collector_PacketsReady;
            ConsoleTimer.Elapsed += ConsoleTimer_Elapsed;
            
            ConsoleTimer.AutoReset = true;
            ConsoleTimer.Start();

            Pass = true;
            DateTime Start = DateTime.Now;
            while (Uptime < ReadTo || true)
            {
            };

            ConsoleTimer.Stop();
            Port.Close();
            Collector.Dispose();
            return Pass;
        }

        private void ConsoleTimer_Elapsed(object Sender, System.Timers.ElapsedEventArgs EventArgs)
        {
            Console.WriteLine($"[SerialPortReceiverTest] Uptime: { Uptime }, Micros: { Micros }. Garbage: { GarbageDataIndicator }, GD: { GarbageDataCount }, Errors: { ErrorCount }");
        }

        private void Collector_PacketsReady(object Sender, PacketsReadyEventArgs EventArgs)
        {
            try
            {
                Message Received = new Message(EventArgs.Packets);
                //Console.WriteLine($"[SerialPortReceiverTest] ID: { Received.MessageID }, Size: { Received.MessageSize }, Integrated: { BitConverter.ToUInt32(Received.Data, 0) }, Data: { Received.Data.ItemizedString() }");
                switch (Received.MessageID)
                {
                    case 1:
                        Uptime = BitConverter.ToUInt32(Received.Data, 0);
                        break;
                    case 2:
                        Micros = BitConverter.ToUInt32(Received.Data, 0);
                        break;
                    default:
                        break;
                }
            }
            catch (PacketException E)
            {
                Console.WriteLine($"Packet Exception: { E.Code }");
                ErrorCount++;
            }
        }

        private void Port_DataReceived(object Sender, SerialDataReceivedEventArgs EventArgs)
        {
            SerialPort EventPort = Sender as SerialPort;
            byte[] ReadInBuffer = new byte[EventPort.BytesToRead];
            EventPort.Read(ReadInBuffer, 0, ReadInBuffer.Length);
            EventPort.DiscardInBuffer();
            //Console.WriteLine($"[SerialPortReceiverTest] { ReadInBuffer.ItemizedString() }");
            try
            {
                Collector.Collect(ReadInBuffer);
                GarbageDataIndicator = false;
                if (!Packet.VerifyStream(ReadInBuffer))
                {
                    GarbageDataIndicator = true;
                    GarbageDataCount++;
                }
            }
            catch (PacketCollectorException)
            {
                GarbageDataIndicator = true;
                GarbageDataCount++;
            }
        }

        public bool TestResult() => Pass;
    }

    public static class Program
    {
        static void Main(string[] args)
        {
            ITester[] Tests = new ITester[]
            {
                new SerializeDeserializeTest(),
                new VerificationTest(),
                //new SerialPortTest(),
                new SerialPortReceiverTest()
            };

            bool FailedTest = false;

            foreach (ITester Tester in Tests)
            {
                Exception Caught = null;
                try
                {
                    Tester.Test();
                }
                catch (Exception E)
                {
                    Caught = E;
                    goto Failed;
                }

                if (!Tester.TestResult()) goto Failed;
                
                Console.WriteLine($"{Tester.TestName()} ran successfuly.");
                continue;
                
            Failed:
                Console.WriteLine($"{Tester.TestName()} failed{ (Caught != null ? $" with exception, Message:{ Caught.Message }" : ".") }");
                FailedTest = true;
            }
            
            if (!FailedTest) Console.WriteLine("All tests ran successfully. Press enter to exit.");
            else Console.WriteLine("Press enter to exit.");

            Console.ReadLine();
        }
    }
}