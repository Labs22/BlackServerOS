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
using System.Xml;

namespace FileFuzz
{
	/// <summary>
	/// Read source files.
	/// </summary>
	public class ReadXml
	{
		private string xmlFile;
		private string xmlSchema;
		private DataSet dsData;
		private XmlTextReader xtrSchema;

		public DataTable dtDataTest;
		public DataTable dtDataFile;
		public DataTable dtDataSource;
		public DataTable dtDataApp;
		public DataTable dtDataTarget;

		public ReadXml(string xmlFileInput, string xmlSchemaInput)
		{
			xmlFile = xmlFileInput;
			xmlSchema = xmlSchemaInput;
		}

		public void	bindData()
		{
			dsData = new DataSet();

			//Create file streams
			FileStream fsXml = new FileStream(xmlFile,FileMode.Open);
			FileStream fsXsd = new FileStream(xmlSchema,FileMode.Open);

			// Load the schema into the DataSet.
			xtrSchema = new XmlTextReader(fsXsd);
			dsData.ReadXmlSchema(xtrSchema);
			xtrSchema.Close();
			fsXsd.Close();

			// Load the data into the DataSet.
			XmlTextReader xtrXml = new XmlTextReader(fsXml);
			dsData.ReadXml(xtrXml);
			xtrXml.Close();
			fsXml.Close();

			// Get a DataTable to use for binding.
			dtDataTest = dsData.Tables["test"];
			dtDataFile = dsData.Tables["file"];
			dtDataSource = dsData.Tables["source"];
			dtDataApp = dsData.Tables["app"];
			dtDataTarget = dsData.Tables["target"];
		}
	}
}
			