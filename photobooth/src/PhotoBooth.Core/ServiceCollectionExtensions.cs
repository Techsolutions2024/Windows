using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using PhotoBooth.Core.Cameras;
using PhotoBooth.Core.Imaging;
using PhotoBooth.Core.Printing;
using PhotoBooth.Core.Sessions;

namespace PhotoBooth.Core;

public static class ServiceCollectionExtensions
{
    public static IServiceCollection AddPhotoBooth(this IServiceCollection services, IConfiguration configuration)
    {
        services.AddSingleton<TimeProvider>(TimeProvider.System);

        services.AddSingleton<SessionStore>();

        services.AddSingleton<IImageRenderer, ImageRenderer>();
        services.AddSingleton<IPrinter, FilePrinter>();

        // Default to a mock camera so the app runs without hardware.
        // Swap to GPhoto2Camera by setting Camera:Driver = gphoto2.
        var driver = configuration["Camera:Driver"]?.Trim().ToLowerInvariant();
        if (driver == "gphoto2")
        {
            services.AddSingleton<ICamera, GPhoto2Camera>();
        }
        else
        {
            services.AddSingleton<ICamera, MockCamera>();
        }

        services.AddSingleton<PhotoBoothService>();
        return services;
    }
}

