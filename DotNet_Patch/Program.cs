using System;
using System.Linq;
using Mono.Cecil;
using Mono.Cecil.Cil;

namespace DotNet_Patch
{
    class Program
    {
        static void Main()
        {
            Form1 form = new Form1();
            form.ShowDialog();
        }
    }
}
