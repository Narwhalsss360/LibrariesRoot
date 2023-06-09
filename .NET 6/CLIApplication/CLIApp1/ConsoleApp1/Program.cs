using ConsoleApp1;

byte[]? MediaBytes = null;
Dictionary<string, string> UserSavedItems = new Dictionary<string, string>();
StreamWriter? CommandExecutionLog = new StreamWriter(Environment.GetFolderPath(Environment.SpecialFolder.Desktop) + "\\ExecutionLog.txt", true);

CLIApplication Application = new()
{
    Name = "ConsoleApp1",
    EntryMarker = " -> ",
    AnyHandler = ExecutingCommand,
    Commands =
    {
        new Command
            (
                "download-media", DownloadMedia, "Downloads media at the specified <url>.", new ArgumentDefinition[] { new ArgumentDefinition(typeof(string), true, "url") }
            ),
        new Command
            (
                "save-media", SaveMedia, "Saves downloaded media at the specified <path?> defaults to Desktop\\Media.", new ArgumentDefinition[] { new ArgumentDefinition(typeof(string), false, "path") }
            ),
        new Command
            (
                "set-background", SetMediaAsBackground, "Sets the downloaded media as the desktop background with the specified <style>.", new ArgumentDefinition[] { new ArgumentDefinition(typeof(string), false, "style") }
            ),
        new Command
            (
                "save-item", SaveItem, "Saves a <value> to a <key>.", new ArgumentDefinition[] { new ArgumentDefinition(typeof(string), true, "key"), new ArgumentDefinition(typeof(string), true, "value") }
            ),
        new Command
            (
                "show-item", ShowItem, "Shows a <value> of a <key?> or all values.", new ArgumentDefinition[] { new ArgumentDefinition(typeof(string), false, "key") }
            ),
        new Command
            (
                "set-log-path", SetLogPath, "Sets the <path> of the log file.", new ArgumentDefinition[] { new ArgumentDefinition(typeof(string), true, "path") }
            ),
        new Command
            (
                "exit", ExitCLI, "Exits the program", new ArgumentDefinition[0]
            )
    }
};

void ExecutingCommand(Dictionary<object, object?> Arguments, string[]? Flags, object? Caller)
{
    CLIApplication? CallerApplication = Caller as CLIApplication;

    if (CallerApplication is null)
        throw new Exception("Unkown caller!");

    if (CommandExecutionLog == null) return;
    string Log = $"{DateTime.Now.ToString("g")}: Executing {CallerApplication.Executing?.Name} with\n   Args:{Arguments}\n   Flags:{(Flags is null ? "" : Flags.ToStringCustomFormat())}\n";
    CommandExecutionLog.WriteLine(Log);
    CommandExecutionLog.Flush();
}

void DownloadMedia(Dictionary<object, object?> Arguments, string[]? Flags, object? Caller)
{
    CLIApplication? CallerApplication = Caller as CLIApplication;

    if (CallerApplication is null)
        throw new Exception("Unkown caller!");

    if (!Arguments.ContainsKey("url"))
    {
        CallerApplication.Error.WriteLine("Must enter a url");
        if (CallerApplication.Executing is not null)
            CallerApplication.RunCommand(CallerApplication.Executing);
        return;
    }

    string? Url = (string?)Arguments["url"];

    Uri? URI;
    if (!Uri.TryCreate(Url, UriKind.RelativeOrAbsolute, out URI))
    {
        CallerApplication.Error.WriteLine("Invalid url");
        return;
    }

    CallerApplication.Out.WriteLine("Downloading...");
    Task<byte[]> DownloadTask = new HttpClient().GetByteArrayAsync(URI);
    while (!DownloadTask.IsCompleted);
    MediaBytes = DownloadTask.Result;
    CallerApplication.Out.WriteLine($"Download Complete {MediaBytes.Length} bytes.");
}

void SaveMedia(Dictionary<object, object?> Arguments, string[]? Flags, object? Caller)
{
    CLIApplication? CallerApplication = Caller as CLIApplication;

    if (CallerApplication is null)
        throw new Exception("Unkown caller!");

    if (MediaBytes == null)
    {
        CallerApplication.ShowError("Must download media first!");
        CallerApplication.ShowHelp("download-media");
        return;
    }

    string SavePath = "";
    if (Arguments.Count > 0)
    {
        string? Argument = (string?)Arguments["path"];
        if (Argument is not null)
        {
            SavePath = Argument;
            goto SkipSet;
        }
    }

    SavePath = Environment.GetFolderPath(Environment.SpecialFolder.Desktop) + "\\media";
SkipSet:
    CallerApplication.Out.WriteLine($"No specified path, saving as {SavePath}");
    if (!ContentUtility.IsValidPath(SavePath))
    {
        CallerApplication.Error.WriteLine($"Could not save {SavePath}");
        if (CallerApplication.Executing is not null)
            CallerApplication.RunCommand(CallerApplication.Executing);
        return;
    }

    CallerApplication.Out.WriteLine("Saving...");
    using (var FS = File.Create(SavePath)) FS.Write(MediaBytes, 0, MediaBytes.Length);
    CallerApplication.Out.WriteLine("Saved!");
}

void SetMediaAsBackground(Dictionary<object, object?> Arguments, string[]? Flags, object? Caller)
{
    CLIApplication? CallerApplication = Caller as CLIApplication;

    if (CallerApplication is null)
        throw new Exception("Unkown caller!");

    if (MediaBytes == null)
    {
        CallerApplication.ShowError("Must download media first!");
        CallerApplication.ShowHelp("download-media");
        return;
    }

    ContentUtility.WallpaperStyle Style = ContentUtility.WallpaperStyle.Centered;

    if (Arguments.Count > 0)
    {
        string? StyleArgument = null;
        if (!Arguments.ContainsKey("style"))
            goto Invalid;

        StyleArgument = (string?)Arguments["style"];
        if (StyleArgument != null)
        {
            ContentUtility.WallpaperStyle? SpecifiedStyle = StyleArgument.GetStyle();
            if (SpecifiedStyle is null)
                goto Invalid;   
        }
    }

    ContentUtility.SetWallpaperAs(MediaBytes, Style);
    CallerApplication.Out.WriteLine("Done.");
    return;
    Invalid:
    CallerApplication.Error.WriteLine("Invalid style");
}

void SaveItem(Dictionary<object, object?> Arguments, string[]? Flags, object? Caller)
{
    CLIApplication? CallerApplication = Caller as CLIApplication;

    if (CallerApplication is null)
        throw new Exception("Unkown caller!");

    if (Arguments.Count < 2)
    {
        if (CallerApplication.Executing is not null)
            CallerApplication.RunCommand(CallerApplication.Executing);
        return;
    }

    string Key = "";
    string? KeyArgument = (string?)Arguments["key"];
    if (KeyArgument is null)
    {
        CallerApplication.Error.WriteLine("Invalid key argument");
        return;
    }

    Key = KeyArgument;

    if (UserSavedItems.ContainsKey(Key))
    {
        CallerApplication.Error.WriteLine($"Key already exists:{CallerApplication.Error.NewLine}    {Arguments["key"]}: {UserSavedItems[Key]}");
        return;
    }

    string Value = "";
    string? ValueArgument = (string?)Arguments["value"];
    if (ValueArgument is null)
    {
        CallerApplication.Error.WriteLine("Invalid value argument");
        return;
    }

    Value = ValueArgument;

    UserSavedItems.Add(Key, Value);
}

void ShowItem(Dictionary<object, object?> Arguments, string[]? Flags, object? Caller)
{
    CLIApplication? CallerApplication = Caller as CLIApplication;

    if (CallerApplication is null)
        throw new Exception("Unkown caller!");

    if (Arguments.Count == 0)
    {
        foreach (var item in UserSavedItems)
            CallerApplication.Out.WriteLine($"{item.Key}: {item.Value}");
    }
    else
    {
        string? KeyArgument = (string?)Arguments["key"];
        if (KeyArgument is null) 
        {
            CallerApplication.Error.WriteLine("Invalid key argument");
            return;
        }

        string Key = KeyArgument;
        string? Value = (string?)UserSavedItems[Key];

        if (Value is null)
        {
            CallerApplication.Error.WriteLine($"{Key} key does not exist!");
            return;
        }

        CallerApplication.Out.WriteLine($"{Key}: {Value}");
    }
}

void SetLogPath(Dictionary<object, object?> Arguments, string[]? Flags, object? Caller)
{
    CLIApplication? CallerApplication = Caller as CLIApplication;

    if (CallerApplication is null)
        throw new Exception("Unkown caller!");

    if (Arguments.Count < 1)
    {
        CallerApplication.Out.WriteLine("Must enter a path!");
        if (CallerApplication.Executing is not null)
            CallerApplication.RunCommand(CallerApplication.Executing);
        return;
    }

    string? SavePathArgument = (string?)Arguments["path"];

    if (SavePathArgument is null)
    {
        CallerApplication.Error.WriteLine("Save path invalid");
        return;
    }

    string SavePath = SavePathArgument;

    if (!ContentUtility.IsValidPath(SavePath))
    {
        CallerApplication.Error.WriteLine($"Please enter a new path. Path Invalid {SavePath}");
        if (CallerApplication.Executing is not null)
            CallerApplication.RunCommand(CallerApplication.Executing);
        return;
    }

    StreamWriter NewLog = new StreamWriter(SavePath);
    if (CommandExecutionLog is null)
    {
        CommandExecutionLog = NewLog;
    }
    else
    {
        CallerApplication.Out.WriteLine("Changing location...");
        using (StreamReader Reader = new StreamReader(((FileStream)CommandExecutionLog.BaseStream).Name)) NewLog.Write(Reader.ReadToEnd());
        NewLog.Close();
        NewLog.Dispose();
    }
    CallerApplication.Out.WriteLine("Done");
}

void ExitCLI(Dictionary<object, object?> Arguments, string[]? Flags, object? Caller)
{
    CLIApplication? CallerApplication = Caller as CLIApplication;

    if (CallerApplication is null)
        throw new Exception("Unkown caller!");

    CallerApplication.Stop = true;
}

Application.Run();
CommandExecutionLog.Close();
CommandExecutionLog.Dispose();