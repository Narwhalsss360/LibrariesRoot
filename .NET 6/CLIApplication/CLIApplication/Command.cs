using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CLIApplication
{
    public delegate void CommandHandler(CommandHandlerArgs Handler);

    public class Command : IEquatable<Command>, IComparable<Command>
    {
        public string? Name { get; init; } = null;
        public string? Description { get; init; } = null;
        public CommandHandler? Handler { get; init; } = null;

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
}