using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Globalization;

using Metro;

using Metro.Logging;

using Metro.LinkLayer;
using Metro.LinkLayer.Ethernet802_3;

using Metro.NetworkLayer;
using Metro.NetworkLayer.ARP;
using Metro.NetworkLayer.IpV4;

using Metro.TransportLayer;
using Metro.TransportLayer.Icmp;
using Metro.TransportLayer.Tcp;
using Metro.TransportLayer.Udp;

namespace ProtoFuzz
{
    public partial class Main : Form
    {
        private const string DRIVER_NAME = @"\\.\ndisprot";
        NdisProtocolDriverInterface driver = new NdisProtocolDriverInterface();
        byte [] [] capturedPackets;
        string [] items = new string[8];
        int packetCount = 0;
        int maxPackets;

        string strSourceMacAddress = null;
        string strDestMacAddress = null;
        string strEthernet = null;
        string strSrcMac = null;
        string strDstMac = null;
        string strEthernetType = null;
        string strData = null;

        string strIp = null;
        string strIpVersion = null;
        string strIpHeaderLength = null;
        string strTypeOfService = null;
        string strIpTotalLength = null;
        string strIpIdentification = null;
        string strIpFlags = null;
        string strIpFragment = null;
        string strIpTtl = null;
        string strIpProtocol = null;
        string strIpChecksum = null;
        string strIpOptions = null;
        string strIpPadding = null;

        string strTcp = null;
        string strTcpSrcPort = null;
        string strTcpDstPort = null;
        string strTcpSeq = null;
        string strTcpAck = null;
        string strTcpOffset = null;
        string strTcpFlags = null;
        string strTcpWindow = null;
        string strTcpChecksum = null;
        string strTcpUrgetPointer = null;
        string strTcpOptions = null;
        string strTcpPadding = null;
        string strTcpData = null;

        string strUdp = null;
        string strUdpSrcPort = null;
        string strUdpDstPort = null;
        string strUdpLength = null;
        string strUdpChecksum = null;
        string strUdpData = null;

        string strIcmp = null;
        string strIcmpMessageType = null;
        string strIcmpCode = null;
        string strIcmpChecksum = null;
        string strIcmpData = null;

        string strArp = null;
        string strArpMediaType = null;
        string strArpProtocolType = null;
        string strArpType = null;
        string strArpSrcMac = null;
        string strArpSrcIp = null;
        string strArpDstMac = null;
        string strArpDstIp = null;

        // Needed to format packet data
        static System.Globalization.CultureInfo oCulture;
        
        Thread captureThread;
        
        public Main()
        {
            InitializeComponent();

            // Number formatting
            oCulture = new System.Globalization.CultureInfo(0x007F);
        }

        private void Main_Load(object sender, EventArgs e)
        {
            try
			{
                // Open the device driver
                driver.OpenDevice (DRIVER_NAME);
			}
			catch (SystemException ex)
			{
                string error = ex.Message;
                error += "\n";
                error += "Please ensure that you have correctly installed the " + 
                    DRIVER_NAME + " device driver.";
                error += "Also, make sure it has been started.";
                error += "You can start the driver by typing \"net start " + 
                    DRIVER_NAME.Substring(DRIVER_NAME.LastIndexOf("\\") + 1) + 
                    "\" at a command prompt.";
                error += "To stop it again, type \"net stop " + 
                    DRIVER_NAME.Substring(DRIVER_NAME.LastIndexOf("\\") + 1) + 
                    "\" in a command prompt.";
                error += "\n";
                error += "Press 'OK' to continue...";
                MessageBox.Show(error, "Error", MessageBoxButtons.OK, 
                    MessageBoxIcon.Error);
				return;
			}

            foreach (NetworkAdapter adapter in driver.Adapters)
            {
                cbxAdapters.Items.Add(adapter.AdapterName);
                if (cbxAdapters.Items.Count > 0)
                    cbxAdapters.SelectedIndex = 0;
            }
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            try
            {
                maxPackets = Convert.ToInt32(tbxPackets.Text);

                capturedPackets = new byte[maxPackets][];

                driver.BindAdapter(driver.Adapters[cbxAdapters.SelectedIndex]);

                ThreadStart packets = new ThreadStart(capturePacket);
                captureThread = new Thread(packets);
                captureThread.Start();
            }
            catch (IndexOutOfRangeException ex)
            {
                MessageBox.Show(ex.Message + 
                    "\nYou must select a valid network adapter.",
                    "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void btnStop_Click(object sender, EventArgs e)
        {
            if (driver.Disposed == true)
                return;
            
            captureThread.Abort();
            driver.CloseDevice();

            for (int packetLoop = 0; packetLoop < packetCount; packetLoop++)
            {
                string sourceIp = "N/A";
                string sourceMac = "N/A";
                string sourcePort = "N/A";
                string destIp = "N/A";
                string destMac = "N/A";
                string destPort = "N/A";
                string protocol = "N/A";

                Ethernet802_3 ethernet = new Ethernet802_3(capturedPackets[packetLoop]);

                sourceMac = ethernet.SourceMACAddress.ToString();
                destMac = ethernet.DestinationMACAddress.ToString();

                switch (ethernet.NetworkProtocol)
                {
                    case NetworkLayerProtocol.IP:
                        IpV4Packet ip = new IpV4Packet(ethernet.Data);

                        sourceIp = ip.SourceAddress.ToString();
                        destIp = ip.DestinationAddress.ToString();
                        protocol = ip.TransportProtocol.ToString().ToUpper();

                        switch (ip.TransportProtocol)
                        {
                            case ProtocolType.Tcp:
                                TcpPacket tcp = new TcpPacket(ip.Data);

                                sourcePort = tcp.SourcePort.ToString();
                                destPort = tcp.DestinationPort.ToString();

                                break;
                            case ProtocolType.Udp:
                                UdpPacket udp = new UdpPacket(ip.Data);

                                sourcePort = udp.SourcePort.ToString();
                                destPort = udp.DestinationPort.ToString();

                                break;
                            case ProtocolType.Icmp:
                                IcmpPacket icmp = new IcmpPacket(ip.Data);
                                break;
                        }
                        break;
                    case NetworkLayerProtocol.ARP:
                        ArpPacket arp = new ArpPacket(ethernet.Data);

                        sourceMac = arp.SourceMACAddress.ToString();
                        destMac = arp.DestinationMACAddress.ToString();
                        sourceIp = arp.SourceIPAddress.ToString();
                        destIp = arp.DestinationIPAddress.ToString();
                        protocol = arp.Protocol.ToString().ToUpper();

                        break;
                }

                string[] items = new string[8];
                items[0] = packetLoop.ToString();
                items[1] = sourceIp;
                items[2] = sourceMac;
                items[3] = sourcePort;
                items[4] = destIp;
                items[5] = destMac;
                items[6] = destPort;
                items[7] = protocol;
                lvwPacketCapture.Items.Add(new ListViewItem(items, 0));
            }
        }

        private void capturePacket()
        {
            while (packetCount < maxPackets)
            {
                byte[] packet = driver.RecievePacket();
                capturedPackets[packetCount] = packet;
                packetCount++;
            }
        }

        private void parsePacket_Click(object sender, EventArgs e)
        {
            selectListViewCapture(lvwPacketCapture, tvwPacketCapture, rtbPacketCapture);
        }

        private void selectListViewCapture(ListView lvw, TreeView tvw, RichTextBox rtb)
        {
            int selectedPacket;
            for (int i = 0; i < lvw.SelectedItems.Count; i++)
            {
                selectedPacket = Convert.ToInt32(lvw.SelectedIndices[0]);
                rtb.Clear();
                rtb.AppendText(PrintData(capturedPackets[selectedPacket]));
                tvw.Nodes.Clear();
                packetTvwDecode(capturedPackets[selectedPacket], tvw);
            }
        }

        static string PrintData(byte [] packet)
        {

            string sData = null;

            int nPosition = 0, nColumns = 16;
            for (int i = 0; i < packet.Length;  i++)
            {
                if (nPosition >= nColumns)
                {
                    nPosition = 1;
                    sData += "\n";
                }
                else
                    nPosition++;

                byte nByte = (byte) packet.GetValue(i);
                if (nByte < 16)
                    sData += "0";

                sData += nByte.ToString("X", oCulture.NumberFormat) + " ";
            }
            sData += "\n";
            return (sData);
        }

        public void packetTvwDecode(byte [] capPacket, TreeView tvwDecode)
        {
            #region Parse Ethernet Header

            Ethernet802_3 ethernet = new Ethernet802_3(capPacket);

            strSourceMacAddress = ethernet.SourceMACAddress.ToString();
            strDestMacAddress = ethernet.DestinationMACAddress.ToString();
            strEthernet = "Ethernet II, Src: " + strSourceMacAddress + ", Dst: " + strDestMacAddress;
            strSrcMac = "Source: " + strSourceMacAddress;
            strDstMac = "Destination: " + strDestMacAddress;
            strEthernetType = "Type: " + ethernet.NetworkProtocol.ToString();
            strData = "Data: " + ethernet.Data.ToString();

            TreeNode nodeEthernet = tvwDecode.Nodes.Add(strEthernet);
            TreeNode nodeEthernetDstMac = nodeEthernet.Nodes.Add(strDstMac);
            TreeNode nodeEthernetSrcMac = nodeEthernet.Nodes.Add(strSrcMac);
            TreeNode nodeType = nodeEthernet.Nodes.Add(strEthernetType);
            TreeNode nodeData = nodeEthernet.Nodes.Add(strData);

            #region Parse Network Protocol

            switch (ethernet.NetworkProtocol)
            {
                case NetworkLayerProtocol.IP:

                    IpV4Packet ip = new IpV4Packet(ethernet.Data);

                    strIp = "Internet Protocol, Src Addr: " + ip.SourceAddress.ToString() + ", Dest Addr: " + ip.DestinationAddress.ToString();
                    strIpVersion = "Version: " + ip.Version.ToString();
                    int intIpHeaderLengthHex = ip.HeaderLength;
                    int intIpHeaderLengthBytes = intIpHeaderLengthHex * 4;
                    strIpHeaderLength = "Header Length: " + intIpHeaderLengthBytes.ToString() + " bytes";
                    strTypeOfService = "Type of Service: " + ip.TypeOfService.ToString();
                    strIpTotalLength = "Total Length: " + ip.TotalLength.ToString();
                    strIpIdentification = "Identification: " + ip.Identification.ToString();
                    strIpFlags = "Flags: " + ip.ControlFlags.ToString();
                    strIpFragment = "Fragment Offset: " + ip.Fragments.ToString();
                    strIpTtl = "Time To Live: " + ip.TimeToLive.ToString();
                    strIpProtocol = "Protocol: " + ip.TransportProtocol.ToString();
                    strIpChecksum = "Header Checksum: " + ip.Checksum.ToString();
                    if (ip.Options != null)
                        strIpOptions = "Options: " + ip.Options.ToString();
                    if (ip.Padding != null)
                        strIpPadding = "Padding: " + ip.Padding.ToString();

                    TreeNode nodeIP = tvwDecode.Nodes.Add(strIp);
                    TreeNode nodeIpVersion = nodeIP.Nodes.Add(strIpVersion);
                    TreeNode nodeIpHeaderLength = nodeIP.Nodes.Add(strIpHeaderLength);
                    TreeNode nodeTypeOfService = nodeIP.Nodes.Add(strTypeOfService);
                    TreeNode nodeIpTotalLength = nodeIP.Nodes.Add(strIpTotalLength);
                    TreeNode nodeIpIdentification = nodeIP.Nodes.Add(strIpIdentification);
                    TreeNode nodeIpFlags = nodeIP.Nodes.Add(strIpFlags);
                    TreeNode nodeIpFragment = nodeIP.Nodes.Add(strIpFragment);
                    TreeNode nodeIpTtl = nodeIP.Nodes.Add(strIpTtl);
                    TreeNode nodeIpProtocol = nodeIP.Nodes.Add(strIpProtocol);
                    TreeNode nodeIpChecksum = nodeIP.Nodes.Add(strIpChecksum);
                    TreeNode nodeIpOptions = null;
                    TreeNode nodeIpPadding = null;
                    if (ip.Options != null)
                        nodeIpOptions = nodeIP.Nodes.Add(strIpOptions);
                    if (ip.Padding != null)
                        nodeIpPadding = nodeIP.Nodes.Add(strIpPadding);

                    //TreeNode nodeData = tvwDecode.Nodes.Add(strData);

                    #region Parse Transport Protocol

                    switch (ip.TransportProtocol)
                    {
                        case ProtocolType.Tcp:

                            TcpPacket tcp = new TcpPacket(ip.Data);

                            strTcp = "Transmission Control Protocol, Src Port: " + tcp.SourcePort.ToString() + ", Dst Port: " + tcp.DestinationPort.ToString() + ", Seq: " + tcp.SequenceNumber.ToString() + ", Ack: " + tcp.AcknowledgmentNumber.ToString();
                            strTcpSrcPort = "Source port: " + tcp.SourcePort.ToString();
                            strTcpDstPort = "Destination port: " + tcp.DestinationPort.ToString();
                            strTcpSeq = "Sequence number: " + tcp.SequenceNumber.ToString();
                            strTcpAck = "Acknowledgement number: " + tcp.AcknowledgmentNumber.ToString();
                            strTcpOffset = "Offset: " + tcp.Offset.ToString();
                            strTcpFlags = "Flags: " + tcp.Flags.ToString();
                            strTcpWindow = "Window size: " + tcp.Window.ToString();
                            strTcpChecksum = "Checksum: " + tcp.Checksum.ToString();
                            strTcpUrgetPointer = "Urgent Pointer: " + tcp.UrgentPointer.ToString();
                            if (tcp.Options != null)
                                strTcpOptions = "Options: " + tcp.Options.ToString();
                            if (tcp.Padding != null)
                                strTcpPadding = "Padding: " + tcp.Padding.ToString();
                            if (tcp.Data != null)
                                strTcpData = "Data: " + tcp.Data.ToString();

                            TreeNode nodeTcp = tvwDecode.Nodes.Add(strTcp);
                            TreeNode nodeTcpSrcPort = nodeTcp.Nodes.Add(strTcpSrcPort);
                            TreeNode nodeTcpDstPort = nodeTcp.Nodes.Add(strTcpDstPort);
                            TreeNode nodeTcpSeq = nodeTcp.Nodes.Add(strTcpSeq);
                            TreeNode nodeTcpAck = nodeTcp.Nodes.Add(strTcpAck);
                            TreeNode nodeTcpOffset = nodeTcp.Nodes.Add(strTcpOffset);
                            TreeNode nodeTcpFlags = nodeTcp.Nodes.Add(strTcpFlags);
                            TreeNode nodeTcpWindow = nodeTcp.Nodes.Add(strTcpWindow);
                            TreeNode nodeTcpChecksum = nodeTcp.Nodes.Add(strTcpChecksum);
                            TreeNode nodeTcpUrgetPointer = nodeTcp.Nodes.Add(strTcpUrgetPointer);
                            TreeNode nodeTcpOptions = null;
                            TreeNode nodeTcpPadding = null;
                            TreeNode nodeTcpData = null;
                            if (tcp.Options != null)
                                nodeTcpOptions = nodeTcp.Nodes.Add(strTcpOptions);
                            if (tcp.Padding != null)
                                nodeTcpPadding = nodeTcp.Nodes.Add(strTcpPadding);
                            if (tcp.Data != null)
                                nodeTcpData = nodeTcp.Nodes.Add(strTcpData);
                            break;

                        case ProtocolType.Udp:

                            UdpPacket udp = new UdpPacket(ip.Data);

                            strUdp = "User Datagram Protocol, Src Port: " + udp.SourcePort.ToString() + ", Dst Port: " + udp.DestinationPort.ToString();
                            strUdpSrcPort = "Source port: " + udp.SourcePort.ToString();
                            strUdpDstPort = "Destination port: " + udp.DestinationPort.ToString();
                            strUdpLength = "Length: " + udp.Length.ToString();
                            strUdpChecksum = "Checksum: " + udp.Checksum.ToString();
                            if (udp.Data != null)
                                strUdpData = "Data: " + udp.Data.ToString();

                            TreeNode nodeUdp = tvwDecode.Nodes.Add(strUdp);
                            TreeNode nodeUdpSrcPort = nodeUdp.Nodes.Add(strUdpSrcPort);
                            TreeNode nodeUdpDstPort = nodeUdp.Nodes.Add(strUdpDstPort);
                            TreeNode nodeUdpLength = nodeUdp.Nodes.Add(strUdpLength);
                            TreeNode nodeUdpChecksum = nodeUdp.Nodes.Add(strUdpChecksum);
                            TreeNode nodeUdpData = null;
                            if (udp.Data != null)
                                nodeUdpData = nodeUdp.Nodes.Add(strUdpData);
                            break;

                        case ProtocolType.Icmp:

                            IcmpPacket icmp = new IcmpPacket(ip.Data);

                            strIcmp = "Internet Control Message Protocol";
                            strIcmpMessageType = "Data: " + icmp.MessageType.ToString();
                            strIcmpCode = "Data: " + icmp.Code.ToString();
                            strIcmpChecksum = "Data: " + icmp.Checksum.ToString();
                            strIcmpData = "Data: " + icmp.Data.ToString();

                            TreeNode nodeIcmp = tvwDecode.Nodes.Add(strIcmp);
                            TreeNode nodeIcmpMessageType = nodeIcmp.Nodes.Add(strIcmpMessageType);
                            TreeNode nodeIcmpCode = nodeIcmp.Nodes.Add(strIcmpCode);
                            TreeNode nodeIcmpChecksum = nodeIcmp.Nodes.Add(strIcmpChecksum);
                            TreeNode nodenodeIcmpData = nodeIcmp.Nodes.Add(strIcmpData);
                            break;
                    }

                    #endregion

                    break;

                case NetworkLayerProtocol.ARP:

                    ArpPacket arp = new ArpPacket(ethernet.Data);

                    strArp = "Address Resolution Protocol";
                    strArpMediaType = "Hardware Type: " + arp.MediaType.ToString();
                    strArpProtocolType = "Protocol Type: " + arp.Protocol.ToString();
                    strArpType = "Opcode: " + arp.Type.ToString();
                    strArpSrcMac = "Sender MAC Address: " + arp.SourceMACAddress.ToString();
                    strArpSrcIp = "Sender IP Address: " + arp.SourceIPAddress.ToString();
                    strArpDstMac = "Target MAC Address: " + arp.DestinationMACAddress.ToString();
                    strArpDstIp = "Target IP Address: " + arp.DestinationIPAddress.ToString();

                    TreeNode nodeArp = tvwDecode.Nodes.Add(strArp);
                    TreeNode nodeMediaType = nodeArp.Nodes.Add(strArpMediaType);
                    TreeNode nodeArpProtocolType = nodeArp.Nodes.Add(strArpProtocolType);
                    TreeNode nodeArpType = nodeArp.Nodes.Add(strArpType);
                    TreeNode nodeArpSrcMac = nodeArp.Nodes.Add(strArpSrcMac);
                    TreeNode nodeArpSrcIp = nodeArp.Nodes.Add(strArpSrcIp);
                    TreeNode nodeArpDstMac = nodeArp.Nodes.Add(strArpDstMac);
                    TreeNode nodeArpDstIp = nodeArp.Nodes.Add(strArpDstIp);
                    break;
            }

            #endregion

            #endregion
        }

        private void tvwPacketCapture_AfterSelect(object sender, TreeViewEventArgs e)
        {
            selectTreeView(tvwPacketCapture, rtbPacketCapture);
        }

        private void selectTreeView(TreeView tvw, RichTextBox rtb)
        {
            if (tvw.SelectedNode.Text == strEthernet)
            {
                rtb.SelectAll();
                rtb.SelectionColor = Color.Black;
                rtb.Select(0, 3 * 14);
                rtb.SelectionColor = Color.Red;
                rtb.Refresh();
            }
            else if (tvw.SelectedNode.Text == strDstMac)
            {
                rtb.SelectAll();
                rtb.SelectionColor = Color.Black;
                rtb.Select(0, 3 * 6);
                rtb.SelectionColor = Color.Red;
                rtb.Refresh();
            }
            else if (tvw.SelectedNode.Text == strSrcMac)
            {
                rtb.SelectAll();
                rtb.SelectionColor = Color.Black;
                rtb.Select(3 * 6, 3 * 6);
                rtb.SelectionColor = Color.Red;
                rtb.Refresh();
            }
            else if (tvw.SelectedNode.Text == strEthernetType)
            {
                rtb.SelectAll();
                rtb.SelectionColor = Color.Black;
                rtb.Select(3 * 12, 3 * 2);
                rtb.SelectionColor = Color.Red;
                rtb.Refresh();
            }
            else if (tvw.SelectedNode.Text == strIp && lvwPacketCapture.SelectedItems[0].SubItems[7].Text != "Ethernet")
            {
                rtb.SelectAll();
                rtb.SelectionColor = Color.Black;
                rtb.Select(3 * 14, 3 * 20 + (rtbPacketCapture.Text.Substring(3 * 14, 3 * 20).Split('\n').GetLength(0)) / 2);
                rtb.SelectionColor = Color.Red;
                rtb.Refresh();
            }
            else if (tvw.SelectedNode.Text == strIpVersion)
            {
                rtb.SelectAll();
                rtb.SelectionColor = Color.Black;
                rtb.Select(3 * 14, 3 * 1);
                rtb.SelectionColor = Color.Red;
                rtb.Refresh();
            }
            //
            else if (tvw.SelectedNode.Text == strData && lvwPacketCapture.SelectedItems[0].SubItems[7].Text == "Ethernet")
            {

                rtb.SelectAll();
                rtb.SelectionColor = Color.Black;
                rtb.Select(3 * 14, rtbPacketCapture.Text.Length - 3 * 14);
                rtb.SelectionColor = Color.Red;
                rtb.Refresh();
            }
            else
            {
                rtb.SelectAll();
                rtb.SelectionColor = Color.Black;
            }
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            driver.CloseDevice();
            Application.Exit();
        }

        private void sendPacketToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string packet = rtbPacketCapture.Text.Replace(" ", "");
            string bytes = null;
            int iterations = 0;
            HexEncoding pktToSend = new HexEncoding();
            int discarded;

            // Brute force fuzzing 
            if (packet.Contains("["))
            {
                bytes = packet.Substring(packet.IndexOf('[') + 1, packet.IndexOf(']') - packet.IndexOf('[') - 1);
                iterations = Convert.ToInt32(Math.Pow(256, (bytes.Length / 2)));

                for (int packetLoop = 0; packetLoop < iterations; packetLoop++)
                {
                    string replace = packet.Replace("[" + bytes + "]", packetLoop.ToString("x2"));
                    byte[] bytesToSend = pktToSend.GetBytes(replace, out discarded);

                    driver.SendPacket(bytesToSend);
                }
            }
            // Fuzzing with predefined strings
            else if (packet.Contains("<"))
            {
                String[] fuzzStrings = new string[100];
                int stringCount = 0;
                bytes = packet.Substring(packet.IndexOf('<') + 1, packet.IndexOf('>') - packet.IndexOf('<') - 1);

                StreamReader srStrings = File.OpenText("Strings.txt");
                string line = null;
                while ((line = srStrings.ReadLine()) != null)
                {
                    fuzzStrings[stringCount] = line;
                    stringCount++;
                }
                srStrings.Close();

                iterations = stringCount;
                for (int packetLoop = 0; packetLoop < iterations; packetLoop++)
                {
                    System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
                    string fuzzStringHex = pktToSend.ByteToHex(encoding.GetBytes(fuzzStrings[packetLoop]));

                    string replace = packet.Replace("<" + bytes + ">", fuzzStringHex);
                    byte[] bytesToSend = pktToSend.GetBytes(replace, out discarded);

                    driver.SendPacket(bytesToSend);
                }
            }
        }

        private void bruteForceToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //TODO Fix first blank char
            while (rtbPacketCapture.Text[rtbPacketCapture.SelectionStart] != ' ')
                rtbPacketCapture.SelectionStart = rtbPacketCapture.SelectionStart - 1;
            while (rtbPacketCapture.Text[rtbPacketCapture.SelectionStart + rtbPacketCapture.SelectionLength] != ' ')
                rtbPacketCapture.SelectionLength = rtbPacketCapture.SelectionLength + 1;
            int selectionStart = rtbPacketCapture.SelectionStart + 1;
            int selectionEnd = selectionStart + rtbPacketCapture.SelectionLength - 1;
            string fuzzTemplate = rtbPacketCapture.Text.Insert(selectionStart, "<");
            fuzzTemplate = fuzzTemplate.Insert(selectionEnd + 1, ">");
            rtbPacketCapture.Clear();
            rtbPacketCapture.AppendText(fuzzTemplate);
        }

        private void stringsToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

    }
}