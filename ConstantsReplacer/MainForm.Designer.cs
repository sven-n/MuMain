namespace ConstantsReplacer
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            _folderLabel = new Label();
            _folderTextBox = new TextBox();
            _fileNamePatternTextBox = new TextBox();
            _fileNamePatternLabel = new Label();
            _searchPatternTextBox = new TextBox();
            label1 = new Label();
            _constantsDefinitionsTextBox = new TextBox();
            label2 = new Label();
            _searchButton = new Button();
            _progressBar = new ProgressBar();
            _cancelButton = new Button();
            SuspendLayout();
            // 
            // _folderLabel
            // 
            _folderLabel.AutoSize = true;
            _folderLabel.Location = new Point(15, 19);
            _folderLabel.Name = "_folderLabel";
            _folderLabel.Size = new Size(43, 15);
            _folderLabel.TabIndex = 0;
            _folderLabel.Text = "Folder:";
            // 
            // _folderTextBox
            // 
            _folderTextBox.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            _folderTextBox.Location = new Point(135, 16);
            _folderTextBox.Name = "_folderTextBox";
            _folderTextBox.Size = new Size(653, 23);
            _folderTextBox.TabIndex = 1;
            _folderTextBox.Text = "C:\\Users\\Sven\\Documents\\GitHub\\MuMain\\Source Main 5.2\\source\\";
            // 
            // _fileNamePatternTextBox
            // 
            _fileNamePatternTextBox.Location = new Point(135, 45);
            _fileNamePatternTextBox.Name = "_fileNamePatternTextBox";
            _fileNamePatternTextBox.Size = new Size(101, 23);
            _fileNamePatternTextBox.TabIndex = 3;
            _fileNamePatternTextBox.Text = "*.cpp";
            // 
            // _fileNamePatternLabel
            // 
            _fileNamePatternLabel.AutoSize = true;
            _fileNamePatternLabel.Location = new Point(15, 48);
            _fileNamePatternLabel.Name = "_fileNamePatternLabel";
            _fileNamePatternLabel.Size = new Size(99, 15);
            _fileNamePatternLabel.TabIndex = 2;
            _fileNamePatternLabel.Text = "Filename Pattern:";
            // 
            // _searchPatternTextBox
            // 
            _searchPatternTextBox.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            _searchPatternTextBox.Location = new Point(135, 74);
            _searchPatternTextBox.Name = "_searchPatternTextBox";
            _searchPatternTextBox.Size = new Size(653, 23);
            _searchPatternTextBox.TabIndex = 4;
            _searchPatternTextBox.Text = "(\\bMODEL_\\w*\\b\\s[+]\\s\\d+\\b)";
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(15, 77);
            label1.Name = "label1";
            label1.Size = new Size(88, 15);
            label1.TabIndex = 5;
            label1.Text = "Search-Pattern:";
            // 
            // _constantsDefinitionsTextBox
            // 
            _constantsDefinitionsTextBox.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            _constantsDefinitionsTextBox.Location = new Point(136, 103);
            _constantsDefinitionsTextBox.Multiline = true;
            _constantsDefinitionsTextBox.Name = "_constantsDefinitionsTextBox";
            _constantsDefinitionsTextBox.ScrollBars = ScrollBars.Both;
            _constantsDefinitionsTextBox.Size = new Size(652, 302);
            _constantsDefinitionsTextBox.TabIndex = 6;
            _constantsDefinitionsTextBox.Text = resources.GetString("_constantsDefinitionsTextBox.Text");
            _constantsDefinitionsTextBox.WordWrap = false;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(15, 106);
            label2.Name = "label2";
            label2.Size = new Size(63, 15);
            label2.TabIndex = 7;
            label2.Text = "Constants:";
            // 
            // _searchButton
            // 
            _searchButton.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            _searchButton.Location = new Point(667, 415);
            _searchButton.Name = "_searchButton";
            _searchButton.Size = new Size(121, 23);
            _searchButton.TabIndex = 8;
            _searchButton.Text = "Search and Replace";
            _searchButton.UseVisualStyleBackColor = true;
            _searchButton.Click += SearchAndReplaceClick;
            // 
            // _progressBar
            // 
            _progressBar.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            _progressBar.Location = new Point(135, 415);
            _progressBar.Name = "_progressBar";
            _progressBar.Size = new Size(445, 23);
            _progressBar.TabIndex = 9;
            _progressBar.Visible = false;
            // 
            // _cancelButton
            // 
            _cancelButton.Enabled = false;
            _cancelButton.Location = new Point(586, 415);
            _cancelButton.Name = "_cancelButton";
            _cancelButton.Size = new Size(75, 23);
            _cancelButton.TabIndex = 10;
            _cancelButton.Text = "Cancel";
            _cancelButton.UseVisualStyleBackColor = true;
            _cancelButton.Click += OnCancelClick;
            // 
            // MainForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(_cancelButton);
            Controls.Add(_progressBar);
            Controls.Add(_searchButton);
            Controls.Add(label2);
            Controls.Add(_constantsDefinitionsTextBox);
            Controls.Add(label1);
            Controls.Add(_searchPatternTextBox);
            Controls.Add(_fileNamePatternTextBox);
            Controls.Add(_fileNamePatternLabel);
            Controls.Add(_folderTextBox);
            Controls.Add(_folderLabel);
            Name = "MainForm";
            Text = "Constants Replacer";
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label _folderLabel;
        private TextBox _folderTextBox;
        private TextBox _fileNamePatternTextBox;
        private Label _fileNamePatternLabel;
        private TextBox _searchPatternTextBox;
        private Label label1;
        private TextBox _constantsDefinitionsTextBox;
        private Label label2;
        private Button _searchButton;
        private ProgressBar _progressBar;
        private Button _cancelButton;
    }
}
