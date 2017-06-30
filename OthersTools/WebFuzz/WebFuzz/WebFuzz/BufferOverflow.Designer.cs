namespace WebFuzz
{
    partial class BufferOverflow
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
            this.lblFillText = new System.Windows.Forms.Label();
            this.lblLength = new System.Windows.Forms.Label();
            this.lblMultiplier = new System.Windows.Forms.Label();
            this.tbxFillText = new System.Windows.Forms.TextBox();
            this.tbxLength = new System.Windows.Forms.TextBox();
            this.tbxMultiplier = new System.Windows.Forms.TextBox();
            this.btnOverflowSubmit = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lblFillText
            // 
            this.lblFillText.AutoSize = true;
            this.lblFillText.Location = new System.Drawing.Point(13, 13);
            this.lblFillText.Name = "lblFillText";
            this.lblFillText.Size = new System.Drawing.Size(56, 17);
            this.lblFillText.TabIndex = 0;
            this.lblFillText.Text = "Fill Text";
            // 
            // lblLength
            // 
            this.lblLength.AutoSize = true;
            this.lblLength.Location = new System.Drawing.Point(13, 40);
            this.lblLength.Name = "lblLength";
            this.lblLength.Size = new System.Drawing.Size(52, 17);
            this.lblLength.TabIndex = 1;
            this.lblLength.Text = "Length";
            // 
            // lblMultiplier
            // 
            this.lblMultiplier.AutoSize = true;
            this.lblMultiplier.Location = new System.Drawing.Point(13, 68);
            this.lblMultiplier.Name = "lblMultiplier";
            this.lblMultiplier.Size = new System.Drawing.Size(64, 17);
            this.lblMultiplier.TabIndex = 2;
            this.lblMultiplier.Text = "Multiplier";
            // 
            // tbxFillText
            // 
            this.tbxFillText.Location = new System.Drawing.Point(90, 10);
            this.tbxFillText.MaxLength = 10;
            this.tbxFillText.Name = "tbxFillText";
            this.tbxFillText.Size = new System.Drawing.Size(100, 22);
            this.tbxFillText.TabIndex = 3;
            this.tbxFillText.Text = "A";
            // 
            // tbxLength
            // 
            this.tbxLength.Location = new System.Drawing.Point(90, 37);
            this.tbxLength.MaxLength = 5;
            this.tbxLength.Name = "tbxLength";
            this.tbxLength.Size = new System.Drawing.Size(100, 22);
            this.tbxLength.TabIndex = 4;
            this.tbxLength.Text = "25";
            // 
            // tbxMultiplier
            // 
            this.tbxMultiplier.Location = new System.Drawing.Point(90, 65);
            this.tbxMultiplier.MaxLength = 5;
            this.tbxMultiplier.Name = "tbxMultiplier";
            this.tbxMultiplier.Size = new System.Drawing.Size(100, 22);
            this.tbxMultiplier.TabIndex = 5;
            this.tbxMultiplier.Text = "40";
            // 
            // btnOverflowSubmit
            // 
            this.btnOverflowSubmit.Location = new System.Drawing.Point(115, 93);
            this.btnOverflowSubmit.Name = "btnOverflowSubmit";
            this.btnOverflowSubmit.Size = new System.Drawing.Size(75, 23);
            this.btnOverflowSubmit.TabIndex = 6;
            this.btnOverflowSubmit.Text = "Submit";
            this.btnOverflowSubmit.UseVisualStyleBackColor = true;
            this.btnOverflowSubmit.Click += new System.EventHandler(this.btnOverflowSubmit_Click);
            // 
            // BufferOverflow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(206, 128);
            this.ControlBox = false;
            this.Controls.Add(this.btnOverflowSubmit);
            this.Controls.Add(this.tbxMultiplier);
            this.Controls.Add(this.tbxLength);
            this.Controls.Add(this.tbxFillText);
            this.Controls.Add(this.lblMultiplier);
            this.Controls.Add(this.lblLength);
            this.Controls.Add(this.lblFillText);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "BufferOverflow";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Overflow";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblFillText;
        private System.Windows.Forms.Label lblLength;
        private System.Windows.Forms.Label lblMultiplier;
        private System.Windows.Forms.TextBox tbxFillText;
        private System.Windows.Forms.TextBox tbxLength;
        private System.Windows.Forms.TextBox tbxMultiplier;
        private System.Windows.Forms.Button btnOverflowSubmit;
    }
}