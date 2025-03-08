namespace ConstantsReplacer
{
    public partial class MainForm : Form
    {
        private CancellationTokenSource? _cancellationTokenSource;

        public MainForm()
        {
            InitializeComponent();
        }

        private void SearchAndReplaceClick(object? sender, EventArgs e)
        {
            this._cancellationTokenSource = new();

            this._progressBar.Visible = true;
            var progress = new Progress<(int Total, int Current)>(value =>
            {
                this._progressBar.Maximum = value.Total;
                this._progressBar.Value = value.Current;
            });

            this._cancelButton.Enabled = true;
            this._searchButton.Enabled = false;
            Task.Run( async () =>
            {
                try
                {

                    var replacer = new Replacer();
                    await replacer.SearchAndReplaceAsync(
                        _folderTextBox.Text,
                        _fileNamePatternTextBox.Text,
                        _searchPatternTextBox.Text,
                        _constantsDefinitionsTextBox.Text,
                        this._cancellationTokenSource.Token,
                        progress);

                }
                catch (Exception exception)
                {
                    this._progressBar.Text = exception.Message;
                }
                finally
                {
                    this.Invoke(() =>
                    {
                        this._cancelButton.Enabled = false;
                        this._searchButton.Enabled = true;
                        this._progressBar.Visible = false;
                    });
                    this._cancellationTokenSource?.Dispose();
                    this._cancellationTokenSource = null;
                }
            });
        }

        private void OnCancelClick(object? sender, EventArgs e)
        {
            this._cancellationTokenSource?.Cancel();
        }
    }

}
