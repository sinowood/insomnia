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
        private string m_title;
        private string m_info_text;
        private string m_btn_text;
        private int m_countdown_seconds;
        private System.Windows.Threading.DispatcherTimer m_timer;

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
            //MessageBox.Show("OnClosed", Title, MessageBoxButton.OK);
        }

        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            // Start an instance of the NavigateUri (in a browser window)
            Process.Start(((Hyperlink)sender).NavigateUri.ToString());
        }

        protected override void OnInitialized(EventArgs e)
        {
            base.OnInitialized(e);

            this.m_title = this.Title.ToString();
            this.m_info_text = this.s_info.Content.ToString();
            this.m_btn_text = this.btn_set.Content.ToString();

            for (int i = 0; i < 12; i++)
            {
                i_hours.Items.Add(i * 2);
                i_hours.Items.Add(i * 2 + 1);
                i_minutes.Items.Add(i * 5);
                //if (i == 0) i_minutes.Items.Add(i + 1);
            }
            i_hours.SelectedIndex = i_minutes.SelectedIndex = 0;

            this.m_timer = new System.Windows.Threading.DispatcherTimer();
            this.m_timer.Interval = TimeSpan.FromSeconds(1);
            this.m_timer.Tick += delegate
            {
                this.m_countdown_seconds -= 1;
                if (this.m_countdown_seconds <= 0)
                {
                    // Exit if timer is expired
                    this.Close();
                }
                this.set_info();
            };
        }

        private void set_info()
        {
            int hours = this.m_countdown_seconds / 3600;
            int minutes = (this.m_countdown_seconds % 3600) / 60;
            int seconds = this.m_countdown_seconds % 60;
            this.s_info.Content = string.Format("in {0}:{1:D2}:{2:D2}", hours, minutes, seconds);
        }

        private void btn_set_Click(object sender, RoutedEventArgs e)
        {
            if (!this.m_timer.IsEnabled)
            {
                this.set_countdown();
            }
            else
            {
                this.clear_countdown();
            }
        }

        private void set_countdown()
        {
            // Set Countdown Timer
            int m_hours = int.Parse(this.i_hours.SelectedValue.ToString());
            int m_minutes = int.Parse(this.i_minutes.SelectedValue.ToString());
            this.m_countdown_seconds = (m_hours * 60 + m_minutes) * 60;
            if (this.m_countdown_seconds > 0)
            {
                if (this.m_timer.IsEnabled) this.m_timer.Stop();
                this.set_info();
                this.m_timer.Start();
                this.btn_set.Content = "clear timer";
                this.Title = this.m_title + ": +Timer";
            }
        }

        private void clear_countdown()
        {
            // Clear Countdown Timer
            if (this.m_timer.IsEnabled) this.m_timer.Stop();
            this.btn_set.Content = this.m_btn_text;
            this.s_info.Content = this.m_info_text;
            this.Title = this.m_title;
            this.update_btn_status();
        }

        private void update_btn_status()
        {
            this.btn_set.IsEnabled = ( this.i_hours.SelectedIndex > 0 || this.i_minutes.SelectedIndex > 0 
                || (this.m_timer != null && this.m_timer.IsEnabled) );
        }

        private void timer_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            this.update_btn_status();
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
