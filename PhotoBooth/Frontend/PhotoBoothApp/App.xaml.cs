using System;
using System.Windows;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using PhotoBooth.Services;
using PhotoBooth.ViewModels;
using PhotoBooth.Views;

namespace PhotoBooth
{
    public partial class App : Application
    {
        private readonly IHost _host;

        public App()
        {
            _host = Host.CreateDefaultBuilder()
                .ConfigureServices((context, services) =>
                {
                    // Services
                    services.AddSingleton<ICameraService, CameraService>();
                    services.AddSingleton<IImageProcessingService, ImageProcessingService>();
                    services.AddSingleton<IPrintService, PrintService>();
                    services.AddSingleton<IConfigurationService, ConfigurationService>();
                    services.AddSingleton<ITemplateService, TemplateService>();
                    services.AddSingleton<ISharingService, SharingService>();
                    services.AddSingleton<ISessionService, SessionService>();

                    // ViewModels
                    services.AddTransient<MainViewModel>();
                    services.AddTransient<GuestDisplayViewModel>();
                    services.AddTransient<SettingsViewModel>();
                    services.AddTransient<TemplateEditorViewModel>();

                    // Views
                    services.AddSingleton<MainWindow>();
                    services.AddTransient<GuestDisplayWindow>();
                    services.AddTransient<SettingsWindow>();
                    services.AddTransient<TemplateEditorWindow>();
                })
                .Build();
        }

        protected override async void OnStartup(StartupEventArgs e)
        {
            await _host.StartAsync();

            var mainWindow = _host.Services.GetRequiredService<MainWindow>();
            mainWindow.Show();

            base.OnStartup(e);
        }

        protected override async void OnExit(ExitEventArgs e)
        {
            using (_host)
            {
                await _host.StopAsync(TimeSpan.FromSeconds(5));
            }

            base.OnExit(e);
        }

        public static T GetService<T>() where T : class
        {
            return ((App)Current)._host.Services.GetRequiredService<T>();
        }
    }
}
