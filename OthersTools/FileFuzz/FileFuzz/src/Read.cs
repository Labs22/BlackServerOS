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
	public class Read
	{
		private BinaryReader brSourceFile;
		private StreamReader arSourceFile;
		public byte [] sourceArray;
		public string sourceString; 
		private int sourceCount;
		private string sourceFile;

		public Read(string fileName)
		{
			sourceFile = fileName;
			sourceArray = null;
			sourceString = null;
			sourceCount = 0;
		}
		
		public bool readBinary()
		{
			try
			{
				brSourceFile = new BinaryReader(File.Open(sourceFile, FileMode.Open));
			}
			catch (System.IO.FileNotFoundException ex)
			{
				MessageBox.Show(ex.Message, "Error - Source File Not Found");
				return false;
			}
			catch (System.IO.DirectoryNotFoundException ex)
			{
				MessageBox.Show(ex.Message, "Error - Source Directory Not Found");
				return false;
			}
			catch (System.IO.IOException ex)
			{
				MessageBox.Show(ex.Message, "Error - File cannot be accessed");
				return false;
			}
			catch (System.UnauthorizedAccessException ex)
			{
				MessageBox.Show(ex.Message, "Error - Access to the file is denied");
				return false;
			}

			sourceArray = new byte[brSourceFile.BaseStream.Length];

			try
			{
				while (brSourceFile.PeekChar() != -1)
				{
					sourceArray[sourceCount] = brSourceFile.ReadByte();
					sourceCount++;
				}
				brSourceFile.Close();
			}
			catch (System.ArgumentException ex)
			{
				MessageBox.Show(@"Count: " + sourceCount.ToString() + " " + ex.Message, "Error");
				return true;
			}
			return true;
		}

		public bool readAscii()
		{
			try
			{
				arSourceFile = new StreamReader(File.Open(sourceFile, FileMode.Open));
			}
			catch (System.IO.FileNotFoundException ex)
			{
				MessageBox.Show(ex.Message, "Error - Source File Not Found");
				return false;
			}
			catch (System.IO.DirectoryNotFoundException ex)
			{
				MessageBox.Show(ex.Message, "Error - Source Directory Not Found");
				return false;
			}
			catch (System.IO.IOException ex)
			{
				MessageBox.Show(ex.Message, "Error - File cannot be accessed");
				return false;
			}
			catch (System.UnauthorizedAccessException ex)
			{
				MessageBox.Show(ex.Message, "Error - Access to the file is denied");
				return false;
			}

			while (arSourceFile.Peek() != -1)
			{
				sourceString += Convert.ToChar(arSourceFile.Read()).ToString();
			}
			arSourceFile.Close();

			return true;
		}
	}
}