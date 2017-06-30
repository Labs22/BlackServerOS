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
using System.Data;
using System.IO;
using System.Windows.Forms;
using System.Diagnostics;

namespace FileFuzz
{
	/// <summary>
	/// Read source files.
	/// </summary>
	public class Execute
	{
		private int startFile = 0;
		private int finishFile;
		private string targetDirectory;
		private string fileExtension;
		private int applicationTimer;
		private string executeAppName;
		private string executeAppArgs;
		private int procCount;

		public event FileFuzz.frmFileFuzz.pbrProgressBarStart pbrStart;
		public event FileFuzz.frmFileFuzz.pbrProgressBarUpdate pbrUpdate;
		public event FileFuzz.frmFileFuzz.tbxCountUpdate tbxUpdate;
		public event FileFuzz.frmFileFuzz.rtbLogOutput rtbLog;

		Process proc = new Process();
		
		public Execute(int startFileInput, int finishFileInput, string targetDirectoryInput, string fileExtensionInput, int applicationTimerInput, string executeAppNameInput, string executeAppArgsInput)
		{
			startFile = startFileInput;
			finishFile = finishFileInput;
			targetDirectory = targetDirectoryInput;
			fileExtension = fileExtensionInput;
			applicationTimer = applicationTimerInput;
			executeAppName = executeAppNameInput;
			executeAppArgs = executeAppArgsInput;
			procCount = startFile;
		}

		public void executeApp()
		{
			bool exceptionFound = false;

			//Initialize progress bar
			if (this.pbrStart != null)
			{
				this.pbrStart(startFile, finishFile);
			}

			while (procCount <= finishFile)
			{
				proc.StartInfo.CreateNoWindow = true;
				proc.StartInfo.UseShellExecute = false;
				proc.StartInfo.RedirectStandardOutput = true;
				proc.StartInfo.RedirectStandardError = true;
				proc.StartInfo.FileName = "crash.exe";
				proc.StartInfo.Arguments = executeAppName + " " + applicationTimer + " " + String.Format(executeAppArgs,  @targetDirectory + procCount.ToString() + fileExtension);
				proc.Start();
				//Update progress bar
				if (this.pbrUpdate != null)
				{
					this.pbrUpdate(procCount);
				}
				//Update counter
				if (this.tbxUpdate != null)
				{
					this.tbxUpdate(procCount);
				}
				proc.WaitForExit();

				//Write std output to rich text box log
				if (this.rtbLog != null && (proc.ExitCode == -1 || proc.ExitCode == 1))
				{
					this.rtbLog(proc.StandardOutput.ReadToEnd());
					this.rtbLog(proc.StandardError.ReadToEnd());
					exceptionFound = true;
				}
				procCount++;
			}
			//Clear the progress bar
			if (this.pbrStart != null)
			{
				this.pbrStart(0, 0);
			}
			//Clear the counter
			if (this.tbxUpdate != null)
			{
				this.tbxUpdate(0);
			}

			if (exceptionFound == false)
				this.rtbLog("No excpetions found\n\n");
			exceptionFound = false;
		}
	}
}