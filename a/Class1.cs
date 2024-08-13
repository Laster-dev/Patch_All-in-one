using a;
using System.Windows;

namespace TestDLL
{
    public class TestClass
    {
        public TestClass() { }
        public void TestFunc() // 改为 public 访问级别
        {
            MessageBox.Show("Myfunc");
            Form1 form1 = new Form1();
            form1.ShowDialog();

        }
    }
}
