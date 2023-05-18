using System.Reflection;

public delegate void CommandHandler(CommandHandlerArgs HandlerArgs);

public static class CommandHandlerExtension
{
    public static string GetCommandName(this CommandHandler Handler)
    {
        return CommandAttribute.GetAttribute(Handler).Name;
    }

    public static string GetCommandDescription(this CommandHandler Handler)
    {
        return CommandAttribute.GetAttribute(Handler).Description;
    }
}

[AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
public class CommandAttribute : Attribute
{
    public readonly string Name;
    public readonly string Description;

    public CommandAttribute(string Name, string Description)
    {
        this.Name = Name;
        this.Description = Description;
    }

    public static CommandAttribute GetAttribute(CommandHandler Handler)
    {
        return (CommandAttribute)GetCustomAttribute(MethodBase.GetMethodFromHandle(Handler.Method.MethodHandle), typeof(CommandAttribute));
    }
}