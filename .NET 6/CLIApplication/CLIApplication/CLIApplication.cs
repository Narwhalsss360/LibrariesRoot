using System.Reflection;

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
    public List<CommandHandler> Commands { get; set; } = new List<CommandHandler>();

    public CLIApplication()
    {
    }

    public CLIApplication(TextWriter? Out = null, TextWriter? Error = null, TextReader? In = null, string Name = "CLI Application", string EntryMarker = "> ", string FlagDelimiter = "--", CommandHandler? AnyHandler = null, List<CommandHandler>? Commands = null, bool Stop = false)
    {
        if (Out is not null) this.Out = Out;
        if (Error is not null) this.Error = Error;
        if (In is not null) this.In = In;
        this.Name = Name;
        this.EntryMarker = EntryMarker;
        this.FlagDelimiter = FlagDelimiter;
        this.AnyHandler = AnyHandler;
        if (Commands is not null) this.Commands = Commands;
        this.Stop = Stop;
    }

    public void ShowError(string Message) { Error.WriteLine(Message); }

    public void ShowError<T>(T Exception) where T: Exception { Error.WriteLine($"{typeof(T).Name}: {Exception.Message}"); }

    public void ShowHelp(CommandHandler Handler) { Out.WriteLine($"    {CommandAttribute.GetAttribute(Handler).Name}: {CommandAttribute.GetAttribute(Handler).Description}"); }

    public void ShowHelp()
    {
        Out.WriteLine($"{Name} Help:");
        foreach (CommandHandler Handler in Commands) ShowHelp(Handler);
    }

    CommandHandlerArgs? GetHandlerArgs(string? Input, CommandHandler? Executing = null)
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

        return new CommandHandlerArgs() { Application = this, Arguments = Arguments.ToArray(), Flags = Flags.ToArray(), Executing = Executing,Input = Input };
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

        CommandHandler? Handler;

        Handler = Commands.Find(H => H.GetCommandName() == Parsed.Arguments[0]);

        if (Handler is null)
        {
            ShowError($"Command '{Parsed.Arguments[0]}' does not exist.");
            goto ReEnter;
        }

        CommandHandlerArgs HandlerArgs = new () { Application = this, Arguments = Arguments, Flags = Parsed.Flags, Executing = Handler, Input = Parsed.Input };
        if (AnyHandler is not null) AnyHandler(HandlerArgs);
        Handler(HandlerArgs);
    }

    public void RunCommand(CommandHandler Handler, bool ShowHelp = false, bool force = false)
    {
        Out.WriteLine($"Enter Arguments For: {Handler.GetCommandName()}");
        if (ShowHelp) this.ShowHelp(Handler);
    ReEnter:
        string? Input = In.ReadLine();
        if (string.IsNullOrEmpty(Input) && !force) return;
        CommandHandlerArgs? Parsed = GetHandlerArgs(Input, Handler);
        if (Parsed is null) goto ReEnter;
        if (Parsed.Arguments is null) goto ReEnter;
        if (Parsed.Arguments.Length == 0) goto ReEnter;
        if (AnyHandler is not null) AnyHandler(Parsed);
        Handler(Parsed);
    }

    public void Run()
    {
        while (!Stop) Get();
    }

    public bool Equals(CLIApplication? Other)
    {
        if (Other == null) return false;
        return Name.Equals(Other.Name) && In.Equals(Other.In) && Out.Equals(Other.In) && Error.Equals(Other.In) && Commands.Equals(Other.Commands);
    }
}