using Microsoft.Win32;
using System.Runtime.InteropServices;

namespace ConsoleApp1
{

    public static class ContentUtility
    {
        public enum WallpaperStyle : int
        {
            Tiled,
            Centered,
            Stretched
        }

        public static WallpaperStyle? GetStyle(this string Style)
        {
            Style = Style.Trim().ToLower();

            if (Style == "tiled") return WallpaperStyle.Tiled;
            else if (Style == "centered") return WallpaperStyle.Centered;
            else if (Style == "stretched") return WallpaperStyle.Stretched;
            else return null;
        }

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static extern int SystemParametersInfo(int uAction, int uParam, string lpvParam, int fuWinIni);

        public static void SetWallpaperAs(byte[] ImageBytes, WallpaperStyle Style = WallpaperStyle.Centered)
        {
            const int SPI_SETDESKWALLPAPER = 20;
            const int SPIF_UPDATEINIFILE = 0x01;
            const int SPIF_SENDWININICHANGE = 0x02;

            string ImageTempPath = $"{Path.GetTempPath()}\\APODImage.jpg";
            using (var FS = File.Create(ImageTempPath))
            {
                FS.Write(ImageBytes, 0, ImageBytes.Length);
            }

            RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Control Panel\Desktop", true);

            if (Style == WallpaperStyle.Stretched)
            {
                key.SetValue(@"WallpaperStyle", 2.ToString());
                key.SetValue(@"TileWallpaper", 0.ToString());
            }

            if (Style == WallpaperStyle.Centered)
            {
                key.SetValue(@"WallpaperStyle", 1.ToString());
                key.SetValue(@"TileWallpaper", 0.ToString());
            }

            if (Style == WallpaperStyle.Tiled)
            {
                key.SetValue(@"WallpaperStyle", 1.ToString());
                key.SetValue(@"TileWallpaper", 1.ToString());
            }

            SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, ImageTempPath, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
        }

        public static bool IsValidPath(string path, bool allowRelativePaths = false)
        {
            bool isValid = true;

            try
            {
                string fullPath = Path.GetFullPath(path);

                if (allowRelativePaths)
                {
                    isValid = Path.IsPathRooted(path);
                }
                else
                {
                    string root = Path.GetPathRoot(path);
                    isValid = string.IsNullOrEmpty(root.Trim(new char[] { '\\', '/' })) == false;
                }
            }
            catch (Exception ex)
            {
                isValid = false;
            }

            return isValid;
        }

    }
}