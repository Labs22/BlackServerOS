/*
    FileFuzz
    Copyright (C) 2005 Michael Sutton <msutton@idefense.com,michaelawsutton@gmail.com>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place, Suite 330, Boston, MA 02111-1307 USA
*/

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using System.Diagnostics;
using System.Text.RegularExpressions;
using System.Text;
using System.Threading;

namespace FileFuzz
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class frmFileFuzz : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Label lblSourceFile;
		private System.Windows.Forms.TextBox tbxSourceFile;
		private System.Windows.Forms.Button btnSourceFile;
		private System.Windows.Forms.OpenFileDialog ofdSourceFile;
		private System.Windows.Forms.Label lblTargetDirectory;
		private System.Windows.Forms.TextBox tbxTargetDirectory;
		private System.Windows.Forms.Button btnTargetDirectory;
		private System.Windows.Forms.FolderBrowserDialog fbdTargetDirectory;
		private System.Windows.Forms.Button btnCreateFiles;
		private System.Windows.Forms.Button btnExecute;
		private System.Windows.Forms.RichTextBox rtbLog;
		private System.Windows.Forms.TextBox tbxExecuteApp;
		private System.Windows.Forms.TextBox tbxExecuteArgs;
		private System.Windows.Forms.Label lblExecuteApp;
		private System.Windows.Forms.Label lblExecuteArgs;
		private System.Windows.Forms.Button btnExecuteAppSelect;
		private System.Windows.Forms.OpenFileDialog ofdExecuteApp;
		private System.Windows.Forms.Label lblFileType;
		private System.Windows.Forms.ComboBox cbxFileType;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.TextBox tbxTargetByte;
		private System.Windows.Forms.TextBox tbxTargetBytesNumber;
		private System.Windows.Forms.Label lblTargetBytesMultiple;
		private System.Windows.Forms.GroupBox gbxTarget;
		private System.Windows.Forms.ProgressBar pbrProgressBar;
		private System.Windows.Forms.Button btnKillAll;
		private System.Windows.Forms.GroupBox gbxScope;
		private System.Windows.Forms.RadioButton rbtAllBytes;
		private System.Windows.Forms.RadioButton rbtRange;
		private System.Windows.Forms.RadioButton rbtRegex;
		private System.Windows.Forms.Label lbltbxTargetBytes;
		public System.Windows.Forms.CheckBox cbxStripFileExt;
		public System.Windows.Forms.CheckBox cbxExcludeTargetDir;
		private System.Windows.Forms.Button btnClearLog;
		private System.Windows.Forms.TextBox tbxCount;
		private System.Windows.Forms.RadioButton rbtDepth;
		private System.Windows.Forms.Label lblExecuteLocation;
		private System.Windows.Forms.Label lblRangeStart;
		private System.Windows.Forms.Label lblRangeFinish;
		private System.Windows.Forms.TextBox tbxRangeStart;
		private System.Windows.Forms.TextBox tbxRangeFinish;
		private System.Windows.Forms.TextBox tbxDepthLocation;
		private System.Windows.Forms.TextBox tbxTargetByteFinish;
		private System.Windows.Forms.TabControl tclMain;
		private System.Windows.Forms.TabPage execute;
		private System.Windows.Forms.TabPage create;
		private System.Windows.Forms.Label lblCount;
		private System.Windows.Forms.TextBox tbxMilliseconds;
		private System.Windows.Forms.Label lblMilliseconds;
		private System.Windows.Forms.Label lblStartFile;
		private System.Windows.Forms.Label lblFinishFile;
		private System.Windows.Forms.TextBox tbxStartFile;
		private System.Windows.Forms.TextBox tbxFinishFile;
		private System.Windows.Forms.TextBox tbxRegexFind;
		private System.Windows.Forms.Label lblRegexFind;
		private System.Windows.Forms.Label lblRegexReplace;
		private System.Windows.Forms.TextBox tbxRegexReplace;
		private System.Windows.Forms.Label lblRegexReplaceMultiplier;
		private System.Windows.Forms.TextBox tbxRegexReplaceMultiplier;
		private System.Windows.Forms.TextBox tbxRegexReplaceCount;
		private System.Windows.Forms.Label lblRegexReplaceCount;
		private System.Windows.Forms.MenuItem mnuFile;
		private System.Windows.Forms.MenuItem mnuFileExit;
		private System.Windows.Forms.MenuItem mnuEdit;
		private System.Windows.Forms.MenuItem mnuEditCopy;
		private System.Windows.Forms.MainMenu mainMenu;
		private System.Windows.Forms.ErrorProvider errorProvider;
		private System.Windows.Forms.TextBox tbxErrorCreate;
		private System.Windows.Forms.TextBox tbxErrorExecute;

		//Delegates to update progress bar
		public delegate void pbrProgressBarStart(int start, int total);
		public delegate void pbrProgressBarUpdate(int increment);

		//Delegate to update counter
		public delegate void tbxCountUpdate(int value);

		//Delegate to write log data
		public delegate void rtbLogOutput(string output);

		//Variables from targets.xml
		public ReadXml xmlAudits;
		string fileName;
		string fileDescription;
		string appName;
		string appDescription;
		string appAction;
		string appLaunch;
		string appFlags;
		string sourceFile;
		string sourceDir;
		string targetDir;

		public frmFileFuzz()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(frmFileFuzz));
			this.lblSourceFile = new System.Windows.Forms.Label();
			this.tbxSourceFile = new System.Windows.Forms.TextBox();
			this.btnSourceFile = new System.Windows.Forms.Button();
			this.ofdSourceFile = new System.Windows.Forms.OpenFileDialog();
			this.lblTargetDirectory = new System.Windows.Forms.Label();
			this.tbxTargetDirectory = new System.Windows.Forms.TextBox();
			this.btnTargetDirectory = new System.Windows.Forms.Button();
			this.fbdTargetDirectory = new System.Windows.Forms.FolderBrowserDialog();
			this.btnCreateFiles = new System.Windows.Forms.Button();
			this.btnExecute = new System.Windows.Forms.Button();
			this.lblRangeStart = new System.Windows.Forms.Label();
			this.lblRangeFinish = new System.Windows.Forms.Label();
			this.rtbLog = new System.Windows.Forms.RichTextBox();
			this.tbxExecuteApp = new System.Windows.Forms.TextBox();
			this.tbxExecuteArgs = new System.Windows.Forms.TextBox();
			this.lblExecuteApp = new System.Windows.Forms.Label();
			this.lblExecuteArgs = new System.Windows.Forms.Label();
			this.btnExecuteAppSelect = new System.Windows.Forms.Button();
			this.ofdExecuteApp = new System.Windows.Forms.OpenFileDialog();
			this.tbxRangeStart = new System.Windows.Forms.TextBox();
			this.tbxRangeFinish = new System.Windows.Forms.TextBox();
			this.lblFileType = new System.Windows.Forms.Label();
			this.cbxFileType = new System.Windows.Forms.ComboBox();
			this.tbxMilliseconds = new System.Windows.Forms.TextBox();
			this.lblMilliseconds = new System.Windows.Forms.Label();
			this.tbxTargetByte = new System.Windows.Forms.TextBox();
			this.tbxTargetBytesNumber = new System.Windows.Forms.TextBox();
			this.lblTargetBytesMultiple = new System.Windows.Forms.Label();
			this.gbxTarget = new System.Windows.Forms.GroupBox();
			this.tbxTargetByteFinish = new System.Windows.Forms.TextBox();
			this.lbltbxTargetBytes = new System.Windows.Forms.Label();
			this.btnKillAll = new System.Windows.Forms.Button();
			this.pbrProgressBar = new System.Windows.Forms.ProgressBar();
			this.gbxScope = new System.Windows.Forms.GroupBox();
			this.tbxRegexReplaceCount = new System.Windows.Forms.TextBox();
			this.lblRegexReplaceCount = new System.Windows.Forms.Label();
			this.tbxRegexReplaceMultiplier = new System.Windows.Forms.TextBox();
			this.lblRegexReplaceMultiplier = new System.Windows.Forms.Label();
			this.tbxRegexReplace = new System.Windows.Forms.TextBox();
			this.lblRegexReplace = new System.Windows.Forms.Label();
			this.lblRegexFind = new System.Windows.Forms.Label();
			this.rbtDepth = new System.Windows.Forms.RadioButton();
			this.tbxRegexFind = new System.Windows.Forms.TextBox();
			this.rbtRegex = new System.Windows.Forms.RadioButton();
			this.rbtRange = new System.Windows.Forms.RadioButton();
			this.rbtAllBytes = new System.Windows.Forms.RadioButton();
			this.lblExecuteLocation = new System.Windows.Forms.Label();
			this.tbxDepthLocation = new System.Windows.Forms.TextBox();
			this.cbxStripFileExt = new System.Windows.Forms.CheckBox();
			this.cbxExcludeTargetDir = new System.Windows.Forms.CheckBox();
			this.btnClearLog = new System.Windows.Forms.Button();
			this.tbxCount = new System.Windows.Forms.TextBox();
			this.tclMain = new System.Windows.Forms.TabControl();
			this.create = new System.Windows.Forms.TabPage();
			this.tbxErrorCreate = new System.Windows.Forms.TextBox();
			this.execute = new System.Windows.Forms.TabPage();
			this.tbxErrorExecute = new System.Windows.Forms.TextBox();
			this.tbxFinishFile = new System.Windows.Forms.TextBox();
			this.tbxStartFile = new System.Windows.Forms.TextBox();
			this.lblFinishFile = new System.Windows.Forms.Label();
			this.lblStartFile = new System.Windows.Forms.Label();
			this.lblCount = new System.Windows.Forms.Label();
			this.mnuFile = new System.Windows.Forms.MenuItem();
			this.mnuFileExit = new System.Windows.Forms.MenuItem();
			this.mnuEdit = new System.Windows.Forms.MenuItem();
			this.mnuEditCopy = new System.Windows.Forms.MenuItem();
			this.mainMenu = new System.Windows.Forms.MainMenu();
			this.errorProvider = new System.Windows.Forms.ErrorProvider();
			this.gbxTarget.SuspendLayout();
			this.gbxScope.SuspendLayout();
			this.tclMain.SuspendLayout();
			this.create.SuspendLayout();
			this.execute.SuspendLayout();
			this.SuspendLayout();
			// 
			// lblSourceFile
			// 
			this.lblSourceFile.AutoSize = true;
			this.lblSourceFile.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblSourceFile.Location = new System.Drawing.Point(16, 16);
			this.lblSourceFile.Name = "lblSourceFile";
			this.lblSourceFile.Size = new System.Drawing.Size(93, 22);
			this.lblSourceFile.TabIndex = 0;
			this.lblSourceFile.Text = "Source File";
			// 
			// tbxSourceFile
			// 
			this.tbxSourceFile.Location = new System.Drawing.Point(16, 48);
			this.tbxSourceFile.Name = "tbxSourceFile";
			this.tbxSourceFile.Size = new System.Drawing.Size(538, 22);
			this.tbxSourceFile.TabIndex = 1;
			this.tbxSourceFile.Text = "";
			// 
			// btnSourceFile
			// 
			this.btnSourceFile.Location = new System.Drawing.Point(568, 48);
			this.btnSourceFile.Name = "btnSourceFile";
			this.btnSourceFile.Size = new System.Drawing.Size(90, 27);
			this.btnSourceFile.TabIndex = 2;
			this.btnSourceFile.Text = "Select";
			this.btnSourceFile.Click += new System.EventHandler(this.btnSourceFile_Click);
			// 
			// ofdSourceFile
			// 
			this.ofdSourceFile.Filter = "All files (*.*)|*.*|All files (*.*)|*.*";
			this.ofdSourceFile.FilterIndex = 2;
			this.ofdSourceFile.InitialDirectory = "c:\\";
			this.ofdSourceFile.RestoreDirectory = true;
			this.ofdSourceFile.Title = "Select Source File";
			// 
			// lblTargetDirectory
			// 
			this.lblTargetDirectory.AutoSize = true;
			this.lblTargetDirectory.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblTargetDirectory.Location = new System.Drawing.Point(16, 80);
			this.lblTargetDirectory.Name = "lblTargetDirectory";
			this.lblTargetDirectory.Size = new System.Drawing.Size(129, 22);
			this.lblTargetDirectory.TabIndex = 3;
			this.lblTargetDirectory.Text = "Target Directory";
			// 
			// tbxTargetDirectory
			// 
			this.tbxTargetDirectory.Location = new System.Drawing.Point(16, 112);
			this.tbxTargetDirectory.Name = "tbxTargetDirectory";
			this.tbxTargetDirectory.Size = new System.Drawing.Size(538, 22);
			this.tbxTargetDirectory.TabIndex = 4;
			this.tbxTargetDirectory.Text = "";
			// 
			// btnTargetDirectory
			// 
			this.btnTargetDirectory.Location = new System.Drawing.Point(568, 112);
			this.btnTargetDirectory.Name = "btnTargetDirectory";
			this.btnTargetDirectory.Size = new System.Drawing.Size(90, 26);
			this.btnTargetDirectory.TabIndex = 5;
			this.btnTargetDirectory.Text = "Select";
			this.btnTargetDirectory.Click += new System.EventHandler(this.btnTargetDirectory_Click);
			// 
			// fbdTargetDirectory
			// 
			this.fbdTargetDirectory.Description = "Select Target Directory";
			this.fbdTargetDirectory.SelectedPath = "C:\\Documents and Settings\\msutton\\My Documents\\iDEFENSE\\Labs\\FileFuzz";
			// 
			// btnCreateFiles
			// 
			this.btnCreateFiles.Location = new System.Drawing.Point(568, 144);
			this.btnCreateFiles.Name = "btnCreateFiles";
			this.btnCreateFiles.Size = new System.Drawing.Size(90, 26);
			this.btnCreateFiles.TabIndex = 6;
			this.btnCreateFiles.Text = "Create";
			this.btnCreateFiles.Click += new System.EventHandler(this.btnCreateFiles_Click);
			// 
			// btnExecute
			// 
			this.btnExecute.Location = new System.Drawing.Point(568, 112);
			this.btnExecute.Name = "btnExecute";
			this.btnExecute.Size = new System.Drawing.Size(90, 26);
			this.btnExecute.TabIndex = 10;
			this.btnExecute.Text = "Execute";
			this.btnExecute.Click += new System.EventHandler(this.btnExecute_Click);
			// 
			// lblRangeStart
			// 
			this.lblRangeStart.AutoSize = true;
			this.lblRangeStart.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblRangeStart.Location = new System.Drawing.Point(106, 46);
			this.lblRangeStart.Name = "lblRangeStart";
			this.lblRangeStart.Size = new System.Drawing.Size(42, 22);
			this.lblRangeStart.TabIndex = 11;
			this.lblRangeStart.Text = "Start";
			this.lblRangeStart.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// lblRangeFinish
			// 
			this.lblRangeFinish.AutoSize = true;
			this.lblRangeFinish.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblRangeFinish.Location = new System.Drawing.Point(230, 46);
			this.lblRangeFinish.Name = "lblRangeFinish";
			this.lblRangeFinish.Size = new System.Drawing.Size(52, 22);
			this.lblRangeFinish.TabIndex = 13;
			this.lblRangeFinish.Text = "Finish";
			this.lblRangeFinish.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// rtbLog
			// 
			this.rtbLog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.rtbLog.AutoSize = true;
			this.rtbLog.Font = new System.Drawing.Font("Courier New", 7.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.rtbLog.Location = new System.Drawing.Point(8, 480);
			this.rtbLog.Name = "rtbLog";
			this.rtbLog.Size = new System.Drawing.Size(664, 184);
			this.rtbLog.TabIndex = 15;
			this.rtbLog.Text = "";
			// 
			// tbxExecuteApp
			// 
			this.tbxExecuteApp.Location = new System.Drawing.Point(16, 48);
			this.tbxExecuteApp.Name = "tbxExecuteApp";
			this.tbxExecuteApp.Size = new System.Drawing.Size(536, 22);
			this.tbxExecuteApp.TabIndex = 16;
			this.tbxExecuteApp.Text = "";
			// 
			// tbxExecuteArgs
			// 
			this.tbxExecuteArgs.Location = new System.Drawing.Point(16, 112);
			this.tbxExecuteArgs.Name = "tbxExecuteArgs";
			this.tbxExecuteArgs.Size = new System.Drawing.Size(536, 22);
			this.tbxExecuteArgs.TabIndex = 17;
			this.tbxExecuteArgs.Text = "";
			// 
			// lblExecuteApp
			// 
			this.lblExecuteApp.AutoSize = true;
			this.lblExecuteApp.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblExecuteApp.Location = new System.Drawing.Point(16, 16);
			this.lblExecuteApp.Name = "lblExecuteApp";
			this.lblExecuteApp.Size = new System.Drawing.Size(90, 22);
			this.lblExecuteApp.TabIndex = 18;
			this.lblExecuteApp.Text = "Application";
			// 
			// lblExecuteArgs
			// 
			this.lblExecuteArgs.AutoSize = true;
			this.lblExecuteArgs.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblExecuteArgs.Location = new System.Drawing.Point(16, 80);
			this.lblExecuteArgs.Name = "lblExecuteArgs";
			this.lblExecuteArgs.Size = new System.Drawing.Size(89, 22);
			this.lblExecuteArgs.TabIndex = 19;
			this.lblExecuteArgs.Text = "Arguments";
			// 
			// btnExecuteAppSelect
			// 
			this.btnExecuteAppSelect.Location = new System.Drawing.Point(568, 48);
			this.btnExecuteAppSelect.Name = "btnExecuteAppSelect";
			this.btnExecuteAppSelect.Size = new System.Drawing.Size(90, 26);
			this.btnExecuteAppSelect.TabIndex = 20;
			this.btnExecuteAppSelect.Text = "Select";
			this.btnExecuteAppSelect.Click += new System.EventHandler(this.btnExecuteAppSelect_Click);
			// 
			// ofdExecuteApp
			// 
			this.ofdExecuteApp.Filter = "All files (*.*)|*.*|All files (*.*)|*.*";
			this.ofdExecuteApp.FilterIndex = 2;
			this.ofdExecuteApp.InitialDirectory = "c:\\windows\\tasks";
			this.ofdExecuteApp.RestoreDirectory = true;
			this.ofdExecuteApp.Title = "Select Source File";
			// 
			// tbxRangeStart
			// 
			this.tbxRangeStart.Location = new System.Drawing.Point(154, 46);
			this.tbxRangeStart.Name = "tbxRangeStart";
			this.tbxRangeStart.Size = new System.Drawing.Size(67, 22);
			this.tbxRangeStart.TabIndex = 21;
			this.tbxRangeStart.Text = "0";
			this.tbxRangeStart.Validating += new System.ComponentModel.CancelEventHandler(this.tbxRangeStart_Validating);
			// 
			// tbxRangeFinish
			// 
			this.tbxRangeFinish.Location = new System.Drawing.Point(288, 46);
			this.tbxRangeFinish.Name = "tbxRangeFinish";
			this.tbxRangeFinish.Size = new System.Drawing.Size(67, 22);
			this.tbxRangeFinish.TabIndex = 22;
			this.tbxRangeFinish.Text = "10";
			this.tbxRangeFinish.Validating += new System.ComponentModel.CancelEventHandler(this.tbxRangeFinish_Validating);
			// 
			// lblFileType
			// 
			this.lblFileType.AutoSize = true;
			this.lblFileType.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblFileType.Location = new System.Drawing.Point(19, 9);
			this.lblFileType.Name = "lblFileType";
			this.lblFileType.Size = new System.Drawing.Size(77, 22);
			this.lblFileType.TabIndex = 23;
			this.lblFileType.Text = "File Type";
			this.lblFileType.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// cbxFileType
			// 
			this.cbxFileType.Location = new System.Drawing.Point(106, 9);
			this.cbxFileType.Name = "cbxFileType";
			this.cbxFileType.Size = new System.Drawing.Size(192, 24);
			this.cbxFileType.TabIndex = 24;
			this.cbxFileType.SelectedIndexChanged += new System.EventHandler(this.cbxFileType_SelectedIndexChanged);
			// 
			// tbxMilliseconds
			// 
			this.tbxMilliseconds.Location = new System.Drawing.Point(120, 208);
			this.tbxMilliseconds.Name = "tbxMilliseconds";
			this.tbxMilliseconds.Size = new System.Drawing.Size(57, 22);
			this.tbxMilliseconds.TabIndex = 26;
			this.tbxMilliseconds.Text = "2000";
			this.tbxMilliseconds.Validating += new System.ComponentModel.CancelEventHandler(this.tbxMilliseconds_Validating);
			// 
			// lblMilliseconds
			// 
			this.lblMilliseconds.AutoSize = true;
			this.lblMilliseconds.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblMilliseconds.Location = new System.Drawing.Point(16, 208);
			this.lblMilliseconds.Name = "lblMilliseconds";
			this.lblMilliseconds.TabIndex = 27;
			this.lblMilliseconds.Text = "Milliseconds";
			this.lblMilliseconds.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// tbxTargetByte
			// 
			this.tbxTargetByte.Location = new System.Drawing.Point(104, 24);
			this.tbxTargetByte.MaxLength = 2;
			this.tbxTargetByte.Name = "tbxTargetByte";
			this.tbxTargetByte.Size = new System.Drawing.Size(32, 22);
			this.tbxTargetByte.TabIndex = 31;
			this.tbxTargetByte.Text = "00";
			this.tbxTargetByte.Validating += new System.ComponentModel.CancelEventHandler(this.tbxTargetByte_Validating);
			// 
			// tbxTargetBytesNumber
			// 
			this.tbxTargetBytesNumber.Location = new System.Drawing.Point(104, 112);
			this.tbxTargetBytesNumber.MaxLength = 4;
			this.tbxTargetBytesNumber.Name = "tbxTargetBytesNumber";
			this.tbxTargetBytesNumber.Size = new System.Drawing.Size(48, 22);
			this.tbxTargetBytesNumber.TabIndex = 36;
			this.tbxTargetBytesNumber.Text = "4";
			this.tbxTargetBytesNumber.Validating += new System.ComponentModel.CancelEventHandler(this.tbxTargetBytesNumber_Validating);
			// 
			// lblTargetBytesMultiple
			// 
			this.lblTargetBytesMultiple.AutoSize = true;
			this.lblTargetBytesMultiple.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblTargetBytesMultiple.Location = new System.Drawing.Point(104, 88);
			this.lblTargetBytesMultiple.Name = "lblTargetBytesMultiple";
			this.lblTargetBytesMultiple.Size = new System.Drawing.Size(18, 22);
			this.lblTargetBytesMultiple.TabIndex = 37;
			this.lblTargetBytesMultiple.Text = "X";
			// 
			// gbxTarget
			// 
			this.gbxTarget.Controls.Add(this.tbxTargetByteFinish);
			this.gbxTarget.Controls.Add(this.lbltbxTargetBytes);
			this.gbxTarget.Controls.Add(this.tbxTargetByte);
			this.gbxTarget.Controls.Add(this.tbxTargetBytesNumber);
			this.gbxTarget.Controls.Add(this.lblTargetBytesMultiple);
			this.gbxTarget.Location = new System.Drawing.Point(16, 176);
			this.gbxTarget.Name = "gbxTarget";
			this.gbxTarget.Size = new System.Drawing.Size(256, 144);
			this.gbxTarget.TabIndex = 38;
			this.gbxTarget.TabStop = false;
			this.gbxTarget.Text = "Target";
			// 
			// tbxTargetByteFinish
			// 
			this.tbxTargetByteFinish.Enabled = false;
			this.tbxTargetByteFinish.Location = new System.Drawing.Point(104, 56);
			this.tbxTargetByteFinish.MaxLength = 2;
			this.tbxTargetByteFinish.Name = "tbxTargetByteFinish";
			this.tbxTargetByteFinish.Size = new System.Drawing.Size(32, 22);
			this.tbxTargetByteFinish.TabIndex = 39;
			this.tbxTargetByteFinish.Text = "FF";
			this.tbxTargetByteFinish.Validating += new System.ComponentModel.CancelEventHandler(this.tbxTargetByteFinish_Validating);
			// 
			// lbltbxTargetBytes
			// 
			this.lbltbxTargetBytes.Enabled = false;
			this.lbltbxTargetBytes.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lbltbxTargetBytes.Location = new System.Drawing.Point(8, 24);
			this.lbltbxTargetBytes.Name = "lbltbxTargetBytes";
			this.lbltbxTargetBytes.Size = new System.Drawing.Size(88, 40);
			this.lbltbxTargetBytes.TabIndex = 38;
			this.lbltbxTargetBytes.Text = "Byte(s) to Overwrite";
			this.lbltbxTargetBytes.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// btnKillAll
			// 
			this.btnKillAll.Location = new System.Drawing.Point(568, 168);
			this.btnKillAll.Name = "btnKillAll";
			this.btnKillAll.Size = new System.Drawing.Size(90, 26);
			this.btnKillAll.TabIndex = 39;
			this.btnKillAll.Text = "Kill All";
			this.btnKillAll.Click += new System.EventHandler(this.btnKill_Click);
			// 
			// pbrProgressBar
			// 
			this.pbrProgressBar.Location = new System.Drawing.Point(8, 456);
			this.pbrProgressBar.Name = "pbrProgressBar";
			this.pbrProgressBar.Size = new System.Drawing.Size(664, 18);
			this.pbrProgressBar.Step = 1;
			this.pbrProgressBar.TabIndex = 40;
			// 
			// gbxScope
			// 
			this.gbxScope.Controls.Add(this.tbxRegexReplaceCount);
			this.gbxScope.Controls.Add(this.lblRegexReplaceCount);
			this.gbxScope.Controls.Add(this.tbxRegexReplaceMultiplier);
			this.gbxScope.Controls.Add(this.lblRegexReplaceMultiplier);
			this.gbxScope.Controls.Add(this.tbxRegexReplace);
			this.gbxScope.Controls.Add(this.lblRegexReplace);
			this.gbxScope.Controls.Add(this.lblRegexFind);
			this.gbxScope.Controls.Add(this.rbtDepth);
			this.gbxScope.Controls.Add(this.tbxRegexFind);
			this.gbxScope.Controls.Add(this.rbtRegex);
			this.gbxScope.Controls.Add(this.rbtRange);
			this.gbxScope.Controls.Add(this.rbtAllBytes);
			this.gbxScope.Controls.Add(this.lblRangeFinish);
			this.gbxScope.Controls.Add(this.tbxRangeStart);
			this.gbxScope.Controls.Add(this.tbxRangeFinish);
			this.gbxScope.Controls.Add(this.lblRangeStart);
			this.gbxScope.Controls.Add(this.lblExecuteLocation);
			this.gbxScope.Controls.Add(this.tbxDepthLocation);
			this.gbxScope.Location = new System.Drawing.Point(288, 176);
			this.gbxScope.Name = "gbxScope";
			this.gbxScope.Size = new System.Drawing.Size(375, 176);
			this.gbxScope.TabIndex = 43;
			this.gbxScope.TabStop = false;
			this.gbxScope.Text = "Scope";
			// 
			// tbxRegexReplaceCount
			// 
			this.tbxRegexReplaceCount.Enabled = false;
			this.tbxRegexReplaceCount.Location = new System.Drawing.Point(312, 144);
			this.tbxRegexReplaceCount.Name = "tbxRegexReplaceCount";
			this.tbxRegexReplaceCount.Size = new System.Drawing.Size(40, 22);
			this.tbxRegexReplaceCount.TabIndex = 55;
			this.tbxRegexReplaceCount.Text = "10";
			this.tbxRegexReplaceCount.Validating += new System.ComponentModel.CancelEventHandler(this.tbxRegexReplaceCount_Validating);
			// 
			// lblRegexReplaceCount
			// 
			this.lblRegexReplaceCount.AutoSize = true;
			this.lblRegexReplaceCount.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblRegexReplaceCount.Location = new System.Drawing.Point(291, 144);
			this.lblRegexReplaceCount.Name = "lblRegexReplaceCount";
			this.lblRegexReplaceCount.Size = new System.Drawing.Size(18, 22);
			this.lblRegexReplaceCount.TabIndex = 54;
			this.lblRegexReplaceCount.Text = "X";
			this.lblRegexReplaceCount.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// tbxRegexReplaceMultiplier
			// 
			this.tbxRegexReplaceMultiplier.Enabled = false;
			this.tbxRegexReplaceMultiplier.Location = new System.Drawing.Point(248, 144);
			this.tbxRegexReplaceMultiplier.Name = "tbxRegexReplaceMultiplier";
			this.tbxRegexReplaceMultiplier.Size = new System.Drawing.Size(40, 22);
			this.tbxRegexReplaceMultiplier.TabIndex = 53;
			this.tbxRegexReplaceMultiplier.Text = "100";
			this.tbxRegexReplaceMultiplier.Validating += new System.ComponentModel.CancelEventHandler(this.tbxRegexReplaceMultiplier_Validating);
			// 
			// lblRegexReplaceMultiplier
			// 
			this.lblRegexReplaceMultiplier.AutoSize = true;
			this.lblRegexReplaceMultiplier.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblRegexReplaceMultiplier.Location = new System.Drawing.Point(227, 144);
			this.lblRegexReplaceMultiplier.Name = "lblRegexReplaceMultiplier";
			this.lblRegexReplaceMultiplier.Size = new System.Drawing.Size(18, 22);
			this.lblRegexReplaceMultiplier.TabIndex = 52;
			this.lblRegexReplaceMultiplier.Text = "X";
			this.lblRegexReplaceMultiplier.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// tbxRegexReplace
			// 
			this.tbxRegexReplace.Enabled = false;
			this.tbxRegexReplace.Location = new System.Drawing.Point(176, 144);
			this.tbxRegexReplace.Name = "tbxRegexReplace";
			this.tbxRegexReplace.Size = new System.Drawing.Size(48, 22);
			this.tbxRegexReplace.TabIndex = 51;
			this.tbxRegexReplace.Text = "A";
			// 
			// lblRegexReplace
			// 
			this.lblRegexReplace.AutoSize = true;
			this.lblRegexReplace.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblRegexReplace.Location = new System.Drawing.Point(104, 144);
			this.lblRegexReplace.Name = "lblRegexReplace";
			this.lblRegexReplace.Size = new System.Drawing.Size(69, 22);
			this.lblRegexReplace.TabIndex = 50;
			this.lblRegexReplace.Text = "Replace";
			this.lblRegexReplace.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// lblRegexFind
			// 
			this.lblRegexFind.AutoSize = true;
			this.lblRegexFind.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblRegexFind.Location = new System.Drawing.Point(104, 112);
			this.lblRegexFind.Name = "lblRegexFind";
			this.lblRegexFind.Size = new System.Drawing.Size(40, 22);
			this.lblRegexFind.TabIndex = 49;
			this.lblRegexFind.Text = "Find";
			this.lblRegexFind.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// rbtDepth
			// 
			this.rbtDepth.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
			this.rbtDepth.Location = new System.Drawing.Point(19, 80);
			this.rbtDepth.Name = "rbtDepth";
			this.rbtDepth.Size = new System.Drawing.Size(77, 28);
			this.rbtDepth.TabIndex = 27;
			this.rbtDepth.Text = "Depth";
			this.rbtDepth.CheckedChanged += new System.EventHandler(this.scope_CheckedChanged);
			// 
			// tbxRegexFind
			// 
			this.tbxRegexFind.Enabled = false;
			this.tbxRegexFind.Location = new System.Drawing.Point(176, 112);
			this.tbxRegexFind.Name = "tbxRegexFind";
			this.tbxRegexFind.Size = new System.Drawing.Size(176, 22);
			this.tbxRegexFind.TabIndex = 26;
			this.tbxRegexFind.Text = "=";
			// 
			// rbtRegex
			// 
			this.rbtRegex.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
			this.rbtRegex.Location = new System.Drawing.Point(19, 112);
			this.rbtRegex.Name = "rbtRegex";
			this.rbtRegex.Size = new System.Drawing.Size(77, 28);
			this.rbtRegex.TabIndex = 25;
			this.rbtRegex.Text = "Match";
			this.rbtRegex.CheckedChanged += new System.EventHandler(this.scope_CheckedChanged);
			// 
			// rbtRange
			// 
			this.rbtRange.Checked = true;
			this.rbtRange.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
			this.rbtRange.Location = new System.Drawing.Point(19, 46);
			this.rbtRange.Name = "rbtRange";
			this.rbtRange.Size = new System.Drawing.Size(77, 28);
			this.rbtRange.TabIndex = 24;
			this.rbtRange.TabStop = true;
			this.rbtRange.Text = "Range";
			this.rbtRange.CheckedChanged += new System.EventHandler(this.scope_CheckedChanged);
			// 
			// rbtAllBytes
			// 
			this.rbtAllBytes.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
			this.rbtAllBytes.Location = new System.Drawing.Point(19, 18);
			this.rbtAllBytes.Name = "rbtAllBytes";
			this.rbtAllBytes.Size = new System.Drawing.Size(96, 28);
			this.rbtAllBytes.TabIndex = 23;
			this.rbtAllBytes.Text = "All Bytes";
			this.rbtAllBytes.CheckedChanged += new System.EventHandler(this.scope_CheckedChanged);
			// 
			// lblExecuteLocation
			// 
			this.lblExecuteLocation.AutoSize = true;
			this.lblExecuteLocation.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblExecuteLocation.Location = new System.Drawing.Point(104, 80);
			this.lblExecuteLocation.Name = "lblExecuteLocation";
			this.lblExecuteLocation.Size = new System.Drawing.Size(71, 22);
			this.lblExecuteLocation.TabIndex = 28;
			this.lblExecuteLocation.Text = "Location";
			this.lblExecuteLocation.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// tbxDepthLocation
			// 
			this.tbxDepthLocation.Enabled = false;
			this.tbxDepthLocation.Location = new System.Drawing.Point(176, 80);
			this.tbxDepthLocation.Name = "tbxDepthLocation";
			this.tbxDepthLocation.Size = new System.Drawing.Size(67, 22);
			this.tbxDepthLocation.TabIndex = 48;
			this.tbxDepthLocation.Text = "0";
			this.tbxDepthLocation.Validating += new System.ComponentModel.CancelEventHandler(this.tbxDepthLocation_Validating);
			// 
			// cbxStripFileExt
			// 
			this.cbxStripFileExt.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.cbxStripFileExt.Location = new System.Drawing.Point(312, 8);
			this.cbxStripFileExt.Name = "cbxStripFileExt";
			this.cbxStripFileExt.Size = new System.Drawing.Size(144, 24);
			this.cbxStripFileExt.TabIndex = 44;
			this.cbxStripFileExt.Text = "Strip File Extension";
			// 
			// cbxExcludeTargetDir
			// 
			this.cbxExcludeTargetDir.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.cbxExcludeTargetDir.Location = new System.Drawing.Point(472, 8);
			this.cbxExcludeTargetDir.Name = "cbxExcludeTargetDir";
			this.cbxExcludeTargetDir.Size = new System.Drawing.Size(176, 24);
			this.cbxExcludeTargetDir.TabIndex = 45;
			this.cbxExcludeTargetDir.Text = "Exclude Target Directory";
			// 
			// btnClearLog
			// 
			this.btnClearLog.Location = new System.Drawing.Point(584, 424);
			this.btnClearLog.Name = "btnClearLog";
			this.btnClearLog.Size = new System.Drawing.Size(90, 26);
			this.btnClearLog.TabIndex = 46;
			this.btnClearLog.Text = "Clear Log";
			this.btnClearLog.Click += new System.EventHandler(this.btnClearLog_Click);
			// 
			// tbxCount
			// 
			this.tbxCount.Font = new System.Drawing.Font("Microsoft Sans Serif", 34F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.tbxCount.Location = new System.Drawing.Point(488, 240);
			this.tbxCount.Name = "tbxCount";
			this.tbxCount.ReadOnly = true;
			this.tbxCount.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
			this.tbxCount.Size = new System.Drawing.Size(168, 72);
			this.tbxCount.TabIndex = 47;
			this.tbxCount.Text = "0";
			// 
			// tclMain
			// 
			this.tclMain.Appearance = System.Windows.Forms.TabAppearance.FlatButtons;
			this.tclMain.Controls.Add(this.create);
			this.tclMain.Controls.Add(this.execute);
			this.tclMain.HotTrack = true;
			this.tclMain.Location = new System.Drawing.Point(0, 40);
			this.tclMain.Name = "tclMain";
			this.tclMain.SelectedIndex = 0;
			this.tclMain.Size = new System.Drawing.Size(681, 384);
			this.tclMain.SizeMode = System.Windows.Forms.TabSizeMode.Fixed;
			this.tclMain.TabIndex = 49;
			// 
			// create
			// 
			this.create.Controls.Add(this.tbxErrorCreate);
			this.create.Controls.Add(this.lblTargetDirectory);
			this.create.Controls.Add(this.tbxTargetDirectory);
			this.create.Controls.Add(this.btnTargetDirectory);
			this.create.Controls.Add(this.btnCreateFiles);
			this.create.Controls.Add(this.gbxTarget);
			this.create.Controls.Add(this.lblSourceFile);
			this.create.Controls.Add(this.tbxSourceFile);
			this.create.Controls.Add(this.btnSourceFile);
			this.create.Controls.Add(this.gbxScope);
			this.create.Location = new System.Drawing.Point(4, 28);
			this.create.Name = "create";
			this.create.Size = new System.Drawing.Size(673, 352);
			this.create.TabIndex = 0;
			this.create.Text = "Create";
			// 
			// tbxErrorCreate
			// 
			this.tbxErrorCreate.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.tbxErrorCreate.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.tbxErrorCreate.ForeColor = System.Drawing.Color.Red;
			this.tbxErrorCreate.Location = new System.Drawing.Point(24, 144);
			this.tbxErrorCreate.Name = "tbxErrorCreate";
			this.tbxErrorCreate.ReadOnly = true;
			this.tbxErrorCreate.Size = new System.Drawing.Size(528, 15);
			this.tbxErrorCreate.TabIndex = 44;
			this.tbxErrorCreate.Text = "";
			// 
			// execute
			// 
			this.execute.Controls.Add(this.tbxErrorExecute);
			this.execute.Controls.Add(this.tbxFinishFile);
			this.execute.Controls.Add(this.tbxStartFile);
			this.execute.Controls.Add(this.lblFinishFile);
			this.execute.Controls.Add(this.lblStartFile);
			this.execute.Controls.Add(this.lblCount);
			this.execute.Controls.Add(this.tbxExecuteArgs);
			this.execute.Controls.Add(this.tbxExecuteApp);
			this.execute.Controls.Add(this.btnExecute);
			this.execute.Controls.Add(this.btnExecuteAppSelect);
			this.execute.Controls.Add(this.btnKillAll);
			this.execute.Controls.Add(this.lblExecuteArgs);
			this.execute.Controls.Add(this.lblExecuteApp);
			this.execute.Controls.Add(this.tbxCount);
			this.execute.Controls.Add(this.lblMilliseconds);
			this.execute.Controls.Add(this.tbxMilliseconds);
			this.execute.Location = new System.Drawing.Point(4, 28);
			this.execute.Name = "execute";
			this.execute.Size = new System.Drawing.Size(673, 352);
			this.execute.TabIndex = 1;
			this.execute.Text = "Execute";
			// 
			// tbxErrorExecute
			// 
			this.tbxErrorExecute.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.tbxErrorExecute.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.tbxErrorExecute.ForeColor = System.Drawing.Color.Red;
			this.tbxErrorExecute.Location = new System.Drawing.Point(16, 240);
			this.tbxErrorExecute.Name = "tbxErrorExecute";
			this.tbxErrorExecute.ReadOnly = true;
			this.tbxErrorExecute.Size = new System.Drawing.Size(448, 15);
			this.tbxErrorExecute.TabIndex = 53;
			this.tbxErrorExecute.Text = "";
			// 
			// tbxFinishFile
			// 
			this.tbxFinishFile.Location = new System.Drawing.Point(120, 176);
			this.tbxFinishFile.Name = "tbxFinishFile";
			this.tbxFinishFile.Size = new System.Drawing.Size(57, 22);
			this.tbxFinishFile.TabIndex = 52;
			this.tbxFinishFile.Text = "10";
			this.tbxFinishFile.Validating += new System.ComponentModel.CancelEventHandler(this.tbxFinishFile_Validating);
			// 
			// tbxStartFile
			// 
			this.tbxStartFile.Location = new System.Drawing.Point(120, 144);
			this.tbxStartFile.Name = "tbxStartFile";
			this.tbxStartFile.Size = new System.Drawing.Size(57, 22);
			this.tbxStartFile.TabIndex = 51;
			this.tbxStartFile.Text = "0";
			this.tbxStartFile.Validating += new System.ComponentModel.CancelEventHandler(this.tbxStartFile_Validating);
			// 
			// lblFinishFile
			// 
			this.lblFinishFile.AutoSize = true;
			this.lblFinishFile.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblFinishFile.Location = new System.Drawing.Point(16, 176);
			this.lblFinishFile.Name = "lblFinishFile";
			this.lblFinishFile.Size = new System.Drawing.Size(84, 22);
			this.lblFinishFile.TabIndex = 50;
			this.lblFinishFile.Text = "Finish File";
			this.lblFinishFile.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// lblStartFile
			// 
			this.lblStartFile.AutoSize = true;
			this.lblStartFile.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblStartFile.Location = new System.Drawing.Point(16, 144);
			this.lblStartFile.Name = "lblStartFile";
			this.lblStartFile.Size = new System.Drawing.Size(75, 22);
			this.lblStartFile.TabIndex = 49;
			this.lblStartFile.Text = "Start File";
			this.lblStartFile.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// lblCount
			// 
			this.lblCount.AutoSize = true;
			this.lblCount.Enabled = false;
			this.lblCount.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblCount.Location = new System.Drawing.Point(384, 272);
			this.lblCount.Name = "lblCount";
			this.lblCount.Size = new System.Drawing.Size(93, 37);
			this.lblCount.TabIndex = 48;
			this.lblCount.Text = "Count";
			this.lblCount.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
			// 
			// mnuFile
			// 
			this.mnuFile.Index = 0;
			this.mnuFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mnuFileExit});
			this.mnuFile.Text = "&File";
			// 
			// mnuFileExit
			// 
			this.mnuFileExit.Index = 0;
			this.mnuFileExit.Text = "E&xit";
			this.mnuFileExit.Click += new System.EventHandler(this.mnuFileExit_Click);
			// 
			// mnuEdit
			// 
			this.mnuEdit.Index = 1;
			this.mnuEdit.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					this.mnuEditCopy});
			this.mnuEdit.Text = "&Edit";
			// 
			// mnuEditCopy
			// 
			this.mnuEditCopy.Index = 0;
			this.mnuEditCopy.Text = "&Copy";
			this.mnuEditCopy.Click += new System.EventHandler(this.mnuEditCopy_Click);
			// 
			// mainMenu
			// 
			this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					 this.mnuFile,
																					 this.mnuEdit});
			// 
			// errorProvider
			// 
			this.errorProvider.ContainerControl = this;
			// 
			// frmFileFuzz
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(6, 15);
			this.ClientSize = new System.Drawing.Size(681, 672);
			this.Controls.Add(this.tclMain);
			this.Controls.Add(this.btnClearLog);
			this.Controls.Add(this.cbxExcludeTargetDir);
			this.Controls.Add(this.cbxStripFileExt);
			this.Controls.Add(this.pbrProgressBar);
			this.Controls.Add(this.lblFileType);
			this.Controls.Add(this.cbxFileType);
			this.Controls.Add(this.rtbLog);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Menu = this.mainMenu;
			this.Name = "frmFileFuzz";
			this.Text = "FileFuzz";
			this.Load += new System.EventHandler(this.frmFileFuzz_Load);
			this.gbxTarget.ResumeLayout(false);
			this.gbxScope.ResumeLayout(false);
			this.tclMain.ResumeLayout(false);
			this.create.ResumeLayout(false);
			this.execute.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			//Universal exception handler
			Application.ThreadException += new ThreadExceptionEventHandler(
				Application_ThreadException);
			
			Application.Run(new frmFileFuzz());
		}

		private void btnSourceFile_Click(object sender, System.EventArgs e)
		{
			if(ofdSourceFile.ShowDialog() == DialogResult.OK) 
			{
				tbxSourceFile.Text = ofdSourceFile.FileName;
			}
		}

		private void btnTargetDirectory_Click(object sender, System.EventArgs e)
		{
			if(fbdTargetDirectory.ShowDialog() == DialogResult.OK) 
			{
				tbxTargetDirectory.Text = fbdTargetDirectory.SelectedPath;
			}
		}

		private void btnCreateFiles_Click(object sender, System.EventArgs e)
		{
			Read readFile = new Read(tbxSourceFile.Text);
			if (rbtRegex.Checked == true)
			{
				if (readFile.readAscii() == false)	//End if source file not found)
					return;
			}
			else if (rbtAllBytes.Checked == true || rbtRange.Checked == true || rbtDepth.Checked == true)
			{	
				if (readFile.readBinary() == false)	//End if source file not found
					return;
			}
			int startByte = 0;
			int finishByte = 0;
			byte bytesToOverwrite;
			string targetDirectory = tbxTargetDirectory.Text;
			string fileExtension = tbxSourceFile.Text.Substring(tbxSourceFile.Text.LastIndexOf("."));
			int bytesToRepeat = Convert.ToInt32(tbxTargetBytesNumber.Text);

			Write writeFile = null;

			int start = 0;
			int finish = 0;

			int fileNumber = 0;

			//All bytes
			if (rbtAllBytes.Checked == true)
			{
				bytesToOverwrite = Convert.ToByte(tbxTargetByte.Text, 16);
				finishByte = readFile.sourceArray.Length-1;
				finish = finishByte;
				writeFile = new Write(readFile.sourceArray, finishByte, bytesToOverwrite, targetDirectory, fileExtension, bytesToRepeat);
				
				//Register progress bar event handlers
				writeFile.pbrStart += new pbrProgressBarStart(pbrHandleStart);
				writeFile.pbrUpdate  += new pbrProgressBarUpdate(pbrHandleUpdate);
				writeFile.writeByte();
			}
			
			//Range of Bytes
			else if (rbtRange.Checked == true)
			{
				bytesToOverwrite = Convert.ToByte(tbxTargetByte.Text, 16);
				startByte = Convert.ToInt32(tbxRangeStart.Text);
				start = startByte;
				finishByte = Convert.ToInt32(tbxRangeFinish.Text);
				finish = finishByte;
				writeFile = new Write(readFile.sourceArray, startByte, finishByte, bytesToOverwrite, targetDirectory, fileExtension, bytesToRepeat);
				
				//Register progress bar event handlers
				writeFile.pbrStart += new pbrProgressBarStart(pbrHandleStart);
				writeFile.pbrUpdate  += new pbrProgressBarUpdate(pbrHandleUpdate);
				writeFile.writeByte();
			}
			
			//Depth of Bytes
			else if (rbtDepth.Checked == true)
			{
				startByte = Convert.ToInt32(tbxDepthLocation.Text);
				start = (int) Convert.ToInt32((tbxTargetByte.Text), 16);
				finish = (int) Convert.ToInt32((tbxTargetByteFinish.Text), 16);
				writeFile = new Write(readFile.sourceArray, startByte, targetDirectory, fileExtension, start, finish);
				
				//Register progress bar event handlers
				writeFile.pbrStart += new pbrProgressBarStart(pbrHandleStart);
				writeFile.pbrUpdate  += new pbrProgressBarUpdate(pbrHandleUpdate);
				for (int loop = start; loop <= finish; loop++)
				{
					//Make the current byte value the same length as the
					//largest value to ensure that the same range of
					//bytes are overwritten
					string currentByte = String.Format("{0:X" + tbxTargetByteFinish.Text.Length + "}", loop);
					//Pad the first character with a '0' if it has a null value
					if (currentByte.Length % 2 != 0)
						currentByte = currentByte.PadLeft(currentByte.Length+1, '0');
					writeFile.writeByte(strToByteArray(currentByte));
					fileNumber++;
				}
			}
			else if (rbtRegex.Checked == true)
			{
				Regex regex = new Regex(tbxRegexFind.Text);
				int matches = regex.Matches(readFile.sourceString, 0).Count;
				string replace = null;
				string source;
				int stringGrowth = Convert.ToInt32(tbxRegexReplaceCount.Text);
				int location = 0;
				int createString = Convert.ToInt32(tbxRegexReplaceMultiplier.Text);
				for (int countCreateString = 1; countCreateString <= createString; countCreateString++)
				{
					replace += tbxRegexReplace.Text;
				}
	
				for (int count = 1; count <= matches; count++)
				{
					string insert = tbxRegexFind.Text;
					for (int countStringGrowth = 1; countStringGrowth <= stringGrowth; countStringGrowth++)
					{
						insert += replace;
						source = regex.Replace(readFile.sourceString, insert, 1, location);
						writeFile = new Write(source, targetDirectory, fileExtension, 0, (matches * countStringGrowth), fileNumber);
						writeFile.writeAscii();
						fileNumber++;
					}
					location = regex.Match(readFile.sourceString, location).Index;
					location++;
				}
			}

			//rtbLog.Clear();
			if (rbtRegex.Checked == true)
			{
				rtbLog.AppendText(readFile.sourceString.Length.ToString());
				rtbLog.AppendText(" characters read.\n");
				rtbLog.AppendText((fileNumber).ToString());
				rtbLog.AppendText(" files written to disk.\n\n");
			}
			else if (rbtDepth.Checked == true)
			{	
				rtbLog.AppendText(readFile.sourceArray.Length.ToString());
				rtbLog.AppendText(" bytes read.\n");
				rtbLog.AppendText((fileNumber).ToString());
				rtbLog.AppendText(" files written to disk.\n\n");
			}
			else if (rbtAllBytes.Checked == true || rbtRange.Checked == true)
			{	
				rtbLog.AppendText(readFile.sourceArray.Length.ToString());
				rtbLog.AppendText(" bytes read.\n");
				rtbLog.AppendText((finish+1).ToString());
				rtbLog.AppendText(" files written to disk.\n\n");
			}
		}

		private void btnExecute_Click(object sender, System.EventArgs e)
		{
			int startFile = Convert.ToInt32(tbxStartFile.Text);
			int finishFile = Convert.ToInt32(tbxFinishFile.Text);
			string targetDirectory = tbxTargetDirectory.Text;
			string fileExtension;
			if (cbxStripFileExt.Checked == true)
				fileExtension = null;
			else
				fileExtension = tbxSourceFile.Text.Substring(tbxSourceFile.Text.LastIndexOf("."));
			int killTimer;
			string applicationName = tbxExecuteApp.Text;
			string applicationArguments = tbxExecuteArgs.Text;

			try
			{
				FileInfo [] targetFiles = null;
				DirectoryInfo targetDirectoryInfo = new DirectoryInfo(tbxTargetDirectory.Text);
				targetFiles = targetDirectoryInfo.GetFiles();
			}
			catch (System.IO.DirectoryNotFoundException ex)
			{
				MessageBox.Show(ex.Message, "Error - Directory not found");
				return;
			}

			try
			{
				killTimer = Convert.ToInt32(tbxMilliseconds.Text);
			}
			catch (System.FormatException ex)
			{
				MessageBox.Show(ex.Message, "Error - Invalid format for milliseconds");
				return;
			}


			Execute executeApplication = null;

			if (cbxExcludeTargetDir.Checked == true)
				targetDirectory = null;

			executeApplication = new Execute(startFile, finishFile, targetDirectory, fileExtension, killTimer, applicationName, applicationArguments);
			executeApplication.pbrStart += new pbrProgressBarStart(pbrHandleStart);
			executeApplication.pbrUpdate  += new pbrProgressBarUpdate(pbrHandleUpdate);
			executeApplication.tbxUpdate += new tbxCountUpdate(tbxHandleUpdate);
			executeApplication.rtbLog += new rtbLogOutput(rtbHandleLog);
			
			//Execute application in new thread
			Thread executeAppThread = 
				new Thread(new ThreadStart(executeApplication.executeApp));
			executeAppThread.Start();
		}

		private void btnExecuteAppSelect_Click(object sender, System.EventArgs e)
		{
			if(ofdExecuteApp.ShowDialog() == DialogResult.OK) 
			{
				tbxExecuteApp.Text = ofdExecuteApp.FileName;
			} 
		}

		private void cbxFileType_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			updateFileRange();
			
			fileName = xmlAudits.dtDataFile.Rows[cbxFileType.SelectedIndex][0].ToString();
			fileDescription = xmlAudits.dtDataFile.Rows[cbxFileType.SelectedIndex][1].ToString();
			sourceFile = xmlAudits.dtDataSource.Rows[cbxFileType.SelectedIndex][0].ToString();
			sourceDir = xmlAudits.dtDataSource.Rows[cbxFileType.SelectedIndex][1].ToString();
			appName = xmlAudits.dtDataApp.Rows[cbxFileType.SelectedIndex][0].ToString();
			appDescription = xmlAudits.dtDataApp.Rows[cbxFileType.SelectedIndex][1].ToString();
			appAction = xmlAudits.dtDataApp.Rows[cbxFileType.SelectedIndex][2].ToString();
			appLaunch = xmlAudits.dtDataApp.Rows[cbxFileType.SelectedIndex][3].ToString();
			appFlags = xmlAudits.dtDataApp.Rows[cbxFileType.SelectedIndex][4].ToString();
			targetDir = xmlAudits.dtDataTarget.Rows[cbxFileType.SelectedIndex][0].ToString();

			tbxSourceFile.Text = sourceDir + sourceFile;
			tbxTargetDirectory.Text = targetDir;
			tbxExecuteApp.Text = appLaunch;
			tbxExecuteArgs.Text = appFlags;

			rtbLog.AppendText("File Name: " + fileName + "\n");
			rtbLog.AppendText("File Description: " + fileDescription + "\n");
			rtbLog.AppendText("Source File: " + sourceFile + "\n");
			rtbLog.AppendText("Source Directory: " + sourceDir + "\n");
			rtbLog.AppendText("Application Name: " + appName + "\n");
			rtbLog.AppendText("Application Description: " + appDescription + "\n");
			rtbLog.AppendText("Application Action: " + appAction + "\n");
			rtbLog.AppendText("Application Launch: " + appLaunch + "\n");
			rtbLog.AppendText("Application Flags: " + appFlags + "\n");
			rtbLog.AppendText("Target Directory: " + targetDir + "\n\n");
		}

		private void btnKill_Click(object sender, System.EventArgs e)
		{

			try
			{
				killProcess(fileName.Substring(0, fileName.LastIndexOf(".")));
			}
			catch (System.ArgumentOutOfRangeException ex)
			{
				MessageBox.Show(ex.Message, "Error - No processes to kill");
			}
		}

		private void killProcess(string processName)
		{
			Process myproc = new Process();
			
			//Get all instances of proc that are open, attempt to close them.
			foreach (Process thisproc in Process.GetProcessesByName(processName)) 
			{
				if(!thisproc.CloseMainWindow())
				{
					//If closing is not successful or no desktop window handle, then force termination.
					thisproc.Kill();
				}
			} // next proc
		}

		private void scope_CheckedChanged(object sender, System.EventArgs e)
		{
			updateFileRange();

			if (rbtAllBytes.Checked == true)
			{
				tbxTargetByte.Enabled = true;
				tbxRangeStart.Enabled = false;
				tbxRangeFinish.Enabled = false;
				tbxTargetByteFinish.Enabled = false;
				tbxDepthLocation.Enabled = false;
				tbxRegexFind.Enabled = false;
				tbxRegexReplace.Enabled = false;
				tbxRegexReplaceMultiplier.Enabled = false;
				tbxRegexReplaceCount.Enabled = false;
				tbxTargetBytesNumber.Enabled = true;
			}
			else if (rbtRange.Checked == true)
			{
				tbxTargetByte.Enabled = true;
				tbxRangeStart.Enabled = true;
				tbxRangeFinish.Enabled = true;
				tbxTargetByteFinish.Enabled = false;
				tbxDepthLocation.Enabled = false;
				tbxRegexFind.Enabled = false;
				tbxRegexReplace.Enabled = false;
				tbxRegexReplaceMultiplier.Enabled = false;
				tbxRegexReplaceCount.Enabled = false;
				tbxTargetBytesNumber.Enabled = true;
			}
			else if (rbtDepth.Checked == true)
			{
				tbxTargetByte.Enabled = true;
				tbxRangeStart.Enabled = false;
				tbxRangeFinish.Enabled = false;
				tbxTargetByteFinish.Enabled = true;
				tbxDepthLocation.Enabled = true;
				tbxRegexFind.Enabled = false;
				tbxRegexReplace.Enabled = false;
				tbxRegexReplaceMultiplier.Enabled = false;
				tbxRegexReplaceCount.Enabled = false;
				tbxTargetBytesNumber.Enabled = false;
			}
			else if (rbtRegex.Checked == true)
			{
				tbxTargetByte.Enabled = false;
				tbxRangeStart.Enabled = false;
				tbxRangeFinish.Enabled = false;
				tbxTargetByteFinish.Enabled = false;
				tbxDepthLocation.Enabled = false;
				tbxRegexFind.Enabled = true;
				tbxRegexReplace.Enabled = true;
				tbxRegexReplaceMultiplier.Enabled = true;
				tbxRegexReplaceCount.Enabled = true;
				tbxTargetBytesNumber.Enabled = false;
			}
		}

		private void frmFileFuzz_Load(object sender, System.EventArgs e)
		{
			xmlAudits = new ReadXml("targets.xml", "targets.xsd");
			xmlAudits.bindData();

			//Bind the combobox to state abreviations.
			cbxFileType.DataSource = xmlAudits.dtDataTest;
			cbxFileType.DisplayMember = "name";
			rtbLog.Clear();
		}

		public static void Application_ThreadException(
			object sender, ThreadExceptionEventArgs ex)
		{
			MessageBox.Show("Error: " + ex.ToString(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
		}

		//Update progress bar
		private void pbrHandleStart(int startInput, int totalInput)
		{
			this.pbrProgressBar.Minimum = startInput;
			this.pbrProgressBar.Maximum = totalInput;
		}

		private void pbrHandleUpdate(int incrementInput)
		{
			this.pbrProgressBar.Value = incrementInput;
		}

		//Update counter
		private void tbxHandleUpdate(int valueInput)
		{
			this.tbxCount.Text = valueInput.ToString();
		}

		//Write data to the rich text box log
		private void rtbHandleLog(string logInput)
		{
			this.rtbLog.AppendText(logInput);
		}

		private void btnClearLog_Click(object sender, System.EventArgs e)
		{
			rtbLog.Clear();
		}

		/// <summary>
		/// Converts hexadecimal string into byte array
		/// </summary>
		/// <param name="str">hexadecimal string</param>
		/// <returns>byte []</returns>
		private static byte[] strToByteArray(string str)
		{
			int lengthByteArray = str.Length/2;
			int numberByte = 0;
			byte [] byteArray = new byte[lengthByteArray];
			for (int loop = 0; loop < str.Length; loop+=2)
			{
				byteArray[numberByte] = hexToByte(str.Substring(loop, 2));
				numberByte++;
			}
			return  byteArray;
		}
		
		/// <summary>
		/// Converts 1 or 2 character string into equivalant byte value
		/// </summary>
		/// <param name="hex">1 or 2 character string</param>
		/// <returns>byte</returns>
		private static byte hexToByte(string hex)
		{
			if (hex.Length > 2 || hex.Length <= 0)
				throw new ArgumentException("Hex must be 1 or 2 characters in length.");
			byte newByte = byte.Parse(hex, System.Globalization.NumberStyles.HexNumber);
			return newByte;
		}

		private void mnuFileExit_Click(object sender, System.EventArgs e)
		{
			Application.Exit();
		}

		private void mnuEditCopy_Click(object sender, System.EventArgs e)
		{
			rtbLog.SelectAll();
			rtbLog.Copy();
		}

		private void updateFileRange()
		{
			if (rbtAllBytes.Checked == true)
			{
				//Determine total number of files in directory
				try
				{
					FileInfo [] targetFiles = null;
					DirectoryInfo targetDirectoryInfo = new DirectoryInfo(tbxTargetDirectory.Text);
					targetFiles = targetDirectoryInfo.GetFiles();
					
					//Change start and finish file numbers in Execute tab
					tbxStartFile.Text = "0";
					tbxFinishFile.Text = targetFiles.Length.ToString();
				}
				catch (System.IO.DirectoryNotFoundException ex)
				{
					tbxStartFile.Text = "0";
					tbxFinishFile.Text = "0";
				}
			}
			else if (rbtRange.Checked == true)
			{
				tbxStartFile.Text = tbxRangeStart.Text;
				tbxFinishFile.Text = tbxRangeFinish.Text;
			}
			else if (rbtDepth.Checked == true)
			{
				tbxStartFile.Text = Convert.ToInt32((tbxTargetByte.Text), 16).ToString();
				tbxFinishFile.Text = Convert.ToInt32((tbxTargetByteFinish.Text), 16).ToString();
			}
			else if (rbtRegex.Checked == true)
			{
				tbxStartFile.Text = "0";
				tbxFinishFile.Text = "0";
			}
		}

		private void tbxTargetByte_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateHex(tbxTargetByte.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxTargetByte.Select(0, tbxTargetByte.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxTargetByte, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxTargetByte, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
		}

		private void tbxTargetByteFinish_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateHex(tbxTargetByteFinish.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxTargetByteFinish.Select(0, tbxTargetByteFinish.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxTargetByteFinish, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxTargetByteFinish, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
		}

		private void tbxTargetBytesNumber_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxTargetBytesNumber.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxTargetBytesNumber.Select(0, tbxTargetByteFinish.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxTargetBytesNumber, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxTargetBytesNumber, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
		}

		private void tbxRangeStart_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxRangeStart.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxRangeStart.Select(0, tbxRangeStart.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxRangeStart, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxRangeStart, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
		}

		private void tbxRangeFinish_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxRangeFinish.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxRangeFinish.Select(0, tbxRangeFinish.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxRangeFinish, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxRangeFinish, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
		}

		private void tbxDepthLocation_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxDepthLocation.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxDepthLocation.Select(0, tbxDepthLocation.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxDepthLocation, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxDepthLocation, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
		}

		private void tbxRegexReplaceMultiplier_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxRegexReplaceMultiplier.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxRegexReplaceMultiplier.Select(0, tbxRegexReplaceMultiplier.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxRegexReplaceMultiplier, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxRegexReplaceMultiplier, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
		}

		private void tbxRegexReplaceCount_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxRegexReplaceCount.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxRegexReplaceCount.Select(0, tbxRegexReplaceCount.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxRegexReplaceCount, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxRegexReplaceCount, errorMsg);

				// Display the error message
				tbxErrorCreate.Text = errorMsg;
			}
		}

		private void tbxStartFile_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxStartFile.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxStartFile.Select(0, tbxStartFile.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxStartFile, errorMsg);

				// Display the error message
				tbxErrorExecute.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxStartFile, errorMsg);

				// Display the error message
				tbxErrorExecute.Text = errorMsg;
			}
		}

		private void tbxFinishFile_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxFinishFile.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxFinishFile.Select(0, tbxFinishFile.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxFinishFile, errorMsg);

				// Display the error message
				tbxErrorExecute.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxFinishFile, errorMsg);

				// Display the error message
				tbxErrorExecute.Text = errorMsg;
			}
		}

		private void tbxMilliseconds_Validating(object sender, System.ComponentModel.CancelEventArgs e)
		{
			string errorMsg;
			errorMsg = validateDigit(tbxMilliseconds.Text);
			if (errorMsg != "")
			{
				e.Cancel = true;
				tbxMilliseconds.Select(0, tbxMilliseconds.Text.Length);

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxMilliseconds, errorMsg);

				// Display the error message
				tbxErrorExecute.Text = errorMsg;
			}
			else
			{
				e.Cancel = false;

				// Set the ErrorProvider error with the text to display. 
				this.errorProvider.SetError(tbxMilliseconds, errorMsg);

				// Display the error message
				tbxErrorExecute.Text = errorMsg;
			}
		}
		
		private string validateHex(string hexString)
		{
			if (hexString == "")
				return "Byte value cannot be blank.";
			
			char [] hexChars = hexString.ToCharArray();
			foreach(char hexChar in hexChars)
			{
				if (Convert.ToInt32(hexChar) >= 0 && Convert.ToInt32(hexChar) <= 47)
					return "Byte value must be in proper hex format";
				else if (Convert.ToInt32(hexChar) >= 58 && Convert.ToInt32(hexChar) <= 64)
					return "Byte value must be in proper hex format";
				else if (Convert.ToInt32(hexChar) >= 71 && Convert.ToInt32(hexChar) <= 96)
					return "Byte value must be in proper hex format";
				else if	(Convert.ToInt32(hexChar) >= 103)
					return "Byte value must be in proper hex format";
			}
			return "";
		}

		private string validateDigit(string digitString)
		{
			if (digitString == "")
				return "Numerical value cannot be blank.";
			
			char [] digitChars = digitString.ToCharArray();
			foreach(char digitChar in digitChars)
			{
				if (!char.IsDigit(digitChar))
					return "Field must contain numerical value.";
			}

			return "";
		}
	}
}