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

namespace FileFuzz
{
	/// <summary>
	/// Read source files.
	/// </summary>
	public class Write
	{
		private int repeatBytes;
		private int startByte = 0;
		private int finishByte;
		private byte [] sourceArray;
		private string text;
		private byte bytesToWrite;
		private string targetDirectory;
		private string fileExtension;
		private int startFile = 0;
		private int finishFile = 0;
		private int fileNumber = 0;

		public event FileFuzz.frmFileFuzz.pbrProgressBarStart pbrStart;
		public event FileFuzz.frmFileFuzz.pbrProgressBarUpdate pbrUpdate;

		//All bytes
		public Write(byte [] sourceArrayInput, int finishByteInput, byte bytesToWriteInput, string targetDirectoryInput, string fileExtensionInput, int repeatBytesInput)
		{
			sourceArray = sourceArrayInput;
			finishByte = finishByteInput;
			bytesToWrite = bytesToWriteInput;
			targetDirectory = targetDirectoryInput;
			fileExtension = fileExtensionInput;
			repeatBytes = repeatBytesInput;
			finishFile = finishByte;
		}

		//Range of bytes
		public Write(byte [] sourceArrayInput, int startByteInput, int finishByteInput, byte bytesToWriteInput, string targetDirectoryInput, string fileExtensionInput, int repeatBytesInput)
		{
			sourceArray = sourceArrayInput;
			startByte = startByteInput;
			finishByte = finishByteInput;
			bytesToWrite = bytesToWriteInput;
			targetDirectory = targetDirectoryInput;
			fileExtension = fileExtensionInput;
			repeatBytes = repeatBytesInput;
			startFile = startByte;
			finishFile = finishByte;
		}

		//Depth
		public Write(byte [] sourceArrayInput, int startByteInput, string targetDirectoryInput, string fileExtensionInput, int startFileInput, int finishFileInput)
		{
			sourceArray = sourceArrayInput;
			startByte = startByteInput;
			targetDirectory = targetDirectoryInput;
			fileExtension = fileExtensionInput;
			startFile = startFileInput;
			finishFile = finishFileInput;
		}

		//Regex
		public Write(string textInput, string targetDirectoryInput, string fileExtensionInput, int startFileInput, int finishFileInput, int fileNumberInput)
		{
			text = textInput;
			targetDirectory = targetDirectoryInput;
			fileExtension = fileExtensionInput;
			startFile = startFileInput;
			finishFile = finishFileInput;
			fileNumber = fileNumberInput;
		}

		public void writeByte()
		{
			//Initialize progress bar
			if (this.pbrStart != null)
			{
				this.pbrStart(startFile, finishFile);
			}
			
			for (int createLoop = startByte; createLoop <= finishByte; createLoop++)
			{
				string targetFile = targetDirectory + @"\" + createLoop + fileExtension;
				DirectoryInfo targetDirInfo = new DirectoryInfo(targetDirectory);
				if (targetDirInfo.Exists == false)
					targetDirInfo.Create();
				byte [] targetBytesOld = new byte [repeatBytes];

				//Overwrite bytes
				for (int byteLoop = 0; byteLoop < repeatBytes; byteLoop++)
				{
					
					if (createLoop + byteLoop < sourceArray.Length)
					{
						targetBytesOld[byteLoop] = sourceArray[createLoop + byteLoop];
						sourceArray[createLoop + byteLoop] = bytesToWrite;
					}
				}

				try
				{
					using(BinaryWriter bwTargetFile = 
							  new BinaryWriter(File.Open(targetFile, FileMode.Create)))
					{
						bwTargetFile.Write(sourceArray);
						bwTargetFile.Close();
					}
				}
				catch (System.IO.IOException ex)
				{
					MessageBox.Show(ex.Message, "Error - File cannot be accessed");
				}

				//Replace overwritten bytes
				for (int byteLoop = 0; byteLoop < repeatBytes; byteLoop++)
				{
					if (createLoop + byteLoop < sourceArray.Length)
						sourceArray[createLoop + byteLoop] = targetBytesOld[byteLoop];
				}
				//Update progress bar
				if (this.pbrUpdate != null)
				{
					this.pbrUpdate(createLoop);
				}
			}
			//Clear the progress bar
			if (this.pbrStart != null)
			{
				this.pbrStart(0, 0);
			}
		}

		public void writeByte(byte [] byteInput)
		{
			//Initialize progress bar
			if (this.pbrStart != null)
			{
				this.pbrStart(startFile, finishFile);
			}
				
			string targetFile = targetDirectory + fileNumber + fileExtension;
			DirectoryInfo targetDirInfo = new DirectoryInfo(targetDirectory);
			if (targetDirInfo.Exists == false)
				targetDirInfo.Create();

			//Overwrite byte
			for (int loop = 0; loop <= byteInput.Length-1; loop++)
			{
				sourceArray[startByte+loop] = byteInput[loop];
			}

			using(BinaryWriter bwTargetFile = 
				new BinaryWriter(File.Open(targetFile, FileMode.Create)))
				{
					bwTargetFile.Write(sourceArray);
					bwTargetFile.Close();
				}
			
			//Update progress bar
			if (this.pbrUpdate != null)
			{
				this.pbrUpdate(fileNumber);
			}

			//Clear the progress bar
			if (this.pbrStart != null && (startFile + fileNumber) == finishFile)
			{
				this.pbrStart(0, 0);
			}

			fileNumber++;
		}

		public void writeAscii()
		{
			//Initialize progress bar
			if (this.pbrStart != null)
			{
				this.pbrStart(startFile, finishFile);
			}

			string targetFile = targetDirectory + fileNumber + fileExtension;
			DirectoryInfo targetDirInfo = new DirectoryInfo(targetDirectory);
			if (targetDirInfo.Exists == false)
				targetDirInfo.Create();

			using( StreamWriter swTargetFile =  
					  new StreamWriter(File.Open(targetFile, FileMode.Create)))
			{
				swTargetFile.Write(text);
				swTargetFile.Close();
			}

			//Update progress bar
			if (this.pbrUpdate != null)
			{
				this.pbrUpdate((int) fileNumber);
			}

			//Clear the progress bar
			if (this.pbrStart != null && fileNumber == finishFile)
			{
				this.pbrStart(0, 0);
			}
		}
	}
}