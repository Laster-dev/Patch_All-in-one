using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Security.Cryptography.X509Certificates;
using System.Diagnostics;
using System.Security.Cryptography;

class Program
{
    static void Main()
    {
        string targetDirectory = @".\w";
        Directory.CreateDirectory(targetDirectory);

        string[] drives = Environment.GetLogicalDrives();

        foreach (var drive in drives)
        {
            try
            {
                ScanDirectory(drive, targetDirectory);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error scanning drive {drive}: {ex.Message}");
            }
        }

        Console.WriteLine("扫描完成");
        Console.ReadLine();
    }

    static void ScanDirectory(string directory, string targetDirectory)
    {
        try
        {
            foreach (string file in Directory.GetFiles(directory, "*.exe", SearchOption.AllDirectories))
            {
                try
                {
                    if (IsDotNetAssembly(file) && HasDigitalSignature(file))
                    {
                        string targetPath = Path.Combine(targetDirectory, Path.GetFileName(file));
                        File.Copy(file, targetPath, true);
                        Console.WriteLine($"复制文件: {file}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error processing file {file}: {ex.Message}");
                }
            }

            foreach (string subDir in Directory.GetDirectories(directory))
            {
                ScanDirectory(subDir, targetDirectory);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error accessing directory {directory}: {ex.Message}");
        }
    }

    static bool IsDotNetAssembly(string filePath)
    {
        try
        {
            AssemblyName assemblyName = AssemblyName.GetAssemblyName(filePath);
            return true;
        }
        catch (BadImageFormatException)
        {
            return false;
        }
        catch (FileLoadException)
        {
            return false;
        }
        catch (Exception)
        {
            return false;
        }
    }

    static bool HasDigitalSignature(string filePath)
    {
        try
        {
            X509Certificate cert = X509Certificate.CreateFromSignedFile(filePath);
            return cert != null;
        }
        catch (CryptographicException)
        {
            return false;
        }
        catch (Exception)
        {
            return false;
        }
    }
}
