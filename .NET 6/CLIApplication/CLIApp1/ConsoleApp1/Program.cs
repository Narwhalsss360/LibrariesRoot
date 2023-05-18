using ConsoleApp1;

#error Implement library using MethodAttribute

byte[]? MediaBytes = null;
Dictionary<string, string> UserSavedItems = new Dictionary<string, string>();

Command Command_DownloadMedia = new()
{
    Name = "download-media",
    Description = "<url: string, key> Downloads media at `url`.",
    Handler = DownloadMedia
};

Command Command_SaveMedia = new()
{
    Name = "save-media",
    Description = "<path? -> Desktop\\media: string> saves media at `path`.",
    Handler = SaveMedia
};

Command Command_SetMediaAsBackground = new()
{
    Name = "set-bg",
    Description = "<style? -> centered: WallpaperStyle, key> sets desktop background as media.",
    Handler = SetMediaAsBackground
};

Command Command_SaveItem = new()
{
    Name = "save-item",
    Description = "<key> <value> saves the value to the key.",
    Handler = SaveItem
};

Command Command_ShowItem = new()
{
    Name = "show-item",
    Description = "<key? -> all> show the value of the key.",
    Handler = ShowItem
};

CLIApplication App = new() { Name = "Web Media Helper", EntryMarker = " -> ", Commands = { Command_DownloadMedia, Command_SaveMedia, Command_SetMediaAsBackground, Command_SaveItem, Command_ShowItem } };

void DownloadMedia(CommandHandlerArgs Args)
{
    if (Args.Arguments is null || Args.Arguments.Length == 0)
    {
        Args.Application.Error.WriteLine("Must enter a url");
        Args.Application.RunCommand(Args.Command, true);
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

void SaveMedia(CommandHandlerArgs Args)
{
    if (MediaBytes == null)
    {
        Args.Application.ShowError("Must download media first!");
        Args.Application.ShowHelp(Command_DownloadMedia);
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
        Args.Application.RunCommand(Args.Command, true);
        return;
    }

    Args.Application.Out.WriteLine("Saving...");
    using (var FS = File.Create(SavePath)) FS.Write(MediaBytes, 0, MediaBytes.Length);
    Args.Application.Out.WriteLine("Saved!");
}

void SetMediaAsBackground(CommandHandlerArgs Args)
{
    if (MediaBytes == null)
    {
        Args.Application.ShowError("Must download media first!");
        Args.Application.ShowHelp(Command_DownloadMedia);
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

void SaveItem(CommandHandlerArgs Args)
{
    if (Args.Arguments is null || Args.Arguments.Length < 2)
    {
        Args.Application.RunCommand(Args.Command, true);
        return;
    }

    if (UserSavedItems.ContainsKey(Args.Arguments[0]))
    {
        Args.Application.Error.WriteLine($"Key already exists:{Args.Application.Error.NewLine}    {Args.Arguments[0]}: {UserSavedItems[Args.Arguments[0]]}");
        return;
    }

    UserSavedItems.Add(Args.Arguments[0], Args.Arguments[1]);
}

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

[CommandFunction("attribute-command", "attribute-command description")]
void AttributeTestFunction(CommandHandlerArgs Args)
{
    Args.Application.Out.WriteLine("Called attribute test function");
}

CLIApplication App2 = new CLIApplication() { Name = "CLIApp2", Commands = { AttributeTestFunction } };

//App.Run();