using ConsoleApp1;
using System.Text;

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
        DownloadMedia,
        SaveMedia,
        SetMediaAsBackground,
        SaveItem,
        ShowItem,
        SetLogPath,
        ExitCLI
    }
};

void ExecutingCommand(CommandHandlerArgs Args)
{
    if (CommandExecutionLog == null) return;
    string Log = $"{DateTime.Now.ToString("g")}: Executing {Args.Executing.GetCommandName()} with\n   Args:{(Args.Arguments is null ? "" : Args.Arguments.ToStringCustomFormat())}\n   Flags:{(Args.Flags is null ? "" : Args.Flags.ToStringCustomFormat())}\n   Input:\"{Args.Input}\"";
    CommandExecutionLog.WriteLine(Log);
    CommandExecutionLog.Flush();
}

[Command("download-media", "<url: string> Downloads the media from the providied url.")]
void DownloadMedia(CommandHandlerArgs Args)
{
    if (Args.Arguments is null || Args.Arguments.Length == 0)
    {
        Args.Application.Error.WriteLine("Must enter a url");
        Args.Application.RunCommand(Args.Executing, true);
        return;
    }

    string Url = Args.Arguments[0];

    if (Args.Input.HasKeywordArguments())
    {
        Dictionary<string, string> Kwargs = Args.Arguments.GetKeywordArguments();
        if (Kwargs.ContainsKey("url"))
        {
            Url = Kwargs["url"];
        }
    }

    Uri URI;
    if (!Uri.TryCreate(Url, UriKind.RelativeOrAbsolute, out URI))
    {
        Args.Application.Error.WriteLine("Invalid url");
        return;
    }

    Args.Application.Out.WriteLine("Downloading...");
    Task<byte[]> DownloadTask = new HttpClient().GetByteArrayAsync(URI);
    while (!DownloadTask.IsCompleted);
    MediaBytes = DownloadTask.Result;
    Args.Application.Out.WriteLine($"Download Complete {MediaBytes.Length} bytes.");
}

[Command("save-media", " <path? => Desktop\\media: string> Saves the media at the provided path.")]
void SaveMedia(CommandHandlerArgs Args)
{
    if (MediaBytes == null)
    {
        Args.Application.ShowError("Must download media first!");
        Args.Application.ShowHelp(DownloadMedia);
        return;
    }

    string? SavePath = null;
    if (Args.Arguments is not null && Args.Arguments.Length > 0)
    {
        if (Args.Input.HasKeywordArguments())
        {
            Dictionary<string, string> Kwargs = Args.Arguments.GetKeywordArguments(); ;
            if (Kwargs.ContainsKey("path"))
            {
                SavePath = Kwargs["path"];
            }
        }

    }
    else
    {
        SavePath = Environment.GetFolderPath(Environment.SpecialFolder.Desktop) + "\\media";
        Args.Application.Out.WriteLine($"Not specified path, saving as {SavePath}");
    }

    if (!ContentUtility.IsValidPath(SavePath))
    {
        Args.Application.Error.WriteLine($"Could not save {SavePath}");
        Args.Application.RunCommand(Args.Executing, true);
        return;
    }

    Args.Application.Out.WriteLine("Saving...");
    using (var FS = File.Create(SavePath)) FS.Write(MediaBytes, 0, MediaBytes.Length);
    Args.Application.Out.WriteLine("Saved!");
}

[Command("set-background", "<style? => centered [centered, tiled, stretched]: string> Sets the current downloaded media as the desktop wallpaper.")]
void SetMediaAsBackground(CommandHandlerArgs Args)
{
    if (MediaBytes == null)
    {
        Args.Application.ShowError("Must download media first!");
        Args.Application.ShowHelp(DownloadMedia);
        return;
    }

    ContentUtility.WallpaperStyle Style = ContentUtility.WallpaperStyle.Centered;

    if (Args.Arguments is not null && Args.Arguments.Length > 0)
    {
        if (Args.Input.HasKeywordArguments())
        {
            Dictionary<string, string> Kwargs = Args.Arguments.GetKeywordArguments();
            if (Kwargs.ContainsKey("style"))
            {
                ContentUtility.WallpaperStyle? NewStyle = Kwargs["style"].GetStyle();
                Style = NewStyle is null ? Style : NewStyle.Value;
            }
        }
        else
        {
            ContentUtility.WallpaperStyle? NewStyle = Args.Arguments[0].GetStyle();
            Style = NewStyle is null ? Style : NewStyle.Value;
        }
    }

    ContentUtility.SetWallpaperAs(MediaBytes, Style);
    Args.Application.Out.WriteLine("Done.");
}

[Command("save-item", "<key: string> <value: string> Saves the value to the key.")]
void SaveItem(CommandHandlerArgs Args)
{
    if (Args.Arguments is null || Args.Arguments.Length < 2)
    {
        Args.Application.RunCommand(Args.Executing, true);
        return;
    }

    if (UserSavedItems.ContainsKey(Args.Arguments[0]))
    {
        Args.Application.Error.WriteLine($"Key already exists:{Args.Application.Error.NewLine}    {Args.Arguments[0]}: {UserSavedItems[Args.Arguments[0]]}");
        return;
    }

    UserSavedItems.Add(Args.Arguments[0], Args.Arguments[1]);
}

[Command("show-item", "<key? => all> Shows the value of the key.")]
void ShowItem(CommandHandlerArgs Args)
{
    if (Args.Arguments is null || Args.Arguments.Length == 0)
    {
        foreach (var item in UserSavedItems) Args.Application.Out.WriteLine($"{item.Key}: {item.Value}");
    }
    else
    {
        if (!UserSavedItems.ContainsKey(Args.Arguments[0]))
        {
            Args.Application.Error.WriteLine($"{Args.Arguments[0]} key does not exist!");
            return;
        }

        Args.Application.Out.WriteLine($"{Args.Arguments[0]}: {UserSavedItems[Args.Arguments[0]]}");
    }
}

[Command("set-log-path", "<path> Sets the path of the current log")]
void SetLogPath(CommandHandlerArgs Args)
{
    if (Args.Arguments is null || Args.Arguments.Length < 1)
    {
        Args.Application.Out.WriteLine("Must enter a path!");
        Args.Application.RunCommand(Args.Executing, true);
        return;
    }

    string? SavePath = Args.Arguments[0];
    if (Args.Arguments is not null && Args.Arguments.Length > 0)
    {
        if (Args.Input.HasKeywordArguments())
        {
            Dictionary<string, string> Kwargs = Args.Arguments.GetKeywordArguments(); ;
            if (Kwargs.ContainsKey("path"))
            {
                SavePath = Kwargs["path"];
            }
        }
    }

    if (!ContentUtility.IsValidPath(SavePath))
    {
        Args.Application.Error.WriteLine($"Please enter a new path. Path Invalid {SavePath}");
        Args.Application.RunCommand(Args.Executing, true);
        return;
    }

    StreamWriter NewLog = new StreamWriter(SavePath);
    if (CommandExecutionLog is null)
    {
        CommandExecutionLog = NewLog;
    }
    else
    {
        Args.Application.Out.WriteLine("Changing location...");
        using (StreamReader Reader = new StreamReader(((FileStream)CommandExecutionLog.BaseStream).Name)) NewLog.Write(Reader.ReadToEnd());
        NewLog.Close();
        NewLog.Dispose();
    }
    Args.Application.Out.WriteLine("Done");
}

[Command("exit", "exits CLI")]
void ExitCLI(CommandHandlerArgs Args)
{
    Args.Application.Stop = true;
}

Application.Run();
CommandExecutionLog.Close();
CommandExecutionLog.Dispose();