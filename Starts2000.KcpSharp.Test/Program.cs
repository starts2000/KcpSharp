using System;

namespace Starts2000.KcpSharp.Test
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("enter 'X' exit test, enter 'N' use Native KCP test, enter 'W' use KCP dotnet wrapper test.");
            Console.WriteLine("please enter key:");
            var key = Console.ReadKey().Key;
            while (key != ConsoleKey.X)
            {
                if (key == ConsoleKey.N)
                {
                    Console.WriteLine();
                    NativeKcpTestWrapper test = new NativeKcpTestWrapper(10, 60, 125, 1000);
                    test.Test(0);
                    test.Test(1);
                    test.Test(2);
                    test.Dispose();
                }
                else if(key == ConsoleKey.W)
                {
                    Console.WriteLine();
                    KcpTest test = new KcpTest(10, 60, 125, 1000);
                    test.Test(0);
                    test.Test(1);
                    test.Test(2);
                    test.Dispose();
                }

                key = Console.ReadKey().Key;
            }
        }
    }
}
