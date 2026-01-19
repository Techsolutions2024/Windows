namespace PhotoBooth.Core.Printing;

/// <summary>
/// Development printer: writes the print payload to disk.
/// In production you can implement:
/// - CUPS IPP printing
/// - Windows spooler printing
/// - DNP/HiTi SDK integrations
/// </summary>
public sealed class FilePrinter : IPrinter
{
    private readonly string _outputDir;

    public FilePrinter()
    {
        _outputDir = Path.Combine(AppContext.BaseDirectory, "prints");
        Directory.CreateDirectory(_outputDir);
    }

    public async Task<PrintResult> PrintAsync(PrintRequest request, CancellationToken cancellationToken)
    {
        var name = $"{Path.GetFileNameWithoutExtension(request.FileName)}-printed{Path.GetExtension(request.FileName)}";
        var path = Path.Combine(_outputDir, name);
        await File.WriteAllBytesAsync(path, request.Bytes, cancellationToken);
        return new PrintResult(name, request.NowUtc);
    }
}

