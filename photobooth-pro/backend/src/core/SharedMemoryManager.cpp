#include "core/SharedMemoryManager.h"
#include <chrono>

namespace photobooth {

SharedMemoryManager::SharedMemoryManager()
    : hMapFile_(NULL), pBuffer_(NULL), hMutex_(NULL), currentSequence_(0), bufferSize_(0) {}

SharedMemoryManager::~SharedMemoryManager() {
    cleanup();
}

bool SharedMemoryManager::initialize(const std::string& mapName, size_t bufferSize) {
    cleanup(); // Clear cũ nếu có

    mapName_ = mapName;
    bufferSize_ = bufferSize;

    // 1. Tạo File Mapping (Vùng nhớ ảo)
    // INVALID_HANDLE_VALUE nghĩa là map vùng nhớ RAM, không cần file vật lý trên ổ cứng
    hMapFile_ = CreateFileMappingA(
        INVALID_HANDLE_VALUE,    // Use paging file
        NULL,                    // Default security
        PAGE_READWRITE,          // Read/Write access
        0,                       // Max. object size (high-order DWORD)
        (DWORD)bufferSize,       // Max. object size (low-order DWORD)
        mapName.c_str()          // Global Name
    );

    if (hMapFile_ == NULL) {
        std::cerr << "[SharedMemory] Could not create file mapping object (" << GetLastError() << ")." << std::endl;
        return false;
    }

    // 2. Map file vào tiến trình hiện tại
    pBuffer_ = MapViewOfFile(
        hMapFile_,               // Handle to map object
        FILE_MAP_ALL_ACCESS,     // Read/Write permission
        0,
        0,
        bufferSize
    );

    if (pBuffer_ == NULL) {
        std::cerr << "[SharedMemory] Could not map view of file (" << GetLastError() << ")." << std::endl;
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
        return false;
    }

    // 3. Tạo Mutex để sync (tránh Frontend đọc khi Backend đang ghi dở)
    // Đặt tên Mutex dựa trên tên Map để đồng bộ
    std::string mutexName = mapName + "_Mutex";
    hMutex_ = CreateMutexA(NULL, FALSE, mutexName.c_str());

    if (hMutex_ == NULL) {
        std::cerr << "[SharedMemory] Could not create mutex (" << GetLastError() << ")." << std::endl;
        cleanup();
        return false;
    }

    // Init Header về 0
    memset(pBuffer_, 0, sizeof(SharedMemoryHeader));
    
    std::cout << "[SharedMemory] Initialized successfully: " << mapName << " (" << bufferSize / 1024 / 1024 << " MB)" << std::endl;
    return true;
}

void SharedMemoryManager::writeFrame(const std::vector<uint8_t>& jpegData, int width, int height) {
    if (!pBuffer_ || !hMutex_) return;

    if (jpegData.size() + sizeof(SharedMemoryHeader) > bufferSize_) {
        std::cerr << "[SharedMemory] Frame too large for buffer!" << std::endl;
        return;
    }

    // Wait for the mutex (Chờ tối đa 10ms, nếu Frontend giữ lâu quá thì bỏ qua frame này để tránh lag Backend)
    DWORD dwWaitResult = WaitForSingleObject(hMutex_, 10); 

    if (dwWaitResult == WAIT_OBJECT_0) {
        try {
            // Update Header
            SharedMemoryHeader* header = static_cast<SharedMemoryHeader*>(pBuffer_);
            
            // Tăng sequence để báo hiệu frame mới
            currentSequence_++; 
            header->sequence = currentSequence_;
            header->dataSize = (uint32_t)jpegData.size();
            header->width = width;
            header->height = height;
            header->timestamp = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::system_clock::now().time_since_epoch()).count();

            // Copy Data (Offset bằng kích thước Header)
            // Header: 128 bytes
            // Data: Bắt đầu từ byte 128
            uint8_t* dataPtr = static_cast<uint8_t*>(pBuffer_) + sizeof(SharedMemoryHeader);
            memcpy(dataPtr, jpegData.data(), jpegData.size());

        } catch (...) {
            std::cerr << "[SharedMemory] Error writing memory." << std::endl;
        }

        // Release mutex
        ReleaseMutex(hMutex_);
    } else {
        // std::cerr << "[SharedMemory] Drop frame: Mutex timeout." << std::endl;
    }
}

void SharedMemoryManager::cleanup() {
    if (pBuffer_) {
        UnmapViewOfFile(pBuffer_);
        pBuffer_ = NULL;
    }

    if (hMapFile_) {
        CloseHandle(hMapFile_);
        hMapFile_ = NULL;
    }

    if (hMutex_) {
        CloseHandle(hMutex_);
        hMutex_ = NULL;
    }
}

} // namespace photobooth
