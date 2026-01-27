#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <iostream>

namespace photobooth {

// Cấu trúc Header nằm ở đầu vùng nhớ chia sẻ
// Frontend sẽ đọc struct này trước để biết frame mới hay cũ, và kích thước bao nhiêu
struct SharedMemoryHeader {
    volatile int64_t sequence;      // Tăng dần mỗi khi có frame mới
    volatile uint32_t dataSize;     // Kích thước tệp JPEG (bytes)
    volatile uint32_t width;        // Chiều rộng ảnh
    volatile uint32_t height;       // Chiều cao ảnh
    volatile uint32_t timestamp;    // Thời gian capture
    uint8_t padding[104];           // Padding để Header tròn 128 bytes (Alignment)
};

class SharedMemoryManager {
public:
    SharedMemoryManager();
    ~SharedMemoryManager();

    // Khởi tạo vùng nhớ chia sẻ với kích thước buffSize (Mặc định 20MB cho an toàn)
    bool initialize(const std::string& mapName, size_t bufferSize = 20 * 1024 * 1024);
    
    // Ghi dữ liệu ảnh vào Shared Memory
    void writeFrame(const std::vector<uint8_t>& jpegData, int width = 0, int height = 0);
    
    void cleanup();

private:
    std::string mapName_;
    HANDLE hMapFile_;
    void* pBuffer_; // Con trỏ tới vùng nhớ đã map
    HANDLE hMutex_; // Mutex để đồng bộ Read/Write
    
    int64_t currentSequence_;
    size_t bufferSize_;
};

} // namespace photobooth
