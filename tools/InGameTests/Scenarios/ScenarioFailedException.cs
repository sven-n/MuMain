namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>A scenario's expectation did not hold.</summary>
internal sealed class ScenarioFailedException(string message) : Exception(message);
