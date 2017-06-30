namespace ProtoFuzz
{
    partial class Main
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
            this.components = new System.ComponentModel.Container();
            this.lblAdapters = new System.Windows.Forms.Label();
            this.btnStop = new System.Windows.Forms.Button();
            this.btnStart = new System.Windows.Forms.Button();
            this.cbxAdapters = new System.Windows.Forms.ComboBox();
            this.lvwPacketCapture = new System.Windows.Forms.ListView();
            this.chrCaptureNumber = new System.Windows.Forms.ColumnHeader();
            this.chrCaptureSourceIp = new System.Windows.Forms.ColumnHeader();
            this.chrCaptureSourceMac = new System.Windows.Forms.ColumnHeader();
            this.chrCaptureSourcePort = new System.Windows.Forms.ColumnHeader();
            this.chrCaptureDestIp = new System.Windows.Forms.ColumnHeader();
            this.chrCaptureDestMac = new System.Windows.Forms.ColumnHeader();
            this.chrCaptureDestPort = new System.Windows.Forms.ColumnHeader();
            this.chrCaptureProtocol = new System.Windows.Forms.ColumnHeader();
            this.lblPackets = new System.Windows.Forms.Label();
            this.tbxPackets = new System.Windows.Forms.TextBox();
            this.tvwPacketCapture = new System.Windows.Forms.TreeView();
            this.rtbPacketCapture = new System.Windows.Forms.RichTextBox();
            this.mspProtoFuzz = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.cmsPacketBytes = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.sendPacketToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fuzzToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.bruteForceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.stringsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mspProtoFuzz.SuspendLayout();
            this.cmsPacketBytes.SuspendLayout();
            this.SuspendLayout();
            // 
            // lblAdapters
            // 
            this.lblAdapters.AutoSize = true;
            this.lblAdapters.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblAdapters.Location = new System.Drawing.Point(3, 28);
            this.lblAdapters.Name = "lblAdapters";
            this.lblAdapters.Size = new System.Drawing.Size(55, 15);
            this.lblAdapters.TabIndex = 6;
            this.lblAdapters.Text = "Adapters";
            // 
            // btnStop
            // 
            this.btnStop.BackColor = System.Drawing.Color.Red;
            this.btnStop.Location = new System.Drawing.Point(528, 60);
            this.btnStop.Name = "btnStop";
            this.btnStop.Size = new System.Drawing.Size(75, 23);
            this.btnStop.TabIndex = 8;
            this.btnStop.Text = "Stop";
            this.btnStop.UseVisualStyleBackColor = false;
            this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
            // 
            // btnStart
            // 
            this.btnStart.BackColor = System.Drawing.Color.LawnGreen;
            this.btnStart.Location = new System.Drawing.Point(443, 60);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(75, 23);
            this.btnStart.TabIndex = 7;
            this.btnStart.Text = "Start";
            this.btnStart.UseVisualStyleBackColor = false;
            this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
            // 
            // cbxAdapters
            // 
            this.cbxAdapters.Location = new System.Drawing.Point(87, 28);
            this.cbxAdapters.Name = "cbxAdapters";
            this.cbxAdapters.Size = new System.Drawing.Size(516, 21);
            this.cbxAdapters.TabIndex = 5;
            // 
            // lvwPacketCapture
            // 
            this.lvwPacketCapture.AllowColumnReorder = true;
            this.lvwPacketCapture.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chrCaptureNumber,
            this.chrCaptureSourceIp,
            this.chrCaptureSourceMac,
            this.chrCaptureSourcePort,
            this.chrCaptureDestIp,
            this.chrCaptureDestMac,
            this.chrCaptureDestPort,
            this.chrCaptureProtocol});
            this.lvwPacketCapture.Location = new System.Drawing.Point(4, 86);
            this.lvwPacketCapture.MultiSelect = false;
            this.lvwPacketCapture.Name = "lvwPacketCapture";
            this.lvwPacketCapture.Size = new System.Drawing.Size(600, 189);
            this.lvwPacketCapture.TabIndex = 9;
            this.lvwPacketCapture.UseCompatibleStateImageBehavior = false;
            this.lvwPacketCapture.View = System.Windows.Forms.View.Details;
            this.lvwPacketCapture.SelectedIndexChanged += new System.EventHandler(this.parsePacket_Click);
            // 
            // chrCaptureNumber
            // 
            this.chrCaptureNumber.Text = "No.";
            this.chrCaptureNumber.Width = 40;
            // 
            // chrCaptureSourceIp
            // 
            this.chrCaptureSourceIp.Text = "Source IP";
            this.chrCaptureSourceIp.Width = 100;
            // 
            // chrCaptureSourceMac
            // 
            this.chrCaptureSourceMac.Text = "Source Mac";
            this.chrCaptureSourceMac.Width = 90;
            // 
            // chrCaptureSourcePort
            // 
            this.chrCaptureSourcePort.Text = "Port";
            this.chrCaptureSourcePort.Width = 40;
            // 
            // chrCaptureDestIp
            // 
            this.chrCaptureDestIp.Text = "Dest IP";
            this.chrCaptureDestIp.Width = 100;
            // 
            // chrCaptureDestMac
            // 
            this.chrCaptureDestMac.Text = "Dest Mac";
            this.chrCaptureDestMac.Width = 90;
            // 
            // chrCaptureDestPort
            // 
            this.chrCaptureDestPort.Text = "Port";
            this.chrCaptureDestPort.Width = 40;
            // 
            // chrCaptureProtocol
            // 
            this.chrCaptureProtocol.Text = "Protocol";
            this.chrCaptureProtocol.Width = 100;
            // 
            // lblPackets
            // 
            this.lblPackets.AutoSize = true;
            this.lblPackets.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblPackets.Location = new System.Drawing.Point(3, 60);
            this.lblPackets.Name = "lblPackets";
            this.lblPackets.Size = new System.Drawing.Size(80, 15);
            this.lblPackets.TabIndex = 10;
            this.lblPackets.Text = "Max. Packets";
            // 
            // tbxPackets
            // 
            this.tbxPackets.Location = new System.Drawing.Point(87, 60);
            this.tbxPackets.Name = "tbxPackets";
            this.tbxPackets.Size = new System.Drawing.Size(49, 20);
            this.tbxPackets.TabIndex = 11;
            this.tbxPackets.Text = "1000";
            // 
            // tvwPacketCapture
            // 
            this.tvwPacketCapture.Location = new System.Drawing.Point(4, 281);
            this.tvwPacketCapture.Name = "tvwPacketCapture";
            this.tvwPacketCapture.Size = new System.Drawing.Size(600, 236);
            this.tvwPacketCapture.TabIndex = 16;
            this.tvwPacketCapture.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.tvwPacketCapture_AfterSelect);
            // 
            // rtbPacketCapture
            // 
            this.rtbPacketCapture.ContextMenuStrip = this.cmsPacketBytes;
            this.rtbPacketCapture.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.rtbPacketCapture.Location = new System.Drawing.Point(3, 543);
            this.rtbPacketCapture.Name = "rtbPacketCapture";
            this.rtbPacketCapture.ReadOnly = true;
            this.rtbPacketCapture.Size = new System.Drawing.Size(600, 187);
            this.rtbPacketCapture.TabIndex = 17;
            this.rtbPacketCapture.Text = "";
            // 
            // mspProtoFuzz
            // 
            this.mspProtoFuzz.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.mspProtoFuzz.Location = new System.Drawing.Point(0, 0);
            this.mspProtoFuzz.Name = "mspProtoFuzz";
            this.mspProtoFuzz.Size = new System.Drawing.Size(608, 24);
            this.mspProtoFuzz.TabIndex = 18;
            this.mspProtoFuzz.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // cmsPacketBytes
            // 
            this.cmsPacketBytes.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.sendPacketToolStripMenuItem,
            this.fuzzToolStripMenuItem});
            this.cmsPacketBytes.Name = "cmsPacketBytes";
            this.cmsPacketBytes.Size = new System.Drawing.Size(153, 70);
            // 
            // sendPacketToolStripMenuItem
            // 
            this.sendPacketToolStripMenuItem.Name = "sendPacketToolStripMenuItem";
            this.sendPacketToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.sendPacketToolStripMenuItem.Text = "Send Packet";
            this.sendPacketToolStripMenuItem.Click += new System.EventHandler(this.sendPacketToolStripMenuItem_Click);
            // 
            // fuzzToolStripMenuItem
            // 
            this.fuzzToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.bruteForceToolStripMenuItem,
            this.stringsToolStripMenuItem});
            this.fuzzToolStripMenuItem.Name = "fuzzToolStripMenuItem";
            this.fuzzToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.fuzzToolStripMenuItem.Text = "Fuzz";
            // 
            // bruteForceToolStripMenuItem
            // 
            this.bruteForceToolStripMenuItem.Name = "bruteForceToolStripMenuItem";
            this.bruteForceToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.bruteForceToolStripMenuItem.Text = "Brute Force";
            this.bruteForceToolStripMenuItem.Click += new System.EventHandler(this.bruteForceToolStripMenuItem_Click);
            // 
            // stringsToolStripMenuItem
            // 
            this.stringsToolStripMenuItem.Name = "stringsToolStripMenuItem";
            this.stringsToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.stringsToolStripMenuItem.Text = "Strings";
            this.stringsToolStripMenuItem.Click += new System.EventHandler(this.stringsToolStripMenuItem_Click);
            // 
            // Main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(608, 738);
            this.Controls.Add(this.mspProtoFuzz);
            this.Controls.Add(this.rtbPacketCapture);
            this.Controls.Add(this.tvwPacketCapture);
            this.Controls.Add(this.tbxPackets);
            this.Controls.Add(this.lblPackets);
            this.Controls.Add(this.lvwPacketCapture);
            this.Controls.Add(this.lblAdapters);
            this.Controls.Add(this.btnStop);
            this.Controls.Add(this.btnStart);
            this.Controls.Add(this.cbxAdapters);
            this.Name = "Main";
            this.Text = "ProtoFuzz";
            this.Load += new System.EventHandler(this.Main_Load);
            this.mspProtoFuzz.ResumeLayout(false);
            this.mspProtoFuzz.PerformLayout();
            this.cmsPacketBytes.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblAdapters;
        private System.Windows.Forms.Button btnStop;
        private System.Windows.Forms.Button btnStart;
        private System.Windows.Forms.ComboBox cbxAdapters;
        private System.Windows.Forms.ListView lvwPacketCapture;
        private System.Windows.Forms.ColumnHeader chrCaptureNumber;
        private System.Windows.Forms.ColumnHeader chrCaptureSourceIp;
        private System.Windows.Forms.ColumnHeader chrCaptureSourceMac;
        private System.Windows.Forms.ColumnHeader chrCaptureSourcePort;
        private System.Windows.Forms.ColumnHeader chrCaptureDestIp;
        private System.Windows.Forms.ColumnHeader chrCaptureDestMac;
        private System.Windows.Forms.ColumnHeader chrCaptureDestPort;
        private System.Windows.Forms.ColumnHeader chrCaptureProtocol;
        private System.Windows.Forms.Label lblPackets;
        private System.Windows.Forms.TextBox tbxPackets;
        private System.Windows.Forms.TreeView tvwPacketCapture;
        private System.Windows.Forms.RichTextBox rtbPacketCapture;
        private System.Windows.Forms.MenuStrip mspProtoFuzz;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ContextMenuStrip cmsPacketBytes;
        private System.Windows.Forms.ToolStripMenuItem sendPacketToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fuzzToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem bruteForceToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem stringsToolStripMenuItem;
    }
}

