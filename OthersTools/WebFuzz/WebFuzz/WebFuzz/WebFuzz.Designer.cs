namespace WebFuzz
{
    partial class frmWebFuzz
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.btnRequest = new System.Windows.Forms.Button();
            this.tbxHost = new System.Windows.Forms.TextBox();
            this.lblHost = new System.Windows.Forms.Label();
            this.rtbRequestHeaders = new System.Windows.Forms.RichTextBox();
            this.cmsRequest = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.cmsRequestHeader = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmDefault = new System.Windows.Forms.ToolStripMenuItem();
            this.tssDefault = new System.Windows.Forms.ToolStripSeparator();
            this.ddmAccept = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmAcceptEncoding = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmAcceptLanguage = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmAuthorization = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmAuthorizationBasic = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmChargeTo = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmConnection = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmContentLength = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmFrom = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmHost = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmIfModifiedSince = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmPragma = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmProxyConnection = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmReferer = new System.Windows.Forms.ToolStripMenuItem();
            this.ddmUserAgent = new System.Windows.Forms.ToolStripMenuItem();
            this.cmsRequestFuzz = new System.Windows.Forms.ToolStripMenuItem();
            this.tmiTraversal = new System.Windows.Forms.ToolStripMenuItem();
            this.tmiFormat = new System.Windows.Forms.ToolStripMenuItem();
            this.tmiMethods = new System.Windows.Forms.ToolStripMenuItem();
            this.tmiOverflow = new System.Windows.Forms.ToolStripMenuItem();
            this.tmiSql = new System.Windows.Forms.ToolStripMenuItem();
            this.tmiXss = new System.Windows.Forms.ToolStripMenuItem();
            this.rtbResponseRaw = new System.Windows.Forms.RichTextBox();
            this.wbrResponse = new System.Windows.Forms.WebBrowser();
            this.lvwResponses = new System.Windows.Forms.ListView();
            this.chdNo = new System.Windows.Forms.ColumnHeader();
            this.chdStatus = new System.Windows.Forms.ColumnHeader();
            this.chdHost = new System.Windows.Forms.ColumnHeader();
            this.chdRequest = new System.Windows.Forms.ColumnHeader();
            this.cmsResponse = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.cmsResponseClear = new System.Windows.Forms.ToolStripMenuItem();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tpgRawRequest = new System.Windows.Forms.TabPage();
            this.rtbRequestRaw = new System.Windows.Forms.RichTextBox();
            this.tpgRawResponse = new System.Windows.Forms.TabPage();
            this.tpgHtml = new System.Windows.Forms.TabPage();
            this.gbxRequestHeaders = new System.Windows.Forms.GroupBox();
            this.gbxResponses = new System.Windows.Forms.GroupBox();
            this.tbxPort = new System.Windows.Forms.TextBox();
            this.lblPort = new System.Windows.Forms.Label();
            this.lblTimeout = new System.Windows.Forms.Label();
            this.tbxTimeout = new System.Windows.Forms.TextBox();
            this.cmsRequest.SuspendLayout();
            this.cmsResponse.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tpgRawRequest.SuspendLayout();
            this.tpgRawResponse.SuspendLayout();
            this.tpgHtml.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnRequest
            // 
            this.btnRequest.Location = new System.Drawing.Point(1076, 2);
            this.btnRequest.Name = "btnRequest";
            this.btnRequest.Size = new System.Drawing.Size(75, 24);
            this.btnRequest.TabIndex = 0;
            this.btnRequest.Text = "Request";
            this.btnRequest.UseVisualStyleBackColor = true;
            this.btnRequest.Click += new System.EventHandler(this.btnRequest_Click);
            // 
            // tbxHost
            // 
            this.tbxHost.Location = new System.Drawing.Point(64, 3);
            this.tbxHost.Name = "tbxHost";
            this.tbxHost.Size = new System.Drawing.Size(299, 22);
            this.tbxHost.TabIndex = 1;
            this.tbxHost.Text = "192.168.1.37";
            // 
            // lblHost
            // 
            this.lblHost.AutoSize = true;
            this.lblHost.Location = new System.Drawing.Point(17, 5);
            this.lblHost.Name = "lblHost";
            this.lblHost.Size = new System.Drawing.Size(37, 17);
            this.lblHost.TabIndex = 8;
            this.lblHost.Text = "Host";
            // 
            // rtbRequestHeaders
            // 
            this.rtbRequestHeaders.ContextMenuStrip = this.cmsRequest;
            this.rtbRequestHeaders.DetectUrls = false;
            this.rtbRequestHeaders.Location = new System.Drawing.Point(12, 43);
            this.rtbRequestHeaders.Name = "rtbRequestHeaders";
            this.rtbRequestHeaders.Size = new System.Drawing.Size(1130, 149);
            this.rtbRequestHeaders.TabIndex = 9;
            this.rtbRequestHeaders.Text = "GET / HTTP/1.1\nAccept: */*\nAccept-Language: en-us\nPragma: no-cache\nUser-Agent: Mo" +
                "zilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; InfoPath.1)\nHost: localhos" +
                "t\nProxy-Connection: Keep-Alive";
            // 
            // cmsRequest
            // 
            this.cmsRequest.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cmsRequestHeader,
            this.cmsRequestFuzz});
            this.cmsRequest.Name = "contextMenuStrip1";
            this.cmsRequest.Size = new System.Drawing.Size(191, 48);
            // 
            // cmsRequestHeader
            // 
            this.cmsRequestHeader.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ddmDefault,
            this.tssDefault,
            this.ddmAccept,
            this.ddmAcceptEncoding,
            this.ddmAcceptLanguage,
            this.ddmAuthorization,
            this.ddmChargeTo,
            this.ddmConnection,
            this.ddmContentLength,
            this.ddmFrom,
            this.ddmHost,
            this.ddmIfModifiedSince,
            this.ddmPragma,
            this.ddmProxyConnection,
            this.ddmReferer,
            this.ddmUserAgent});
            this.cmsRequestHeader.Name = "cmsRequestHeader";
            this.cmsRequestHeader.Size = new System.Drawing.Size(190, 22);
            this.cmsRequestHeader.Text = "Add Header";
            // 
            // ddmDefault
            // 
            this.ddmDefault.Name = "ddmDefault";
            this.ddmDefault.Size = new System.Drawing.Size(204, 22);
            this.ddmDefault.Text = "Default Headers";
            this.ddmDefault.Click += new System.EventHandler(this.ddmDefault_Click);
            // 
            // tssDefault
            // 
            this.tssDefault.Name = "tssDefault";
            this.tssDefault.Size = new System.Drawing.Size(201, 6);
            // 
            // ddmAccept
            // 
            this.ddmAccept.Name = "ddmAccept";
            this.ddmAccept.Size = new System.Drawing.Size(204, 22);
            this.ddmAccept.Text = "Accept";
            this.ddmAccept.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmAcceptEncoding
            // 
            this.ddmAcceptEncoding.Name = "ddmAcceptEncoding";
            this.ddmAcceptEncoding.Size = new System.Drawing.Size(204, 22);
            this.ddmAcceptEncoding.Text = "Accept-Encoding";
            this.ddmAcceptEncoding.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmAcceptLanguage
            // 
            this.ddmAcceptLanguage.Name = "ddmAcceptLanguage";
            this.ddmAcceptLanguage.Size = new System.Drawing.Size(204, 22);
            this.ddmAcceptLanguage.Text = "Accept-Language";
            this.ddmAcceptLanguage.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmAuthorization
            // 
            this.ddmAuthorization.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ddmAuthorizationBasic});
            this.ddmAuthorization.Name = "ddmAuthorization";
            this.ddmAuthorization.Size = new System.Drawing.Size(204, 22);
            this.ddmAuthorization.Text = "Authorization";
            this.ddmAuthorization.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmAuthorizationBasic
            // 
            this.ddmAuthorizationBasic.Name = "ddmAuthorizationBasic";
            this.ddmAuthorizationBasic.Size = new System.Drawing.Size(124, 22);
            this.ddmAuthorizationBasic.Text = "Basic";
            this.ddmAuthorizationBasic.Click += new System.EventHandler(this.ddmAuthorizationBasic_Click);
            // 
            // ddmChargeTo
            // 
            this.ddmChargeTo.Name = "ddmChargeTo";
            this.ddmChargeTo.Size = new System.Drawing.Size(204, 22);
            this.ddmChargeTo.Text = "Charge-To";
            this.ddmChargeTo.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmConnection
            // 
            this.ddmConnection.Name = "ddmConnection";
            this.ddmConnection.Size = new System.Drawing.Size(204, 22);
            this.ddmConnection.Text = "Connection";
            this.ddmConnection.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmContentLength
            // 
            this.ddmContentLength.Name = "ddmContentLength";
            this.ddmContentLength.Size = new System.Drawing.Size(204, 22);
            this.ddmContentLength.Text = "Content-Length";
            this.ddmContentLength.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmFrom
            // 
            this.ddmFrom.Name = "ddmFrom";
            this.ddmFrom.Size = new System.Drawing.Size(204, 22);
            this.ddmFrom.Text = "From";
            this.ddmFrom.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmHost
            // 
            this.ddmHost.Name = "ddmHost";
            this.ddmHost.Size = new System.Drawing.Size(204, 22);
            this.ddmHost.Text = "Host";
            this.ddmHost.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmIfModifiedSince
            // 
            this.ddmIfModifiedSince.Name = "ddmIfModifiedSince";
            this.ddmIfModifiedSince.Size = new System.Drawing.Size(204, 22);
            this.ddmIfModifiedSince.Text = "If-Modified-Since";
            this.ddmIfModifiedSince.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmPragma
            // 
            this.ddmPragma.Name = "ddmPragma";
            this.ddmPragma.Size = new System.Drawing.Size(204, 22);
            this.ddmPragma.Text = "Pragma";
            this.ddmPragma.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmProxyConnection
            // 
            this.ddmProxyConnection.Name = "ddmProxyConnection";
            this.ddmProxyConnection.Size = new System.Drawing.Size(204, 22);
            this.ddmProxyConnection.Text = "Proxy-Connection";
            this.ddmProxyConnection.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmReferer
            // 
            this.ddmReferer.Name = "ddmReferer";
            this.ddmReferer.Size = new System.Drawing.Size(204, 22);
            this.ddmReferer.Text = "Referer";
            this.ddmReferer.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // ddmUserAgent
            // 
            this.ddmUserAgent.Name = "ddmUserAgent";
            this.ddmUserAgent.Size = new System.Drawing.Size(204, 22);
            this.ddmUserAgent.Text = "User-Agent";
            this.ddmUserAgent.Click += new System.EventHandler(this.ddmHeader_Click);
            // 
            // cmsRequestFuzz
            // 
            this.cmsRequestFuzz.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tmiTraversal,
            this.tmiFormat,
            this.tmiMethods,
            this.tmiOverflow,
            this.tmiSql,
            this.tmiXss});
            this.cmsRequestFuzz.Name = "cmsRequestFuzz";
            this.cmsRequestFuzz.Size = new System.Drawing.Size(190, 22);
            this.cmsRequestFuzz.Text = "Add Fuzz Type";
            // 
            // tmiTraversal
            // 
            this.tmiTraversal.Name = "tmiTraversal";
            this.tmiTraversal.Size = new System.Drawing.Size(215, 22);
            this.tmiTraversal.Text = "Directory Traversal";
            this.tmiTraversal.Click += new System.EventHandler(this.tmiTraversal_Click);
            // 
            // tmiFormat
            // 
            this.tmiFormat.Name = "tmiFormat";
            this.tmiFormat.Size = new System.Drawing.Size(215, 22);
            this.tmiFormat.Text = "Format String";
            this.tmiFormat.Click += new System.EventHandler(this.tmiFormat_Click);
            // 
            // tmiMethods
            // 
            this.tmiMethods.Name = "tmiMethods";
            this.tmiMethods.Size = new System.Drawing.Size(215, 22);
            this.tmiMethods.Text = "Methods";
            this.tmiMethods.Click += new System.EventHandler(this.tmiMethods_Click);
            // 
            // tmiOverflow
            // 
            this.tmiOverflow.Name = "tmiOverflow";
            this.tmiOverflow.Size = new System.Drawing.Size(215, 22);
            this.tmiOverflow.Text = "Overflow";
            this.tmiOverflow.Click += new System.EventHandler(this.tmiOverflow_Click);
            // 
            // tmiSql
            // 
            this.tmiSql.Name = "tmiSql";
            this.tmiSql.Size = new System.Drawing.Size(215, 22);
            this.tmiSql.Text = "SQL Injection";
            this.tmiSql.Click += new System.EventHandler(this.tmiSql_Click);
            // 
            // tmiXss
            // 
            this.tmiXss.Name = "tmiXss";
            this.tmiXss.Size = new System.Drawing.Size(215, 22);
            this.tmiXss.Text = "XSS Injection";
            this.tmiXss.Click += new System.EventHandler(this.tmiXss_Click);
            // 
            // rtbResponseRaw
            // 
            this.rtbResponseRaw.DetectUrls = false;
            this.rtbResponseRaw.Dock = System.Windows.Forms.DockStyle.Fill;
            this.rtbResponseRaw.Location = new System.Drawing.Point(3, 3);
            this.rtbResponseRaw.Name = "rtbResponseRaw";
            this.rtbResponseRaw.Size = new System.Drawing.Size(1117, 427);
            this.rtbResponseRaw.TabIndex = 11;
            this.rtbResponseRaw.Text = "";
            // 
            // wbrResponse
            // 
            this.wbrResponse.Dock = System.Windows.Forms.DockStyle.Fill;
            this.wbrResponse.Location = new System.Drawing.Point(3, 3);
            this.wbrResponse.MinimumSize = new System.Drawing.Size(20, 20);
            this.wbrResponse.Name = "wbrResponse";
            this.wbrResponse.ScriptErrorsSuppressed = true;
            this.wbrResponse.Size = new System.Drawing.Size(1117, 427);
            this.wbrResponse.TabIndex = 12;
            // 
            // lvwResponses
            // 
            this.lvwResponses.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.chdNo,
            this.chdStatus,
            this.chdHost,
            this.chdRequest});
            this.lvwResponses.ContextMenuStrip = this.cmsResponse;
            this.lvwResponses.Location = new System.Drawing.Point(13, 218);
            this.lvwResponses.Name = "lvwResponses";
            this.lvwResponses.Size = new System.Drawing.Size(1131, 155);
            this.lvwResponses.TabIndex = 13;
            this.lvwResponses.UseCompatibleStateImageBehavior = false;
            this.lvwResponses.View = System.Windows.Forms.View.Details;
            this.lvwResponses.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.lvwResponses_ColumnClick);
            this.lvwResponses.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.lvwResponse_ItemSelectionChanged);
            // 
            // chdNo
            // 
            this.chdNo.Text = "No.";
            this.chdNo.Width = 62;
            // 
            // chdStatus
            // 
            this.chdStatus.Text = "Status";
            this.chdStatus.Width = 80;
            // 
            // chdHost
            // 
            this.chdHost.Text = "Host";
            this.chdHost.Width = 169;
            // 
            // chdRequest
            // 
            this.chdRequest.Text = "Request";
            this.chdRequest.Width = 384;
            // 
            // cmsResponse
            // 
            this.cmsResponse.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cmsResponseClear});
            this.cmsResponse.Name = "cmsResponse";
            this.cmsResponse.Size = new System.Drawing.Size(124, 26);
            // 
            // cmsResponseClear
            // 
            this.cmsResponseClear.Name = "cmsResponseClear";
            this.cmsResponseClear.Size = new System.Drawing.Size(123, 22);
            this.cmsResponseClear.Text = "Clear";
            this.cmsResponseClear.Click += new System.EventHandler(this.cmsResponseClear_Click);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tpgRawRequest);
            this.tabControl1.Controls.Add(this.tpgRawResponse);
            this.tabControl1.Controls.Add(this.tpgHtml);
            this.tabControl1.Location = new System.Drawing.Point(13, 385);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(1131, 462);
            this.tabControl1.TabIndex = 14;
            // 
            // tpgRawRequest
            // 
            this.tpgRawRequest.Controls.Add(this.rtbRequestRaw);
            this.tpgRawRequest.Location = new System.Drawing.Point(4, 25);
            this.tpgRawRequest.Name = "tpgRawRequest";
            this.tpgRawRequest.Size = new System.Drawing.Size(1123, 433);
            this.tpgRawRequest.TabIndex = 2;
            this.tpgRawRequest.Text = "Raw Request";
            this.tpgRawRequest.UseVisualStyleBackColor = true;
            // 
            // rtbRequestRaw
            // 
            this.rtbRequestRaw.DetectUrls = false;
            this.rtbRequestRaw.Dock = System.Windows.Forms.DockStyle.Fill;
            this.rtbRequestRaw.Location = new System.Drawing.Point(0, 0);
            this.rtbRequestRaw.Name = "rtbRequestRaw";
            this.rtbRequestRaw.Size = new System.Drawing.Size(1123, 433);
            this.rtbRequestRaw.TabIndex = 13;
            this.rtbRequestRaw.Text = "";
            // 
            // tpgRawResponse
            // 
            this.tpgRawResponse.Controls.Add(this.rtbResponseRaw);
            this.tpgRawResponse.Location = new System.Drawing.Point(4, 25);
            this.tpgRawResponse.Name = "tpgRawResponse";
            this.tpgRawResponse.Padding = new System.Windows.Forms.Padding(3);
            this.tpgRawResponse.Size = new System.Drawing.Size(1123, 433);
            this.tpgRawResponse.TabIndex = 0;
            this.tpgRawResponse.Text = "Raw Response";
            this.tpgRawResponse.UseVisualStyleBackColor = true;
            // 
            // tpgHtml
            // 
            this.tpgHtml.Controls.Add(this.wbrResponse);
            this.tpgHtml.Location = new System.Drawing.Point(4, 25);
            this.tpgHtml.Name = "tpgHtml";
            this.tpgHtml.Padding = new System.Windows.Forms.Padding(3);
            this.tpgHtml.Size = new System.Drawing.Size(1123, 433);
            this.tpgHtml.TabIndex = 1;
            this.tpgHtml.Text = "HTML";
            this.tpgHtml.UseVisualStyleBackColor = true;
            // 
            // gbxRequestHeaders
            // 
            this.gbxRequestHeaders.Location = new System.Drawing.Point(6, 25);
            this.gbxRequestHeaders.Name = "gbxRequestHeaders";
            this.gbxRequestHeaders.Size = new System.Drawing.Size(1147, 173);
            this.gbxRequestHeaders.TabIndex = 15;
            this.gbxRequestHeaders.TabStop = false;
            this.gbxRequestHeaders.Text = "Request Headers";
            // 
            // gbxResponses
            // 
            this.gbxResponses.Location = new System.Drawing.Point(6, 196);
            this.gbxResponses.Name = "gbxResponses";
            this.gbxResponses.Size = new System.Drawing.Size(1147, 183);
            this.gbxResponses.TabIndex = 16;
            this.gbxResponses.TabStop = false;
            this.gbxResponses.Text = "Responses";
            // 
            // tbxPort
            // 
            this.tbxPort.Location = new System.Drawing.Point(447, 4);
            this.tbxPort.MaxLength = 5;
            this.tbxPort.Name = "tbxPort";
            this.tbxPort.Size = new System.Drawing.Size(60, 22);
            this.tbxPort.TabIndex = 17;
            this.tbxPort.Text = "80";
            // 
            // lblPort
            // 
            this.lblPort.AutoSize = true;
            this.lblPort.Location = new System.Drawing.Point(404, 6);
            this.lblPort.Name = "lblPort";
            this.lblPort.Size = new System.Drawing.Size(34, 17);
            this.lblPort.TabIndex = 18;
            this.lblPort.Text = "Port";
            // 
            // lblTimeout
            // 
            this.lblTimeout.AutoSize = true;
            this.lblTimeout.Location = new System.Drawing.Point(561, 6);
            this.lblTimeout.Name = "lblTimeout";
            this.lblTimeout.Size = new System.Drawing.Size(149, 17);
            this.lblTimeout.TabIndex = 19;
            this.lblTimeout.Text = "Timeout (Milliseconds)";
            // 
            // tbxTimeout
            // 
            this.tbxTimeout.Location = new System.Drawing.Point(726, 4);
            this.tbxTimeout.MaxLength = 5;
            this.tbxTimeout.Name = "tbxTimeout";
            this.tbxTimeout.Size = new System.Drawing.Size(60, 22);
            this.tbxTimeout.TabIndex = 20;
            this.tbxTimeout.Text = "5000";
            // 
            // frmWebFuzz
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1157, 884);
            this.Controls.Add(this.tbxTimeout);
            this.Controls.Add(this.lblTimeout);
            this.Controls.Add(this.lblPort);
            this.Controls.Add(this.tbxPort);
            this.Controls.Add(this.rtbRequestHeaders);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.gbxRequestHeaders);
            this.Controls.Add(this.lvwResponses);
            this.Controls.Add(this.tbxHost);
            this.Controls.Add(this.lblHost);
            this.Controls.Add(this.gbxResponses);
            this.Controls.Add(this.btnRequest);
            this.Name = "frmWebFuzz";
            this.Text = "WebFuzz";
            this.cmsRequest.ResumeLayout(false);
            this.cmsResponse.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.tpgRawRequest.ResumeLayout(false);
            this.tpgRawResponse.ResumeLayout(false);
            this.tpgHtml.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnRequest;
        private System.Windows.Forms.TextBox tbxHost;
        private System.Windows.Forms.Label lblHost;
        private System.Windows.Forms.RichTextBox rtbResponseRaw;
        private System.Windows.Forms.WebBrowser wbrResponse;
        private System.Windows.Forms.ContextMenuStrip cmsRequest;
        private System.Windows.Forms.ToolStripMenuItem cmsRequestHeader;
        private System.Windows.Forms.ToolStripMenuItem cmsRequestFuzz;
        private System.Windows.Forms.ToolStripMenuItem ddmAccept;
        private System.Windows.Forms.ToolStripMenuItem ddmFrom;
        private System.Windows.Forms.ToolStripMenuItem ddmAcceptEncoding;
        private System.Windows.Forms.ToolStripMenuItem ddmAcceptLanguage;
        private System.Windows.Forms.ToolStripMenuItem ddmUserAgent;
        private System.Windows.Forms.ToolStripMenuItem ddmReferer;
        private System.Windows.Forms.ToolStripMenuItem ddmAuthorization;
        private System.Windows.Forms.ToolStripMenuItem ddmChargeTo;
        private System.Windows.Forms.ToolStripMenuItem ddmIfModifiedSince;
        private System.Windows.Forms.ToolStripMenuItem ddmPragma;
        private System.Windows.Forms.ToolStripMenuItem tmiSql;
        private System.Windows.Forms.ListView lvwResponses;
        private System.Windows.Forms.ColumnHeader chdNo;
        private System.Windows.Forms.ColumnHeader chdStatus;
        private System.Windows.Forms.ColumnHeader chdHost;
        private System.Windows.Forms.ColumnHeader chdRequest;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tpgRawResponse;
        private System.Windows.Forms.TabPage tpgHtml;
        private System.Windows.Forms.TabPage tpgRawRequest;
        private System.Windows.Forms.RichTextBox rtbRequestRaw;
        private System.Windows.Forms.ToolStripMenuItem tmiXss;
        private System.Windows.Forms.ToolStripMenuItem tmiMethods;
        private System.Windows.Forms.ToolStripMenuItem ddmDefault;
        private System.Windows.Forms.ToolStripMenuItem tmiOverflow;
        private System.Windows.Forms.GroupBox gbxRequestHeaders;
        private System.Windows.Forms.GroupBox gbxResponses;
        private System.Windows.Forms.ToolStripMenuItem ddmAuthorizationBasic;
        private System.Windows.Forms.ToolStripSeparator tssDefault;
        private System.Windows.Forms.RichTextBox rtbRequestHeaders;
        private System.Windows.Forms.TextBox tbxPort;
        private System.Windows.Forms.Label lblPort;
        private System.Windows.Forms.ContextMenuStrip cmsResponse;
        private System.Windows.Forms.ToolStripMenuItem cmsResponseClear;
        private System.Windows.Forms.ToolStripMenuItem ddmHost;
        private System.Windows.Forms.ToolStripMenuItem ddmConnection;
        private System.Windows.Forms.ToolStripMenuItem ddmProxyConnection;
        private System.Windows.Forms.ToolStripMenuItem ddmContentLength;
        private System.Windows.Forms.Label lblTimeout;
        private System.Windows.Forms.TextBox tbxTimeout;
        private System.Windows.Forms.ToolStripMenuItem tmiTraversal;
        private System.Windows.Forms.ToolStripMenuItem tmiFormat;
    }
}

