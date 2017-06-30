/*
    WebFuzz
    Copyright (C) 2006 Michael Sutton <msutton@idefense.com, michaelawsutton@gmail.com>

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
using System.Collections;
using System.Text;
using System.IO;
using System.Windows.Forms;

namespace WebFuzz
{
    class Read
    {
        string sourceFile;
        
        private StreamReader sourceStream;
        string line;
        ArrayList sourceArray = new ArrayList(); 

        public Read(string fileName)
        {
            sourceFile = fileName;
            sourceStream = null;
            line = null;
        }

        public ArrayList readFile()
        {
            try
            {
                sourceStream = new StreamReader(File.Open(sourceFile, FileMode.Open));
            }
            catch (System.IO.FileNotFoundException ex)
            {
                MessageBox.Show(ex.Message, "Error - Source File Not Found");
                return null;
            }
            catch (System.IO.DirectoryNotFoundException ex)
            {
                MessageBox.Show(ex.Message, "Error - Source Directory Not Found");
                return null;
            }
            catch (System.IO.IOException ex)
            {
                MessageBox.Show(ex.Message, "Error - File cannot be accessed");
                return null;
            }
            catch (System.UnauthorizedAccessException ex)
            {
                MessageBox.Show(ex.Message, "Error - Access to the file is denied");
                return null;
            }

            line = sourceStream.ReadLine();
            while (line != null)
            {
                sourceArray.Add(line);
                line = sourceStream.ReadLine();
            }
            sourceStream.Close();

            return sourceArray;
        }
    }
}
