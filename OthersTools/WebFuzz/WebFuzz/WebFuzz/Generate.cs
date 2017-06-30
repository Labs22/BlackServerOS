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

namespace WebFuzz
{
    class Generate
    {
        ArrayList sourceArray = new ArrayList();

        string overflowString;
        int overflowSize;
        int overflowMultiplier;

        public Generate(string inputString, int inputSize, int inputMultiplier)
        {
            overflowString = inputString;
            overflowSize = inputSize;
            overflowMultiplier = inputMultiplier;
        }

        public ArrayList buildArray()
        {
            string fuzzVariable = "";
            for (int varCount = 1; varCount <= overflowSize; varCount++)
            {
                fuzzVariable += overflowString;
            }
            string fuzzString = "";
            for (int stringCount = 1; stringCount <= overflowMultiplier; stringCount++)
            {
                fuzzString += fuzzVariable;
                sourceArray.Add(fuzzString);
            }
            return sourceArray;
        }

    }
}
