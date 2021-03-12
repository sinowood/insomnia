// Copyright (C) Microsoft Corporation. All Rights Reserved.
// This code released under the terms of the Microsoft Public License
// (Ms-PL, http://opensource.org/licenses/ms-pl.html).

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Navigation;
using Delay;

namespace Insomnia
{
    public partial class Window1 : Window
    {
        private uint m_previousExecutionState;

        public Window1()
        {
            InitializeComponent();
            
            // Enable "minimize to tray" behavior for this Window
            MinimizeToTray.Enable(this);

            // Set new state to prevent system sleep (note: still allows screen saver)
            m_previousExecutionState = NativeMethods.SetThreadExecutionState(
                NativeMethods.ES_CONTINUOUS | NativeMethods.ES_SYSTEM_REQUIRED);
            if (0 == m_previousExecutionState)
            {
                MessageBox.Show("Call to SetThreadExecutionState failed unexpectedly.",
                    Title, MessageBoxButton.OK, MessageBoxImage.Error);
                // No way to recover; fail gracefully
                Close();
            }

            // Start minimized if requested via the command-line
            foreach (var arg in Environment.GetCommandLineArgs())
            {
                if (0 == string.Compare(arg, "-minimize", true))
                {
                    Loaded += delegate { WindowState = WindowState.Minimized; };
                }
            }
        }

        protected override void OnClosed(System.EventArgs e)
        {
            base.OnClosed(e);

            // Restore previous state
            if (0 == NativeMethods.SetThreadExecutionState(m_previousExecutionState))
            {
                // No way to recover; already exiting
            }
        }

        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            // Start an instance of the NavigateUri (in a browser window)
            Process.Start(((Hyperlink)sender).NavigateUri.ToString());
        }
    }

    internal static class NativeMethods
    {
        // Import SetThreadExecutionState Win32 API and necessary flags
        [DllImport("kernel32.dll")]
        public static extern uint SetThreadExecutionState(uint esFlags);
        public const uint ES_CONTINUOUS = 0x80000000;
        public const uint ES_SYSTEM_REQUIRED = 0x00000001;
    }
}
