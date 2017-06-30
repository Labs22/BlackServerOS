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
    public partial class BasicAuthentication : Form
    {
        string username;
        string password;
        
        //Delegate to update Baic Authentication Header
        public delegate void updateBasicAuth(string basicAuth);
        public static event updateBasicAuth addBasicAuth; 

        public BasicAuthentication()
        {
            InitializeComponent();
        }

        public void encode()
        {
            byte[] authBytes = System.Text.Encoding.ASCII.GetBytes(username + ":" + password);
            String encodedString = Convert.ToBase64String(authBytes);
            String authString = "Authorization: Basic " + encodedString;
            addBasicAuth(authString);
            return;
        }

        private void btnBasicAuthentication_Click(object sender, EventArgs e)
        {
            username = tbxUsername.Text;
            password = tbxPassword.Text;
            
            encode();
            Dispose();
        }
    }
}