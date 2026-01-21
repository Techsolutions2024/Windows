#include "camera/CanonCamera.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace photobooth {

// =================================================================================================
// SDK CONSTANTS & MAPPINGS
// =================================================================================================

// Helper maps for converting between SDK values and User Friendly Strings

static const std::map<EdsUInt32, int> ISO_MAP = {
    {0x00, 0}, // Auto
    {0x28, 6},     {0x30, 12},    {0x38, 25},    {0x40, 50},    {0x48, 100},
    {0x4b, 125},   {0x4d, 160},   {0x50, 200},   {0x53, 250},   {0x55, 320},
    {0x58, 400},   {0x5b, 500},   {0x5d, 640},   {0x60, 800},   {0x63, 1000},
    {0x65, 1250},  {0x68, 1600},  {0x6b, 2000},  {0x6d, 2500},  {0x70, 3200},
    {0x73, 4000},  {0x75, 5000},  {0x78, 6400},  {0x7b, 8000},  {0x7d, 10000},
    {0x80, 12800}, {0x88, 25600}, {0x90, 51200}, {0x98, 102400}};

static const std::map<EdsUInt32, std::string> AV_MAP = {
    {0x08, "1.0"}, {0x0B, "1.1"}, {0x0D, "1.2"}, {0x10, "1.4"}, {0x13, "1.6"},
    {0x15, "1.8"}, {0x18, "2.0"}, {0x1B, "2.2"}, {0x1D, "2.5"}, {0x20, "2.8"},
    {0x23, "3.2"}, {0x25, "3.5"}, {0x28, "4.0"}, {0x2B, "4.5"}, {0x2D, "5.0"},
    {0x30, "5.6"}, {0x33, "6.3"}, {0x35, "7.1"}, {0x38, "8.0"}, {0x3B, "9.0"},
    {0x3D, "10"},  {0x40, "11"},  {0x43, "13"},  {0x45, "14"},  {0x48, "16"},
    {0x4B, "18"},  {0x4D, "20"},  {0x50, "22"},  {0x53, "25"},  {0x55, "29"},
    {0x58, "32"}};
// Note: Some newer mappings might differ slightly or depend on camera, but this
// is standard. Actually, in previous code f/2.8 was 0x48 !!! WAIT. Discrepancy
// detected. Standard EDSDK: 0x08 = f/1.0
// ... 0x10 = f/1.4
// ... 0x18 = f/2.0
// ... 0x20 = f/2.8
// ... 0x28 = f/4.0
// ... 0x30 = f/5.6
// ... 0x38 = f/8.0
// ... 0x40 = f/11
// ... 0x48 = f/16
// ... 0x50 = f/22
//
// The PREVIOUS CODE in CanonCamera.cpp had:
// if (av == "f/2.8") return 0x48;  <-- This contradicts standard EDSDK usually
// unless it's a specific older mapping or I am misremembering. Let's re-verify.
// 0x48 is usually f/16 or similar high value. However, I must trust the user's
// codebase if it was working? Or was it just a mockup? The previous code said:
// "Basic mapping, exhaustive table needed for real app". It is highly likely
// the previous code was GUESSING. 0x48 for f/2.8 is extremely high. 0x10
// is 1.4. 0x18 is 2.0. 0x20 is 2.8. I WILL USE THE STANDARD EDSDK MAPPING. The
// previous code was likely wrong or a placeholder.

static const std::map<EdsUInt32, std::string> AV_MAP_CORRECT = {
    {0x08, "f/1.0"}, {0x0B, "f/1.1"}, {0x0D, "f/1.2"}, {0x10, "f/1.4"},
    {0x13, "f/1.6"}, {0x15, "f/1.8"}, {0x18, "f/2.0"}, {0x1B, "f/2.2"},
    {0x1D, "f/2.5"}, {0x20, "f/2.8"}, {0x23, "f/3.2"}, {0x25, "f/3.5"},
    {0x28, "f/4.0"}, {0x2B, "f/4.5"}, {0x2D, "f/5.0"}, {0x30, "f/5.6"},
    {0x33, "f/6.3"}, {0x35, "f/7.1"}, {0x38, "f/8.0"}, {0x3B, "f/9.0"},
    {0x3D, "f/10"},  {0x40, "f/11"},  {0x43, "f/13"},  {0x45, "f/14"},
    {0x48, "f/16"},  {0x4B, "f/18"},  {0x4D, "f/20"},  {0x50, "f/22"},
    {0x53, "f/25"},  {0x55, "f/29"},  {0x58, "f/32"}};

static const std::map<EdsUInt32, std::string> TV_MAP = {
    {0x0C, "Bulk"},   {0x10, "30\""},   {0x13, "25\""},   {0x15, "20\""},
    {0x18, "15\""},   {0x1B, "13\""},   {0x1D, "10\""},   {0x20, "8\""},
    {0x23, "6\""},    {0x25, "5\""},    {0x28, "4\""},    {0x2B, "3.2\""},
    {0x2D, "2.5\""},  {0x30, "2\""},    {0x33, "1.6\""},  {0x35, "1.3\""},
    {0x38, "1\""},    {0x3B, "0.8\""},  {0x3D, "0.6\""},  {0x40, "0.5\""},
    {0x43, "0.4\""},  {0x45, "0.3\""},  {0x48, "1/4"},    {0x4B, "1/5"},
    {0x4D, "1/6"},    {0x50, "1/8"},    {0x53, "1/10"},   {0x55, "1/13"},
    {0x58, "1/15"},   {0x5B, "1/20"},   {0x5D, "1/25"},   {0x60, "1/30"},
    {0x63, "1/40"},   {0x65, "1/50"},   {0x68, "1/60"},   {0x6B, "1/80"},
    {0x6D, "1/100"},  {0x70, "1/125"},  {0x73, "1/160"},  {0x75, "1/200"},
    {0x78, "1/250"},  {0x7B, "1/320"},  {0x7D, "1/400"},  {0x80, "1/500"},
    {0x83, "1/640"},  {0x85, "1/800"},  {0x88, "1/1000"}, {0x8B, "1/1250"},
    {0x8D, "1/1600"}, {0x90, "1/2000"}, {0x93, "1/2500"}, {0x95, "1/3200"},
    {0x98, "1/4000"}, {0x9B, "1/5000"}, {0x9D, "1/6400"}, {0xA0, "1/8000"}};

static const std::map<EdsUInt32, std::string> WB_MAP = {
    {0, "Auto"},        // kEdsWhiteBalance_Auto
    {1, "Daylight"},    // kEdsWhiteBalance_Daylight
    {2, "Cloudy"},      // kEdsWhiteBalance_Cloudy
    {3, "Tungsten"},    // kEdsWhiteBalance_Tungsten
    {4, "Fluorescent"}, // kEdsWhiteBalance_Fluorescent
    {5, "Flash"},       // kEdsWhiteBalance_Flash
    {6, "Manual"},      // kEdsWhiteBalance_Manual
    {8, "Shade"},       // kEdsWhiteBalance_Shade
    {9, "ColorTemp"}    // kEdsWhiteBalance_ColorTemp
};

// Helpers to reverse lookup
EdsUInt32 getISOCode(int iso) {
  for (const auto &pair : ISO_MAP) {
    if (pair.second == iso)
      return pair.first;
  }
  return 0x58; // Default 400
}

EdsUInt32 getAvCode(const std::string &val) {
  for (const auto &pair : AV_MAP_CORRECT) {
    if (pair.second == val)
      return pair.first;
  }
  return 0x30; // Default f/5.6
}

EdsUInt32 getTvCode(const std::string &val) {
  for (const auto &pair : TV_MAP) {
    if (pair.second == val)
      return pair.first;
  }
  return 0x70; // Default 1/125
}

EdsUInt32 getWBCode(const std::string &val) {
  for (const auto &pair : WB_MAP) {
    if (pair.second == val)
      return pair.first;
  }
  return 0; // Auto
}

CanonCamera::CanonCamera(EdsCameraRef camera)
    : camera_(camera), connected_(false), liveViewActive_(false) {}

CanonCamera::~CanonCamera() {
  disconnect();
  if (camera_) {
    EdsRelease(camera_);
  }
}

bool CanonCamera::connect() {
  if (connected_)
    return true;

  EdsError err = EdsOpenSession(camera_);
  if (err == EDS_ERR_OK) {
    connected_ = true;

    // Get camera name
    EdsDataType dataType;
    EdsUInt32 dataSize;
    char name[256];
    err = EdsGetPropertySize(camera_, kEdsPropID_ProductName, 0, &dataType,
                             &dataSize);
    if (err == EDS_ERR_OK) {
      err = EdsGetPropertyData(camera_, kEdsPropID_ProductName, 0, dataSize,
                               name);
      if (err == EDS_ERR_OK) {
        name_ = name;
      }
    }

    // Register handlers
    EdsSetObjectEventHandler(camera_, kEdsObjectEvent_All, handleObjectEvent,
                             this);
    EdsSetPropertyEventHandler(camera_, kEdsPropertyEvent_All,
                               handlePropertyEvent, this);
    EdsSetCameraStateEventHandler(camera_, kEdsStateEvent_All, handleStateEvent,
                                  this);

    // Configure for save to host
    EdsUInt32 saveTo = kEdsSaveTo_Host;
    EdsSetPropertyData(camera_, kEdsPropID_SaveTo, 0, sizeof(saveTo), &saveTo);

    return true;
  }
  return false;
}

void CanonCamera::disconnect() {
  if (connected_) {
    stopLiveView();
    EdsCloseSession(camera_);
    connected_ = false;
  }
}

bool CanonCamera::isConnected() const { return connected_; }

std::string CanonCamera::getName() const { return name_; }

bool CanonCamera::startLiveView(LiveViewCallback callback) {
  if (!connected_)
    return false;
  if (liveViewActive_)
    return true;

  liveViewCallback_ = callback;
  liveViewActive_ = true;

  // Start Live View on Camera
  EdsUInt32 device = kEdsEvfOutputDevice_PC;
  EdsError err = EdsSetPropertyData(camera_, kEdsPropID_Evf_OutputDevice, 0,
                                    sizeof(device), &device);

  if (err != EDS_ERR_OK) {
    liveViewActive_ = false;
    return false;
  }

  liveViewThread_ = std::thread(&CanonCamera::liveViewLoop, this);
  return true;
}

void CanonCamera::stopLiveView() {
  if (!liveViewActive_)
    return;

  liveViewActive_ = false;
  if (liveViewThread_.joinable()) {
    liveViewThread_.join();
  }

  // Stop Live View on Camera
  if (connected_) {
    EdsUInt32 device = 0;
    EdsSetPropertyData(camera_, kEdsPropID_Evf_OutputDevice, 0, sizeof(device),
                       &device);
  }
}

bool CanonCamera::isLiveViewActive() const { return liveViewActive_; }

void CanonCamera::liveViewLoop() {
  while (liveViewActive_) {
    if (!connected_)
      break;

    EdsEvfImageRef evfImage = nullptr;
    EdsError err = EdsCreateEvfImageRef(camera_, &evfImage);

    if (err == EDS_ERR_OK) {
      err = EdsDownloadEvfImage(camera_, evfImage);
      if (err == EDS_ERR_OK) {
        // Get pointer to data
        unsigned char *data = nullptr;
        EdsUInt64 length = 0;
        EdsGetPointer(evfImage, (EdsVoid **)&data);
        EdsGetLength(evfImage, &length);

        if (data && length > 0 && liveViewCallback_) {
          std::vector<uint8_t> frame(data, data + length);
          liveViewCallback_(frame, 0, 0);
        }
      }
      if (evfImage)
        EdsRelease(evfImage);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
  }
}

void CanonCamera::capture(CaptureMode mode, CaptureCallback callback) {
  if (!connected_) {
    if (callback)
      callback({false, "", {}, 0, 0, "Camera not connected"});
    return;
  }

  captureCallback_ = callback;

  EdsError err = EdsSendCommand(camera_, kEdsCameraCommand_TakePicture, 0);
  if (err != EDS_ERR_OK) {
    if (callback)
      callback({false, "", {}, 0, 0, "Failed to send capture command"});
    captureCallback_ = nullptr;
  }
}

void CanonCamera::captureWithCountdown(int seconds, CaptureMode mode,
                                       CaptureCallback callback) {
  std::thread([this, seconds, mode, callback]() {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    this->capture(mode, callback);
  }).detach();
}

bool CanonCamera::setSettings(const CameraSettings &settings) {
  if (!connected_)
    return false;

  bool success = true;
  EdsError err;

  // ISO
  EdsUInt32 isoVal = getISOCode(settings.iso);
  err = EdsSetPropertyData(camera_, kEdsPropID_ISOSpeed, 0, sizeof(isoVal),
                           &isoVal);
  if (err != EDS_ERR_OK)
    success = false;

  // Aperture
  EdsUInt32 avVal = getAvCode(settings.aperture);
  err = EdsSetPropertyData(camera_, kEdsPropID_Av, 0, sizeof(avVal), &avVal);
  if (err != EDS_ERR_OK)
    success = false;

  // Shutter Speed
  EdsUInt32 tvVal = getTvCode(settings.shutterSpeed);
  err = EdsSetPropertyData(camera_, kEdsPropID_Tv, 0, sizeof(tvVal), &tvVal);
  if (err != EDS_ERR_OK)
    success = false;

  // White Balance
  EdsUInt32 wbVal = getWBCode(settings.whiteBalance);
  err = EdsSetPropertyData(camera_, kEdsPropID_WhiteBalance, 0, sizeof(wbVal),
                           &wbVal);
  if (err != EDS_ERR_OK)
    success = false;

  // Cache settings
  settings_ = settings;
  return success;
}

CameraSettings CanonCamera::getSettings() const {
  if (!connected_)
    return settings_;

  CameraSettings current;
  EdsError err;
  EdsUInt32 val;

  // ISO
  err = EdsGetPropertyData(camera_, kEdsPropID_ISOSpeed, 0, sizeof(val), &val);
  if (err == EDS_ERR_OK) {
    auto it = ISO_MAP.find(val);
    if (it != ISO_MAP.end())
      current.iso = it->second;
    else
      current.iso = 0;
  }

  // Aperture
  err = EdsGetPropertyData(camera_, kEdsPropID_Av, 0, sizeof(val), &val);
  if (err == EDS_ERR_OK) {
    auto it = AV_MAP_CORRECT.find(val);
    if (it != AV_MAP_CORRECT.end())
      current.aperture = it->second;
  }

  // Shutter
  err = EdsGetPropertyData(camera_, kEdsPropID_Tv, 0, sizeof(val), &val);
  if (err == EDS_ERR_OK) {
    auto it = TV_MAP.find(val);
    if (it != TV_MAP.end())
      current.shutterSpeed = it->second;
  }

  // WB
  err = EdsGetPropertyData(camera_, kEdsPropID_WhiteBalance, 0, sizeof(val),
                           &val);
  if (err == EDS_ERR_OK) {
    auto it = WB_MAP.find(val);
    if (it != WB_MAP.end())
      current.whiteBalance = it->second;
  }

  return current;
}

// Get Supported Values Helper
std::vector<EdsUInt32> getPropertyDesc(EdsCameraRef camera,
                                       EdsPropertyID propID) {
  EdsPropertyDesc desc = {0};
  EdsError err = EdsGetPropertyDesc(camera, propID, &desc);
  std::vector<EdsUInt32> values;
  if (err == EDS_ERR_OK) {
    for (EdsInt32 i = 0; i < desc.numElements; i++) {
      values.push_back(desc.propDesc[i]);
    }
  }
  return values;
}

std::vector<int> CanonCamera::getSupportedISO() const {
  if (!connected_)
    return {100, 200, 400, 800, 1600}; // Fallback

  std::vector<EdsUInt32> values = getPropertyDesc(camera_, kEdsPropID_ISOSpeed);
  std::vector<int> result;
  for (auto val : values) {
    auto it = ISO_MAP.find(val);
    if (it != ISO_MAP.end() && it->second != 0) {
      result.push_back(it->second);
    }
  }
  return result;
}

std::vector<std::string> CanonCamera::getSupportedApertures() const {
  if (!connected_)
    return {"f/2.8", "f/4.0", "f/5.6"};

  std::vector<EdsUInt32> values = getPropertyDesc(camera_, kEdsPropID_Av);
  std::vector<std::string> result;
  for (auto val : values) {
    auto it = AV_MAP_CORRECT.find(val);
    if (it != AV_MAP_CORRECT.end()) {
      result.push_back(it->second);
    }
  }
  return result;
}

std::vector<std::string> CanonCamera::getSupportedShutterSpeeds() const {
  if (!connected_)
    return {"1/125", "1/250"};

  std::vector<EdsUInt32> values = getPropertyDesc(camera_, kEdsPropID_Tv);
  std::vector<std::string> result;
  for (auto val : values) {
    auto it = TV_MAP.find(val);
    if (it != TV_MAP.end()) {
      result.push_back(it->second);
    }
  }
  return result;
}

std::vector<std::string> CanonCamera::getSupportedWhiteBalances() const {
  if (!connected_)
    return {"Auto", "Daylight", "Cloudy", "Tungsten", "Fluorescent"};

  std::vector<EdsUInt32> values =
      getPropertyDesc(camera_, kEdsPropID_WhiteBalance);
  std::vector<std::string> result;
  for (auto val : values) {
    auto it = WB_MAP.find(val);
    if (it != WB_MAP.end()) {
      result.push_back(it->second);
    }
  }
  return result;
}

// Helpers
bool CanonCamera::downloadImage(EdsDirectoryItemRef dirItem,
                                CaptureResult &result) {
  EdsError err = EDS_ERR_OK;
  EdsStreamRef stream = NULL;

  EdsDirectoryItemInfo dirItemInfo;
  err = EdsGetDirectoryItemInfo(dirItem, &dirItemInfo);
  if (err != EDS_ERR_OK)
    return false;

  std::string filename = "data/" + std::string(dirItemInfo.szFileName);
  result.filePath = filename;

  err = EdsCreateFileStream(filename.c_str(),
                            kEdsFileCreateDisposition_CreateAlways,
                            kEdsAccess_ReadWrite, &stream);
  if (err != EDS_ERR_OK)
    return false;

  err = EdsDownload(dirItem, dirItemInfo.size, stream);
  if (err == EDS_ERR_OK) {
    EdsDownloadComplete(dirItem);
    result.success = true;
  } else {
    result.success = false;
    result.errorMessage = "Download failed";
  }

  if (stream)
    EdsRelease(stream);
  return err == EDS_ERR_OK;
}

// Callbacks
EdsError EDSCALLBACK CanonCamera::handleObjectEvent(EdsObjectEvent event,
                                                    EdsBaseRef object,
                                                    EdsVoid *context) {
  CanonCamera *cam = static_cast<CanonCamera *>(context);
  if (event == kEdsObjectEvent_DirItemCreated) {
    if (cam->captureCallback_) {
      CaptureResult result;
      if (cam->downloadImage(object, result)) {
        cam->captureCallback_(result);
      } else {
        result.success = false;
        result.errorMessage = "Failed to download";
        cam->captureCallback_(result);
      }
      cam->captureCallback_ = nullptr;
    }
  }
  if (object)
    EdsRelease(object);
  return EDS_ERR_OK;
}

EdsError EDSCALLBACK CanonCamera::handlePropertyEvent(EdsPropertyEvent event,
                                                      EdsPropertyID property,
                                                      EdsUInt32 param,
                                                      EdsVoid *context) {
  return EDS_ERR_OK;
}

EdsError EDSCALLBACK CanonCamera::handleStateEvent(EdsStateEvent event,
                                                   EdsUInt32 param,
                                                   EdsVoid *context) {
  return EDS_ERR_OK;
}

} // namespace photobooth
