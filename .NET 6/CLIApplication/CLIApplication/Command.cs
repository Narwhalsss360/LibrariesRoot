public delegate void CommandHandler(CommandHandlerArgs Handler);

[AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
public class CommandFunctionAttribute : Attribute
{
    public readonly string Name;
    public readonly string Description;

    public CommandFunctionAttribute(string Name, string Description)
    {
        this.Name = Name;
        this.Description = Description;
    }
}

public class Command : IEquatable<Command>, IComparable<Command>
{
    public string? Name { get; init; } = null;
    public string? Description { get; init; } = null;
    public CommandHandler? Handler { get; init; } = null;

    public static Command? GetFromAttributes(CommandHandler Handler)
    {
        System.Reflection.MethodBase? MethodBase = System.Reflection.MethodBase.GetMethodFromHandle(Handler.Method.MethodHandle);
        if (MethodBase is null) return null;

        CommandFunctionAttribute? CommandFunctionAttributes = null;
        foreach (object HandlerAttribute in System.Reflection.MethodBase.GetMethodFromHandle(Handler.Method.MethodHandle).GetCustomAttributes(true))
        {
            CommandFunctionAttributes = HandlerAttribute as CommandFunctionAttribute;
            if (CommandFunctionAttributes is not  null) break;
        }
        if (CommandFunctionAttributes is null) return null;
        return new Command() { Name = CommandFunctionAttributes.Name, Description = CommandFunctionAttributes.Description, Handler = Handler };
    }

    public bool Equals(Command? Other)
    {
        if (Other == null) return false;
        if (Other.Name == null) return false;
        return Other.Name == Name;
    }

    public int CompareTo(Command? Other)
    {
        if (Other == null) return 1;
        if (Other.Name == null) return 1;
        return Other.Name.CompareTo(Name);
    }
}