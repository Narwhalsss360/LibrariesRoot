public class CommandHandlerArgs : IEquatable<CommandHandlerArgs>
{
    public CLIApplication? Application { get; init; }= null;
    public Command? Command { get; init; }= null;
    public string[]? Arguments { get; init; }= null;
    public string[]? Flags { get; init; }= null;
    public string? Input { get; init; }= null;

    public bool Equals(CommandHandlerArgs? Other)
    {
        if (Other == null) return false;
        return Application == Other.Application && Command == Other.Command && Arguments == Other.Arguments && Flags == Other.Flags && Input == Other.Input;
    }
}