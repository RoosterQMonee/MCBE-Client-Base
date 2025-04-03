using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal;
using Microsoft.VisualBasic;

namespace Injector_cli
{
    internal static class Injector
    {
        static string RunCommand(string command, string args)
        {
            var process = new Process()
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = command,
                    Arguments = args,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                }
            };
            process.Start();


            var output = process.StandardOutput.ReadToEnd();
            var error = process.StandardError.ReadToEnd();

            return string.IsNullOrEmpty(error) ? output : error;
        }

        #region Windows API Imports

        [DllImport("kernel32.dll")]
        private static extern IntPtr OpenProcess(IntPtr dwDesiredAccess, bool bInheritHandle, uint processId);

        [DllImport("kernel32.dll")]
        private static extern bool CloseHandle(IntPtr hObject);

        [DllImport("kernel32.dll")]
        private static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll")]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, char[] lpBuffer, int nSize, out IntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll")]
        private static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, ref IntPtr lpThreadId);

        [DllImport("kernel32.dll")]
        private static extern uint WaitForSingleObject(IntPtr handle, uint milliseconds);

        [DllImport("kernel32.dll")]
        private static extern bool VirtualFreeEx(IntPtr hProcess, IntPtr lpAddress, int dwSize, IntPtr dwFreeType);

        [DllImport("user32.dll")]
        private static extern IntPtr FindWindow(String lpClassName, String lpWindowName);

        [DllImport("user32.dll")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);

        #endregion

        public static bool InjectDll(string dllPath)
        {
            // Validate input file
            if (!File.Exists(dllPath))
            {
                Console.WriteLine($"{dllPath} is not a valid file!");
                return false;
            }

            // Verify minimum DLL size
            if (File.ReadAllBytes(dllPath).Length < 10)
            {
                Console.WriteLine("DLL corrupted");
                return false;
            }

            // Locate target process
            var minecraftProcesses = Process.GetProcessesByName("Minecraft.Windows");
            if (minecraftProcesses.Length == 0)
            {
                Console.WriteLine("Launching Minecraft");

                // Launch Minecraft UWP application
                RunCommand("explorer.exe", "shell:appsFolder\\Microsoft.MinecraftUWP_8wekyb3d8bbwe!App");

                // Wait for Minecraft to start
                Task.Run(async () =>
                {
                    int attempts = 0;
                    while (minecraftProcesses.Length == 0)
                    {
                        if (++attempts > 20)
                            return;

                        minecraftProcesses = Process.GetProcessesByName("Minecraft.Windows");
                        await Task.Delay(100);
                    }
                    await Task.Delay(3000);
                }).Wait();
            }

            // Select responding process
            var targetProcess = minecraftProcesses.FirstOrDefault(p => p.Responding);
            if (targetProcess == null)
            {
                Console.WriteLine("Could not find responding Minecraft process");
                return false;
            }

            try
            {
                // Configure file permissions
                var fileInfo = new FileInfo(dllPath);
                var accessControl = fileInfo.GetAccessControl();
                accessControl.AddAccessRule(new FileSystemAccessRule(
                    new SecurityIdentifier("S-1-15-2-1"),
                    FileSystemRights.FullControl,
                    InheritanceFlags.None,
                    PropagationFlags.NoPropagateInherit,
                    AccessControlType.Allow));
                fileInfo.SetAccessControl(accessControl);
            }
            catch (Exception)
            {
                Console.WriteLine("Failed to set SID, run as administrator");
                return false;
            }

            // Check if DLL is already loaded
            foreach (ProcessModule module in targetProcess.Modules)
            {
                if (module.FileName.Equals(dllPath, StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine($"{dllPath} is already injected!");
                    return false;
                }
            }

            IntPtr processHandle = IntPtr.Zero;
            IntPtr allocatedMemory = IntPtr.Zero;
            IntPtr remoteThread = IntPtr.Zero;

            try
            {
                // Obtain process handle
                processHandle = OpenProcess((IntPtr)0x1F0FFF, false, (uint)targetProcess.Id);
                if (processHandle == IntPtr.Zero || !targetProcess.Responding)
                {
                    Console.WriteLine("Failed to get process handle");
                    return false;
                }

                // Allocate memory in target process
                allocatedMemory = VirtualAllocEx(processHandle, IntPtr.Zero,
                    (uint)(dllPath.Length + 1), 0x1000, 0x40);

                // Write DLL path to allocated memory
                WriteProcessMemory(processHandle, allocatedMemory,
                    dllPath.ToCharArray(), dllPath.Length, out IntPtr bytesWritten);

                // Get LoadLibrary address
                IntPtr loadLibraryAddress = GetProcAddress(
                    GetModuleHandle("kernel32.dll"),
                    "LoadLibraryA");

                // Create remote thread to execute LoadLibrary
                remoteThread = CreateRemoteThread(processHandle,
                    IntPtr.Zero, 0, loadLibraryAddress,
                    allocatedMemory, 0, ref bytesWritten);

                if (remoteThread == IntPtr.Zero)
                {
                    Console.WriteLine("Failed to create thread");
                    return false;
                }

                // Wait for thread completion
                uint waitForResult = WaitForSingleObject(remoteThread, 5000);
                if (waitForResult == 128 || waitForResult == 258)
                {
                    CloseHandle(remoteThread);
                }
                else
                {
                    VirtualFreeEx(processHandle, allocatedMemory, 0, (IntPtr)0x8000);
                    CloseHandle(remoteThread);
                    CloseHandle(processHandle);
                }

                // Bring Minecraft window to foreground
                IntPtr windowHandle = FindWindow(null, "Minecraft");
                if (windowHandle != IntPtr.Zero)
                {
                    SetForegroundWindow(windowHandle);
                }
                else
                {
                    Console.WriteLine("Couldn't get window handle");
                }

                return true;
            }
            finally
            {
                // Cleanup resources
                if (remoteThread != IntPtr.Zero)
                    CloseHandle(remoteThread);
                if (allocatedMemory != IntPtr.Zero)
                    VirtualFreeEx(processHandle, allocatedMemory, 0, (IntPtr)0x8000);
                if (processHandle != IntPtr.Zero)
                    CloseHandle(processHandle);
            }
        }
    }
}