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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Timers;

//namespace testWebRequest
namespace WebFuzz
{
    public partial class frmWebFuzz : Form
    {
        //Arrays for requests and responses
        string[] requestsRaw = new string[1000];
        string[] responsesRaw = new string[1000];
        string[] responsesHtml = new string[1000];
        string[] responsesHost = new string[1000];
        string[] responsesPath = new string[1000];

        //Variables for Overflow fuzz type
        string overflowFill;
        int overflowLength;
        int overflowMultiplier;

        string defaultRequest = "GET / HTTP/1.1\nAccept: */*\nAccept-Language: en-us\nPragma: no-cache\nUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; InfoPath.1)\nHost: localhost\nProxy-Connection: Keep-Alive";

        private static ManualResetEvent ReadDone;

        //Timer for read timeouts
        System.Timers.Timer readTimeout = new System.Timers.Timer();

        private Sort lvwColumnSorter;
        
        public frmWebFuzz()
        {
            InitializeComponent();

            // Create an instance of a Sort column sorter and assign it
            // to the ListView control.
            lvwColumnSorter = new Sort();
            this.lvwResponses.ListViewItemSorter = lvwColumnSorter;

            //Register event handler for Basic Authentication menu option
            WebFuzz.BasicAuthentication.addBasicAuth += new WebFuzz.BasicAuthentication.updateBasicAuth(addHeader);

            WebFuzz.BufferOverflow.addOverflow += new WebFuzz.BufferOverflow.updateOverflow(addOverflow);
        }

        private void btnRequest_Click(object sender, EventArgs e)
        {
            //Strip URI handler from tbxHost if included
            string host = tbxHost.Text;
            if (host.Contains(@"://"))
                host = tbxHost.Text.Substring(tbxHost.Text.IndexOf(@"://") + 3);

            string request = "";

            string rawRequest = rtbRequestHeaders.Text;
            string fuzz = "";
            //Allow single requests with no fuzz variables
            if (rawRequest.Contains("[") != true || rawRequest.Contains("]") != true)
                rawRequest = "[None]" + rawRequest;
            //Parse raw request to identify fuzzing variables ([XXX])
            while (rawRequest.Contains("[") && rawRequest.Contains("]"))
            {
                fuzz = rawRequest.Substring(rawRequest.IndexOf('[') + 1, (rawRequest.IndexOf(']') - rawRequest.IndexOf('[')) - 1);

                int arrayCount = 0;
                int arrayEnd = 0;
                Read fuzzText = null;
                WebFuzz.Generate fuzzGenerate = null;
                ArrayList fuzzArray = null;
                string replaceString = "";

                string[] fuzzVariables = { "SQL", "XSS", "Methods", "Overflow", "Traversal", "Format" };

                switch (fuzz)
                {
                    case "SQL":
                        fuzzText = new Read("sqlinjection.txt");
                        fuzzArray = fuzzText.readFile();
                        arrayEnd = fuzzArray.Count;
                        replaceString = "[SQL]";
                        break;
                    case "XSS":
                        fuzzText = new Read("xssinjection.txt");
                        fuzzArray = fuzzText.readFile();
                        arrayEnd = fuzzArray.Count;
                        replaceString = "[XSS]";
                        break;
                    case "Methods":
                        fuzzText = new Read("methods.txt");
                        fuzzArray = fuzzText.readFile();
                        arrayEnd = fuzzArray.Count;
                        replaceString = "[Methods]";
                        break;
                    case "Overflow":
                        fuzzGenerate = new WebFuzz.Generate(overflowFill, overflowLength, overflowMultiplier);
                        fuzzArray = fuzzGenerate.buildArray();
                        arrayEnd = fuzzArray.Count;
                        replaceString = "[Overflow]";
                        break;
                    case "Traversal":
                        fuzzGenerate = new WebFuzz.Generate("../", 1, 10);
                        fuzzArray = fuzzGenerate.buildArray();
                        arrayEnd = fuzzArray.Count;
                        replaceString = "[Traversal]";
                        break;
                    case "Format":
                        fuzzGenerate = new WebFuzz.Generate("%n", 1, 10);
                        fuzzArray = fuzzGenerate.buildArray();
                        arrayEnd = fuzzArray.Count;
                        replaceString = "[Format]";
                        break;
                    case "None":
                        ArrayList nullValueArrayList = new ArrayList();
                        nullValueArrayList.Add("");
                        fuzzArray = nullValueArrayList;
                        arrayEnd = fuzzArray.Count;
                        replaceString = "[None]";
                        break;
                    default:
                        arrayEnd = 1;
                        break;
                }

                while (arrayCount < arrayEnd)
                {
                    request = "";
                    bool replacementMade = false;
                    string[] headers = rawRequest.Split('\n');
                    foreach (string header in headers)
                    {
                        string line = header;
                        if (replaceString != "")
                        {
                            //Replace the first instance of a fuzz variable
                            //Split string to account for 2+ identical fuzz variables on one line
                            string line1 = "";
                            string line2 = "";
                            if (line != "")
                            {
                                line1 = line.Substring(0, line.IndexOf(replaceString) + replaceString.Length);
                                line2 = line.Substring(line1.Length);
                            }
                            if (replacementMade == false && line1.Contains(replaceString))
                            {
                                line1 = line1.Replace(replaceString, fuzzArray[arrayCount].ToString());
                                replacementMade = true;
                            }
                            line = line1 + line2;
                            // Remove subsequent fuzz variables
                            foreach (string fuzzVariable in fuzzVariables)
                                line = line.Replace("[" + fuzzVariable + "]", "");
                        }
                        request += line += "\r\n";
                    }

                    request += "\r\n";

                    sendRequest(request, host);
                    arrayCount++;
                }

                //Remove completed fuzz strings
                //Split string to account for 2+ identical fuzz variables on one line
                string rawRequest1 = rawRequest.Substring(0, rawRequest.IndexOf(fuzz) + fuzz.Length + 1);
                string rawRequest2 = rawRequest.Substring(rawRequest1.Length);
                rawRequest1 = rawRequest1.Replace("[" + fuzz + "]", "");
                rawRequest = rawRequest1 + rawRequest2;
            }
        }

        public void sendRequest (string requestString, string requestHost)
        {
            ReadDone = new ManualResetEvent(false);
            
            string reqHost = requestHost;
            string reqString = requestString;
            byte [] reqBytes = Encoding.UTF8.GetBytes(reqString);

            string dataReceived = "";

            TcpClient client;
            NetworkStream stream;
            ClientState cs;

            try
            {
                client = new TcpClient();
                client.Connect(reqHost, Convert.ToInt32(tbxPort.Text));
                stream = client.GetStream();
                cs = new ClientState(stream, reqBytes);
            }
            catch (SocketException ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            catch (System.IO.IOException ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            } 
            
            IAsyncResult result = stream.BeginWrite(cs.ByteBuffer, 0,
                                               cs.ByteBuffer.Length,
                                               new AsyncCallback(OnWriteComplete),
                                               cs);

            result.AsyncWaitHandle.WaitOne(); // block until EndWrite is called

            // Create receive buffer
            cs.ByteBuffer = new byte[100000];

            // Receive the result back from the server
            try
            {
                result = stream.BeginRead(cs.ByteBuffer, cs.TotalBytes,
                                             cs.ByteBuffer.Length - cs.TotalBytes,
                                             new AsyncCallback(OnReadComplete), cs);
            }
            catch (System.IO.IOException ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                ReadDone.Close();
                return;
            }

            readTimeout.Elapsed += new ElapsedEventHandler(OnTimedEvent);
            readTimeout.Interval = Convert.ToInt32(tbxTimeout.Text);
            readTimeout.Enabled = true;

            ReadDone.WaitOne(); // Block until ReadDone is manually set

            stream.Close();
            client.Close();

            ReadDone.Close();

            dataReceived = cs.EchoResponse;
            string html = dataReceived;
            string status = "Error";
            if (html.Length >= 9)
            {
                status = html.Substring(9, 3);
                html.ToLower();
            }
            int htmlStart = html.IndexOf(@"<html>");
            if (htmlStart > 0)
                html = html.Substring(htmlStart);
            else
                html = "";
            int htmlEnd = html.IndexOf(@"</html>");
            if (htmlEnd > 0)
                html = html.Remove(htmlEnd);

            rtbRequestRaw.Text = reqString;
            rtbResponseRaw.Text = dataReceived;
            wbrResponse.DocumentText = html;

            string path = getPath(reqString);

            lvwResponses.Items.Add(lvwResponses.Items.Count.ToString());
            lvwResponses.Items[lvwResponses.Items.Count - 1].SubItems.Add(status);
            lvwResponses.Items[lvwResponses.Items.Count - 1].SubItems.Add(reqHost);
            lvwResponses.Items[lvwResponses.Items.Count - 1].SubItems.Add(requestString.Substring(0, requestString.IndexOf("\r\n")));
            // Force redrawing of responses to show progress
            lvwResponses.Refresh();

            requestsRaw[lvwResponses.Items.Count - 1] = reqString;
            responsesRaw[lvwResponses.Items.Count - 1] = dataReceived;
            responsesHtml[lvwResponses.Items.Count - 1] = html;
            responsesHost[lvwResponses.Items.Count - 1] = reqHost;
            responsesPath[lvwResponses.Items.Count - 1] = path;

            return;
        }

        private string getPath(string responseHeaders)
        {
            try
            {
                string firstHeader = responseHeaders.Substring(0, responseHeaders.IndexOf('\n'));
                if (firstHeader.Contains("://"))
                    firstHeader = firstHeader.Replace("://", "XXX");
                firstHeader = firstHeader.Substring(firstHeader.IndexOf('/'), firstHeader.IndexOf(' ', firstHeader.IndexOf(' ') + 1) - firstHeader.IndexOf('/'));
                return firstHeader;
            }
            catch (ArgumentOutOfRangeException ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return "";
            }
        }

        private void lvwResponse_ItemSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e)
        {
            if (e.Item.Selected)
            {
                rtbRequestRaw.Text = requestsRaw[e.ItemIndex];
                rtbResponseRaw.Text = responsesRaw[e.ItemIndex];
                wbrResponse.DocumentText = responsesHtml[e.ItemIndex];
            }
        }

        private void tmiSql_Click(object sender, EventArgs e)
        {
            rtbRequestHeaders.SelectedText = "[SQL]";
        }

        private void tmiXss_Click(object sender, EventArgs e)
        {
            rtbRequestHeaders.SelectedText = "[XSS]";
        }

        private void tmiMethods_Click(object sender, EventArgs e)
        {
            rtbRequestHeaders.SelectedText = "[Methods]";
        }

        private void tmiOverflow_Click(object sender, EventArgs e)
        {
            BufferOverflow bufferOverflow = new BufferOverflow();
            bufferOverflow.Visible = true;
            
            rtbRequestHeaders.SelectedText = "[Overflow]";
        }

        private void tmiTraversal_Click(object sender, EventArgs e)
        {
            rtbRequestHeaders.SelectedText = "[Traversal]";
        }

        private void tmiFormat_Click(object sender, EventArgs e)
        {
            rtbRequestHeaders.SelectedText = "[Format]";
        }

        private void ddmDefault_Click(object sender, EventArgs e)
        {
            rtbRequestHeaders.Text = "";
            rtbRequestHeaders.Text = defaultRequest;
        }

        private void ddmHeader_Click(object sender, EventArgs e)
        {
            rtbRequestHeaders.SelectedText = sender.ToString() + ": ";
        }

        private void ddmAuthorizationBasic_Click(object sender, EventArgs e)
        {
            BasicAuthentication basicAuthenticantion = new BasicAuthentication();
            basicAuthenticantion.Visible = true;
        }

        private void addHeader(string inputHeader)
        {
            rtbRequestHeaders.SelectedText = inputHeader;
        }

        private void addOverflow(string inputFill, int inputLength, int inputMultiplier)
        {
            overflowFill = inputFill;
            overflowLength = inputLength;
            overflowMultiplier = inputMultiplier;
        }

        public void OnReadComplete(IAsyncResult ar)
        {
            
            readTimeout.Elapsed += new ElapsedEventHandler(OnTimedEvent);
            readTimeout.Interval = Convert.ToInt32(tbxTimeout.Text);
            readTimeout.Enabled = true;

            ClientState cs = (ClientState)ar.AsyncState;
            int bytesRcvd;

            try
            {
                bytesRcvd = cs.NetStream.EndRead(ar);
            }
            catch (System.IO.IOException ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                //ReadDone.Set(); // Signal read complete
                return;
            }
            catch (System.ObjectDisposedException ex)
            {
                //MessageBox.Show(ex.Message, "Error - Read Timeout", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            cs.AppendResponse(Encoding.ASCII.GetString(cs.ByteBuffer, cs.TotalBytes, bytesRcvd));
            cs.AddToTotalBytes(bytesRcvd);

            //if (cs.TotalBytes < cs.ByteBuffer.Length)
            if (bytesRcvd != 0)
            {
                cs.NetStream.BeginRead(cs.ByteBuffer, cs.TotalBytes,
                    cs.ByteBuffer.Length - cs.TotalBytes,
                    new AsyncCallback(OnReadComplete), cs);
            }
            else
            {
                readTimeout.Enabled = false;
                if (ReadDone.Set() == false)
                    ReadDone.Set(); // Signal read complete event 
            }
        }

        public static void OnWriteComplete(IAsyncResult ar)
        {
            try
            {
                ClientState cs = (ClientState)ar.AsyncState;
                cs.NetStream.EndWrite(ar);
            }
            catch (System.ObjectDisposedException ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void OnTimedEvent(object source, ElapsedEventArgs e)
        {
            try
            {
                readTimeout.Enabled = false;
                if (ReadDone.Set() == false)
                    ReadDone.Set();
            }
            catch (System.ObjectDisposedException ex)
            {
                //MessageBox.Show(ex.Message, "Error - Read Timeout", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
        }

        private void cmsResponseClear_Click(object sender, EventArgs e)
        {
            lvwResponses.Items.Clear();
            rtbResponseRaw.Text = "";
            rtbRequestRaw.Text = "";
            wbrResponse.DocumentText = "";
        }

        private void lvwResponses_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            // Determine if clicked column is already the column that is being sorted.
            if (lvwColumnSorter.Order == SortOrder.Ascending)
            {
                // Reverse the current sort direction for this column.
                if (lvwColumnSorter.Order == SortOrder.Ascending)
                {
                    lvwColumnSorter.Order = SortOrder.Descending;
                }
                else
                {
                    lvwColumnSorter.Order = SortOrder.Ascending;
                }
            }
            else
            {
                // Set the column number that is to be sorted; default to ascending.
                lvwColumnSorter.SortColumn = e.Column;
                lvwColumnSorter.Order = SortOrder.Ascending;
            }

            // Perform the sort with these new sort options.
            this.lvwResponses.Sort();
        }
    }

    // Maintain state information to be passed to 
    // EndWriteCallback and EndReadCallback.
    class ClientState
    {
        // Object to contain client state, including the network stream
        // and the send/recv buffer

        private byte[] byteBuffer;
        private NetworkStream netStream;
        private StringBuilder echoResponse;
        private int totalBytesRcvd = 0; // Total bytes received so far

        public ClientState(NetworkStream netStream, byte[] byteBuffer)
        {
            this.netStream = netStream;
            this.byteBuffer = byteBuffer;
            echoResponse = new StringBuilder();
        }

        public NetworkStream NetStream
        {
            get
            {
                return netStream;
            }
        }

        public byte[] ByteBuffer
        {
            set
            {
                byteBuffer = value;
            }
            get
            {
                return byteBuffer;
            }
        }

        public void AppendResponse(String response)
        {
            echoResponse.Append(response);
        }
        public String EchoResponse
        {
            get
            {
                return echoResponse.ToString();
            }
        }

        public void AddToTotalBytes(int count)
        {
            totalBytesRcvd += count;
        }
        public int TotalBytes
        {
            get
            {
                return totalBytesRcvd;
            }
        }
    }
}