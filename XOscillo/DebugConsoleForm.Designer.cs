﻿namespace XOscillo
{
   partial class DebugConsoleForm
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
         this.text = new System.Windows.Forms.TextBox();
         this.SuspendLayout();
         // 
         // text
         // 
         this.text.Dock = System.Windows.Forms.DockStyle.Fill;
         this.text.Location = new System.Drawing.Point(0, 0);
         this.text.Multiline = true;
         this.text.Name = "text";
         this.text.ReadOnly = true;
         this.text.Size = new System.Drawing.Size(284, 262);
         this.text.TabIndex = 0;
         // 
         // DebugConsoleForm
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(284, 262);
         this.Controls.Add(this.text);
         this.Name = "DebugConsoleForm";
         this.Text = "DebugConsoleForm";
         this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.DebugConsoleForm_FormClosing);
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private System.Windows.Forms.TextBox text;
   }
}