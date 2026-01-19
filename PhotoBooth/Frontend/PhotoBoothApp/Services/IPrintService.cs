using System.Collections.Generic;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using PhotoBooth.Models;

namespace PhotoBooth.Services
{
    public interface IPrintService
    {
        event EventHandler<PrintStatusEventArgs> PrintStatusChanged;

        List<PrinterInfo> GetAvailablePrinters();
        bool SetDefaultPrinter(string printerName);
        PrinterInfo GetDefaultPrinter();

        Task<bool> PrintImageAsync(BitmapImage image, PrintSettings settings = null);
        Task<bool> PrintImagesAsync(BitmapImage[] images, PrintSettings settings = null);
        
        bool TestPrint();
        PrinterStatus GetPrinterStatus(string printerName);
        
        int GetPrintQueueCount();
        void ClearPrintQueue();
    }

    public class PrintStatusEventArgs : EventArgs
    {
        public string PrinterName { get; set; }
        public PrintStatus Status { get; set; }
        public string Message { get; set; }
    }

    public enum PrintStatus
    {
        Idle,
        Printing,
        Completed,
        Error,
        OutOfPaper,
        OutOfInk
    }
}
