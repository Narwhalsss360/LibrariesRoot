using System;
using ByteExtensions;

namespace ByteExtensionsTest
{
    internal class Program
    {
        static string GetBitsString(byte Byte)
        {
            string Output = $"[0]: {Byte.GetBit(0)}";
            for (int i = 1; i < 8; i++) Output += $" [{i}]: {Byte.GetBit(i)}";
            return Output;
        }

        static void Main(string[] args)
        {
            byte myNumber = 13;
            
            Console.WriteLine(GetBitsString(myNumber));
            myNumber.SetBit(5);
            Console.WriteLine(GetBitsString(myNumber));
            myNumber.FlipBit(3);
            Console.WriteLine(GetBitsString(myNumber));
            Console.ReadLine();
        }
    }
}