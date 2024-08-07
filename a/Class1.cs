using System.Windows;

namespace TestDLL
{
    public class TestClass
    {
        public TestClass() { }
        public void TestFunc() // 改为 public 访问级别
        {
            MessageBox.Show("Myfunc");
        }
    }
}
