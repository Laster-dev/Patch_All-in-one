using Mono.Cecil;
using Mono.Cecil.Cil;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DotNet_Patch
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {

            // 设置自定义解析器
            var resolver = new DefaultAssemblyResolver();
            resolver.AddSearchDirectory(System.IO.Path.GetDirectoryName(textBox_exe.Text));
            resolver.AddSearchDirectory(System.IO.Path.GetDirectoryName(textBox_dll.Text));

            var readParams = new ReaderParameters { AssemblyResolver = resolver };

            // 加载主程序集
            var assembly = AssemblyDefinition.ReadAssembly(textBox_exe.Text, readParams);

            // 加载 DLL 并获取需要的类型和方法
            var dllAssembly = AssemblyDefinition.ReadAssembly(textBox_dll.Text, readParams);
            //MessageBox.Show($"{textBox_namespace.Text}.{textBox_classname.Text}");
            var typeC = dllAssembly.MainModule.Types.First(t => t.FullName == $"{textBox_namespace.Text}.{textBox_classname.Text}");
            var methodD = typeC.Methods.First(m => m.Name == $"{textBox_funcname.Text}" && m.IsPublic);


            // 获取主程序集的入口点
            var entryPoint = assembly.EntryPoint;
            var il = entryPoint.Body.GetILProcessor();

            // 创建实例并调用方法
            var constructor = typeC.Methods.First(m => m.IsConstructor && !m.HasParameters);
            var ctor = il.Create(OpCodes.Newobj, assembly.MainModule.ImportReference(constructor));
            var call = il.Create(OpCodes.Call, assembly.MainModule.ImportReference(methodD));

            // 修改 IL 代码，在原入口点前插入新指令
            il.InsertBefore(entryPoint.Body.Instructions.First(), ctor);
            il.InsertAfter(ctor, call);
            // 保存修改后的程序集
            assembly.Write(@"temp.exe");
            File.WriteAllBytes("ILRepack.exe", Resource1.ILRepack);
            File.WriteAllBytes("ILRepack.runtimeconfig.json",Resource1.ILRepack_runtimeconfig);

            if (checkBox1.Checked)
            {
                // 创建一个新的进程启动信息对象
                ProcessStartInfo startInfo = new ProcessStartInfo();

                // 设置要启动的应用程序的路径
                startInfo.FileName = "ILRepack.exe";

                // 设置要传递给应用程序的参数
                startInfo.Arguments = $"/out:{textBox_exe.Text}_Patch.exe temp.exe {textBox_dll.Text}";

                // 创建并启动进程
                Thread.Sleep(1000);
                Process process = new Process();
                process.StartInfo = startInfo;

                try
                {
                    process.Start();
                    MessageBox.Show($"已保存至    {textBox_exe.Text}_Patch.exe");
                    File.Delete("ILRepack.exe");
                    File.Delete("ILRepack.runtimeconfig.json");
                    File.Delete("temp.exe");
                }
                catch (Exception ex)
                {
                    MessageBox.Show("启动合并程序时发生错误: " + ex.Message);
                }
                MessageBox.Show($"已保存至temp.exe");
            }
            

        }
    }
}
