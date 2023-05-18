using Microsoft.Win32;
using System.Runtime.InteropServices;
using System.Security.Cryptography;

public static class Utility
{
    public static bool HasKeywordArguments(this string Arg)
    {
        return Arg.Contains('=');
    }

    public static Dictionary<string, string> GetKeywordArguments(this string[] Arguments)
    {
        Dictionary<string, string> Kwargs = new();

        foreach (string KwargString in Arguments)
        {
            if (!KwargString.HasKeywordArguments()) continue;
            int EqualsIdx = KwargString.IndexOf('=');
            Kwargs.Add(KwargString.Substring(0, EqualsIdx).Trim(), KwargString.Substring(EqualsIdx + 1, KwargString.Length - EqualsIdx - 1));
        }
        return Kwargs;
    }
}

public class CLIApplication : IEquatable<CLIApplication>
{
    public TextWriter Out { get; set; } = Console.Out;
    public TextWriter Error { get; set; } = Console.Error;
    public TextReader In { get; set; } = Console.In;
    public string Name { get; set; } = "CLI App";
    public string EntryMarker { get; set; } = "> ";
    public string FlagDelimiter { get; set; } = "--";
    public CommandHandler? AnyHandler = null;
    public bool Stop = false;
    public List<object> Commands { get; set; } = new List<object>();

    public CLIApplication()
    {
    }

    public CLIApplication(TextWriter? Out = null, TextWriter? Error = null, TextReader? In = null, string Name = "CLI Application", string EntryMarker = "> ", string FlagDelimiter = "--", CommandHandler? AnyHandler = null, List<object>? Commands = null, bool Stop = false)
    {
        if (Out is not null) this.Out = Out;
        if (Error is not null) this.Error = Error;
        if (In is not null) this.In = In;
        this.Name = Name;
        this.EntryMarker = EntryMarker;
        this.FlagDelimiter = FlagDelimiter;
        this.AnyHandler = AnyHandler;
        this.Stop = Stop;
    }

    public void ShowError(string Message) { Error.WriteLine(Message); }

    public void ShowError<T>(T Exception) where T: Exception { Error.WriteLine($"{typeof(T).Name}: {Exception.Message}"); }

    public void ShowHelp(Command Command) { Out.WriteLine($"    {Command.Name}: {Command.Description}"); }

    public void ShowHelp()
    {
        Out.WriteLine($"{Name} Help:");
        foreach (Command Command in _Commands) ShowHelp(Command);
    }

    CommandHandlerArgs? GetHandlerArgs(string? Input, Command? Command = null)
    {
        if (string.IsNullOrEmpty(Input)) return null;
        List<string> Arguments = new();
        List<string> Flags = new();

        foreach (string QuoteSplit in Input.Split('"'))
        {
            if (string.IsNullOrEmpty(QuoteSplit)) continue;
            if (string.IsNullOrWhiteSpace(QuoteSplit)) continue;
            bool Quoted = false;
            int SplitIndex = Input.IndexOf(QuoteSplit);
            if (SplitIndex > 0 && SplitIndex + QuoteSplit.Length < Input.Length) if (Input[SplitIndex - 1] == '"' && Input[SplitIndex + QuoteSplit.Length] == '"') Quoted = true;

            if (Quoted) Arguments.Add(QuoteSplit);
            else foreach (string SpaceSplit in QuoteSplit.Split(' '))
                {
                    if (string.IsNullOrWhiteSpace (SpaceSplit)) continue;
                    (SpaceSplit.StartsWith(FlagDelimiter) ? Flags : Arguments).Add(SpaceSplit);
                }
        }

        return new CommandHandlerArgs() { Application = this, Command = Command, Arguments = Arguments.ToArray(), Flags = Flags.ToArray(), Input = Input };
    }

    void Get()
    {
    ReEnter:
        Out.Write($"{Name}{EntryMarker}");
        CommandHandlerArgs? Parsed = GetHandlerArgs(In.ReadLine());
        if (Parsed is null) goto ReEnter;
        if (Parsed.Arguments is null) goto ReEnter;
        if (Parsed.Arguments.Length == 0) goto ReEnter;

        if (Parsed.Arguments[0] == "help")
        {
            ShowHelp();
            return;
        }

        string[]? Arguments = Parsed.Arguments.Length > 1 ? new string[Parsed.Arguments.Length - 1] : null;
        if (Arguments is not null) Array.Copy(Parsed.Arguments, 1, Arguments, 0, Parsed.Arguments.Length - 1); //Test

        Command? Command = _Commands.Find(Cmd => Cmd.Name == Parsed.Arguments[0]);
        if (Command is null)
        {
            ShowError($"Command '{Parsed.Arguments[0]}' does not exist.");
            goto ReEnter;
        }

        CommandHandlerArgs HandlerArgs = new () { Application = this, Command = Command, Arguments = Arguments, Flags = Parsed.Flags, Input = Parsed.Input };
        if (AnyHandler is not null) AnyHandler(HandlerArgs);
        if (Command.Handler is not null) Command.Handler(HandlerArgs);
    }

    public void RunCommand(Command Command, bool ShowHelp = false)
    {
        Out.WriteLine($"Enter Arguments For: {Command.Name}");
        if (ShowHelp) this.ShowHelp(Command);
    ReEnter:
        CommandHandlerArgs? Parsed = GetHandlerArgs(In.ReadLine(), Command);
        if (Parsed is null) goto ReEnter;
        if (Parsed.Arguments is null) goto ReEnter;
        if (Parsed.Arguments.Length == 0) goto ReEnter;
        if (Command.Handler is not null) Command.Handler(Parsed);
    }

    public void Run()
    {
        while (!Stop) Get();
    }

    public bool Equals(CLIApplication? Other)
    {
        if (Other == null) return false;
        return Name.Equals(Other.Name) && In.Equals(Other.In) && Out.Equals(Other.In) && Error.Equals(Other.In) && _Commands.Equals(Other._Commands);
    }
}