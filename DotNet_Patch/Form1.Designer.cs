namespace DotNet_Patch
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.textBox_namespace = new System.Windows.Forms.TextBox();
            this.textBox_classname = new System.Windows.Forms.TextBox();
            this.textBox_dll = new System.Windows.Forms.TextBox();
            this.textBox_funcname = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.textBox_exe = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // textBox_namespace
            // 
            this.textBox_namespace.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.textBox_namespace.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textBox_namespace.ForeColor = System.Drawing.SystemColors.Menu;
            this.textBox_namespace.Location = new System.Drawing.Point(134, 117);
            this.textBox_namespace.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBox_namespace.Name = "textBox_namespace";
            this.textBox_namespace.Size = new System.Drawing.Size(265, 31);
            this.textBox_namespace.TabIndex = 0;
            this.textBox_namespace.Text = "TestDLL";
            // 
            // textBox_classname
            // 
            this.textBox_classname.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.textBox_classname.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textBox_classname.ForeColor = System.Drawing.SystemColors.Menu;
            this.textBox_classname.Location = new System.Drawing.Point(134, 167);
            this.textBox_classname.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBox_classname.Name = "textBox_classname";
            this.textBox_classname.Size = new System.Drawing.Size(265, 31);
            this.textBox_classname.TabIndex = 2;
            this.textBox_classname.Text = "TestClass";
            // 
            // textBox_dll
            // 
            this.textBox_dll.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.textBox_dll.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textBox_dll.ForeColor = System.Drawing.SystemColors.Menu;
            this.textBox_dll.Location = new System.Drawing.Point(134, 67);
            this.textBox_dll.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBox_dll.Name = "textBox_dll";
            this.textBox_dll.Size = new System.Drawing.Size(265, 31);
            this.textBox_dll.TabIndex = 3;
            this.textBox_dll.Text = "a.dll";
            // 
            // textBox_funcname
            // 
            this.textBox_funcname.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.textBox_funcname.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textBox_funcname.ForeColor = System.Drawing.SystemColors.Menu;
            this.textBox_funcname.Location = new System.Drawing.Point(134, 216);
            this.textBox_funcname.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBox_funcname.Name = "textBox_funcname";
            this.textBox_funcname.Size = new System.Drawing.Size(265, 31);
            this.textBox_funcname.TabIndex = 4;
            this.textBox_funcname.Text = "TestFunc";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.label1.Location = new System.Drawing.Point(12, 76);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(100, 24);
            this.label1.TabIndex = 5;
            this.label1.Text = "我的程序集";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.label2.Location = new System.Drawing.Point(12, 127);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(107, 24);
            this.label2.TabIndex = 6;
            this.label2.Text = "namespace";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.label3.Location = new System.Drawing.Point(12, 176);
            this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(98, 24);
            this.label3.TabIndex = 7;
            this.label3.Text = "classname";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.label4.Location = new System.Drawing.Point(12, 225);
            this.label4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(95, 24);
            this.label4.TabIndex = 8;
            this.label4.Text = "funcname";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.label5.Location = new System.Drawing.Point(12, 27);
            this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(64, 24);
            this.label5.TabIndex = 10;
            this.label5.Text = "白程序";
            // 
            // textBox_exe
            // 
            this.textBox_exe.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.textBox_exe.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textBox_exe.ForeColor = System.Drawing.SystemColors.Menu;
            this.textBox_exe.Location = new System.Drawing.Point(134, 17);
            this.textBox_exe.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.textBox_exe.Name = "textBox_exe";
            this.textBox_exe.Size = new System.Drawing.Size(265, 31);
            this.textBox_exe.TabIndex = 9;
            this.textBox_exe.Text = "test.exe";
            // 
            // button1
            // 
            this.button1.BackColor = System.Drawing.Color.Gray;
            this.button1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.button1.Location = new System.Drawing.Point(16, 315);
            this.button1.Margin = new System.Windows.Forms.Padding(4);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(383, 40);
            this.button1.TabIndex = 11;
            this.button1.Text = "Build";
            this.button1.UseVisualStyleBackColor = false;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Font = new System.Drawing.Font("微软雅黑", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.checkBox1.ForeColor = System.Drawing.SystemColors.ButtonFace;
            this.checkBox1.Location = new System.Drawing.Point(20, 264);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(134, 31);
            this.checkBox1.TabIndex = 12;
            this.checkBox1.Text = "合并程序集";
            this.checkBox1.UseVisualStyleBackColor = true;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(11F, 24F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(18)))), ((int)(((byte)(18)))), ((int)(((byte)(18)))));
            this.ClientSize = new System.Drawing.Size(414, 368);
            this.Controls.Add(this.checkBox1);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.textBox_exe);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textBox_funcname);
            this.Controls.Add(this.textBox_dll);
            this.Controls.Add(this.textBox_classname);
            this.Controls.Add(this.textBox_namespace);
            this.Font = new System.Drawing.Font("微软雅黑", 10.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBox_namespace;
        private System.Windows.Forms.TextBox textBox_classname;
        private System.Windows.Forms.TextBox textBox_dll;
        private System.Windows.Forms.TextBox textBox_funcname;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox textBox_exe;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.CheckBox checkBox1;
    }
}