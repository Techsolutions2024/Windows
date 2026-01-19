using System.Diagnostics;

namespace PhotoBooth.Core.Cameras;

/// <summary>
/// Minimal gphoto2 adapter for Linux. Requires `gphoto2` installed and a supported camera connected.
/// It captures to a temporary folder and reads the file bytes.
/// </summary>
public sealed class GPhoto2Camera : ICamera
{
    public async Task<CaptureResult> CaptureAsync(CancellationToken cancellationToken)
    {
        var capturedAt = DateTimeOffset.UtcNow;
        var baseDir = Path.Combine(Path.GetTempPath(), "photobooth", "gphoto2", capturedAt.ToString("yyyyMMdd"));
        Directory.CreateDirectory(baseDir);

        var fileName = $"capture-{capturedAt:yyyyMMdd-HHmmss}.jpg";
        var fullPath = Path.Combine(baseDir, fileName);

        // gphoto2 creates the file; --filename supports absolute paths.
        var args = $"--capture-image-and-download --force-overwrite --filename \"{fullPath}\"";
        var psi = new ProcessStartInfo
        {
            FileName = "gphoto2",
            Arguments = args,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };

        using var process = Process.Start(psi) ?? throw new InvalidOperationException("Failed to start gphoto2");

        var stdoutTask = process.StandardOutput.ReadToEndAsync(cancellationToken);
        var stderrTask = process.StandardError.ReadToEndAsync(cancellationToken);

        await process.WaitForExitAsync(cancellationToken);
        var stdout = await stdoutTask;
        var stderr = await stderrTask;

        if (process.ExitCode != 0)
        {
            throw new InvalidOperationException($"gphoto2 failed ({process.ExitCode}).\nSTDOUT: {stdout}\nSTDERR: {stderr}");
        }

        var bytes = await File.ReadAllBytesAsync(fullPath, cancellationToken);
        return new CaptureResult(fileName, "image/jpeg", bytes, capturedAt);
    }
}

