#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>

namespace photobooth {

enum class PrintSize {
    Size4x6,
    Size5x7,
    Size6x8,
    Custom
};

struct PrintJob {
    int id;
    std::string imagePath;
    PrintSize size;
    int copies;
    std::string status; // "pending", "printing", "completed", "failed"
    std::string timestamp;
};

class PrintManager {
public:
    PrintManager();
    ~PrintManager();

    bool initialize();
    void shutdown();

    // Printer management
    std::vector<std::string> getAvailablePrinters();
    bool selectPrinter(const std::string& printerName);
    std::string getSelectedPrinter() const;

    // Print queue
    int addPrintJob(const std::string& imagePath, PrintSize size, int copies = 1);
    bool cancelPrintJob(int jobId);
    void clearQueue();
    std::vector<PrintJob> getQueue();
    int getQueueSize();

    // Printing
    bool print(const std::string& imagePath, PrintSize size, int copies = 1);
    void processPrintQueue();
    bool isPrinting() const;

    // Settings
    void setPrintSize(PrintSize size);
    PrintSize getPrintSize() const;
    void setDefaultCopies(int copies);
    int getDefaultCopies() const;

private:
    std::string selectedPrinter_;
    PrintSize defaultSize_;
    int defaultCopies_;
    
    std::queue<PrintJob> printQueue_;
    std::mutex queueMutex_;
    bool printing_;
    int nextJobId_;

    bool printInternal(const PrintJob& job);
    bool sendToPrinter(const std::string& imagePath, PrintSize size);
    std::pair<int, int> getSizeDimensions(PrintSize size);
};

} // namespace photobooth
