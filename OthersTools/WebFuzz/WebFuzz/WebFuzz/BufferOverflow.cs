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
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WebFuzz
{
    public partial class BufferOverflow : Form
    {
        string fillText;
        int length;
        int multiplier;

        //Delegate to update Overflow variables
        public delegate void updateOverflow(string fillText, int length, int multiplier);
        public static event updateOverflow addOverflow;

        public BufferOverflow()
        {
            InitializeComponent();    
        }

        private void btnOverflowSubmit_Click(object sender, EventArgs e)
        {
            fillText = tbxFillText.Text;
            length = Convert.ToInt32(tbxLength.Text);
            multiplier = Convert.ToInt32(tbxMultiplier.Text);

            addOverflow(fillText, length, multiplier);
            Dispose();
        }

    }
}