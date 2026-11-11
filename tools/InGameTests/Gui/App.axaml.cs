using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;

namespace MuMain.Tools.InGameTests.Gui;

/// <summary>The window's application; <see cref="GuiApp.Run"/> starts it.</summary>
internal sealed partial class App : Application
{
    /// <summary>The command line the window starts with.</summary>
    public static RunnerOptions StartOptions { get; set; } = null!;

    public override void Initialize() => AvaloniaXamlLoader.Load(this);

    public override void OnFrameworkInitializationCompleted()
    {
        if (this.ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            desktop.MainWindow = new MainWindow(StartOptions);
        }

        base.OnFrameworkInitializationCompleted();
    }
}

/// <summary>Opens the window and returns when it closes.</summary>
internal static class GuiApp
{
    public static int Run(RunnerOptions options, string[] args)
    {
        App.StartOptions = options;
        return AppBuilder.Configure<App>()
            .UsePlatformDetect()
            .LogToTrace()
            .StartWithClassicDesktopLifetime(args);
    }
}
