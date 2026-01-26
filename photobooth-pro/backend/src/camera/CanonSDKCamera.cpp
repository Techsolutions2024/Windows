#include "camera/CanonSDKCamera.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <sstream>

#include "server/LiveViewServer.h"

namespace fs = std::filesystem;

namespace photobooth {

// Static instance for callbacks
CanonSDKCamera *CanonSDKCamera::currentInstance_ = nullptr;

CanonSDKCamera::CanonSDKCamera(EdsCameraRef camera, EdsUInt32 bodyID)
    : cameraRef_(camera) {
  // Create CameraModel from MultiCamCui with save to host
  cameraModel_ = std::make_unique<CameraModel>(camera, bodyID, kEdsSaveTo_Host);
  currentInstance_ = this;
}

CanonSDKCamera::~CanonSDKCamera() {
  disconnect();
  if (currentInstance_ == this) {
    currentInstance_ = nullptr;
  }
}

bool CanonSDKCamera::connect() {
  if (connected_)
    return true;

  std::lock_guard<std::mutex> lock(mutex_);

  // Use CameraModel's OpenSessionCommand
  if (cameraModel_->OpenSessionCommand()) {
    connected_ = true;

    // Get camera name
    EdsChar modelName[EDS_MAX_NAME] = {0};
    EdsGetPropertyData(cameraRef_, kEdsPropID_ProductName, 0, sizeof(modelName),
                       modelName);
    name_ = modelName;
    cameraModel_->setModelName(modelName);

    // Register event handlers
    EdsSetObjectEventHandler(cameraRef_, kEdsObjectEvent_All, handleObjectEvent,
                             this);
    EdsSetPropertyEventHandler(cameraRef_, kEdsPropertyEvent_All,
                               handlePropertyEvent, this);
    EdsSetCameraStateEventHandler(cameraRef_, kEdsStateEvent_All,
                                  handleStateEvent, this);

    // Read current settings from camera
    EdsUInt32 iso = 0, av = 0, tv = 0, wb = 0, iq = 0, ae = 0;
    EdsGetPropertyData(cameraRef_, kEdsPropID_ISOSpeed, 0, sizeof(iso), &iso);
    EdsGetPropertyData(cameraRef_, kEdsPropID_Av, 0, sizeof(av), &av);
    EdsGetPropertyData(cameraRef_, kEdsPropID_Tv, 0, sizeof(tv), &tv);
    EdsGetPropertyData(cameraRef_, kEdsPropID_WhiteBalance, 0, sizeof(wb), &wb);
    EdsGetPropertyData(cameraRef_, kEdsPropID_ImageQuality, 0, sizeof(iq), &iq);
    EdsGetPropertyData(cameraRef_, kEdsPropID_AEModeSelect, 0, sizeof(ae), &ae);

    extendedSettings_.isoCode = iso;
    extendedSettings_.apertureCode = av;
    extendedSettings_.shutterSpeedCode = tv;
    extendedSettings_.whiteBalanceCode = wb;
    extendedSettings_.imageQualityCode = iq;
    extendedSettings_.aeModeCode = ae;

    return true;
  }
  return false;
}

void CanonSDKCamera::disconnect() {
  if (!connected_)
    return;

  stopLiveView();

  std::lock_guard<std::mutex> lock(mutex_);

  if (cameraModel_) {
    cameraModel_->CloseSessionCommand();
  }
  connected_ = false;
}

bool CanonSDKCamera::isConnected() const { return connected_; }

std::string CanonSDKCamera::getName() const { return name_; }

bool CanonSDKCamera::startLiveView(LiveViewCallback callback) {
  if (!connected_ || liveViewActive_)
    return false;

  std::lock_guard<std::mutex> lock(mutex_);

  liveViewCallback_ = callback;

  // Enable EVF mode
  EdsError err = cameraModel_->StartEvfCommand();
  if (err != EDS_ERR_OK) {
    return false;
  }

  // Set output to PC
  EdsUInt32 device = kEdsEvfOutputDevice_PC;
  err = EdsSetPropertyData(cameraRef_, kEdsPropID_Evf_OutputDevice, 0,
                           sizeof(device), &device);
  if (err != EDS_ERR_OK) {
    return false;
  }

  liveViewActive_ = true;
  liveViewRunning_ = true;

  // Start live view thread
  liveViewThread_ = std::thread(&CanonSDKCamera::liveViewLoop, this);

  return true;
}

void CanonSDKCamera::stopLiveView() {
  if (!liveViewActive_)
    return;

  liveViewRunning_ = false;

  if (liveViewThread_.joinable()) {
    liveViewThread_.join();
  }

  std::lock_guard<std::mutex> lock(mutex_);

  // Disable output to PC
  EdsUInt32 device = 0;
  EdsGetPropertyData(cameraRef_, kEdsPropID_Evf_OutputDevice, 0, sizeof(device),
                     &device);
  device &= ~kEdsEvfOutputDevice_PC;
  EdsSetPropertyData(cameraRef_, kEdsPropID_Evf_OutputDevice, 0, sizeof(device),
                     &device);

  cameraModel_->EndEvfCommand();

  liveViewActive_ = false;
  liveViewCallback_ = nullptr;
}

bool CanonSDKCamera::isLiveViewActive() const { return liveViewActive_; }

void CanonSDKCamera::liveViewLoop() {
  EdsStreamRef stream = nullptr;
  EdsEvfImageRef evfImage = nullptr;

  // Wait for EVF to stabilize
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  while (liveViewRunning_ && connected_) {
    EdsError err = EDS_ERR_OK;

    // Create memory stream for image data
    err = EdsCreateMemoryStream(0, &stream);
    if (err != EDS_ERR_OK) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }

    // Create EVF image reference
    err = EdsCreateEvfImageRef(stream, &evfImage);
    if (err != EDS_ERR_OK) {
      EdsRelease(stream);
      stream = nullptr;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }

    // Download EVF image
    {
      std::lock_guard<std::mutex> lock(mutex_);
      err = EdsDownloadEvfImage(cameraRef_, evfImage);
    }

    if (err == EDS_ERR_OK) {
      // Get image data from stream
      EdsUInt64 length = 0;
      EdsGetLength(stream, &length);

      if (length > 0) {
        EdsVoid *imageData = nullptr;
        EdsGetPointer(stream, &imageData);

        if (imageData) {
          // 1. Copy raw data
          std::vector<uint8_t> rawData(static_cast<uint8_t *>(imageData),
                                       static_cast<uint8_t *>(imageData) +
                                           length);

          // 2. Release EDSDK resources immediately (critical for FPS)
          if (evfImage) {
            EdsRelease(evfImage);
            evfImage = nullptr;
          }
          if (stream) {
            EdsRelease(stream);
            stream = nullptr;
          }

          // 3. Process with OpenCV
          if (!rawData.empty()) {
            try {
              // Decode (EDSDK sends JPEG usually)
              cv::Mat frame = cv::imdecode(rawData, cv::IMREAD_COLOR);

              if (!frame.empty()) {
                cv::Mat resized;
                // Resize to 720p (1280x720)
                // Note: This might change aspect ratio if input isn't 16:9
                cv::resize(frame, resized, cv::Size(1280, 720));

                // Encode to JPEG with quality 70
                std::vector<uint8_t> encoded;
                std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 70};
                cv::imencode(".jpg", resized, encoded, params);

                // 4. Send to LiveViewServer
                LiveViewServer::getInstance().updateFrame(std::move(encoded));
              }
            } catch (const cv::Exception &e) {
              // Convert error to string safely or just ignore for live view
              // robustness
            }
          }
        }
      }
    } else if (err == EDS_ERR_OBJECT_NOTREADY) {
      // EVF data not ready, wait and retry
      std::this_thread::sleep_for(
          std::chrono::milliseconds(20)); // Reduced wait
    }

    // Ensure cleanup if not done above
    if (evfImage) {
      EdsRelease(evfImage);
      evfImage = nullptr;
    }
    if (stream) {
      EdsRelease(stream);
      stream = nullptr;
    }

    // Poll EDSDK events (critical for proper operation)
    EdsGetEvent();

    // Frame rate control (~30fps)
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
  }
}

void CanonSDKCamera::capture(CaptureMode mode, CaptureCallback callback) {
  if (!connected_) {
    CaptureResult result;
    result.success = false;
    result.errorMessage = "Camera not connected";
    if (callback)
      callback(result);
    return;
  }

  captureCallback_ = callback;

  std::lock_guard<std::mutex> lock(mutex_);

  // Take picture
  EdsError err = cameraModel_->TakePicture(
      kEdsCameraCommand_ShutterButton_Completely_NonAF);

  if (err != EDS_ERR_OK) {
    CaptureResult result;
    result.success = false;
    result.errorMessage = "Capture failed with error: " + std::to_string(err);
    if (callback)
      callback(result);
    captureCallback_ = nullptr;
  }
  // On success, the image will be delivered via handleObjectEvent callback
}

void CanonSDKCamera::captureWithCountdown(int seconds, CaptureMode mode,
                                          CaptureCallback callback) {
  // Wait for countdown then capture
  std::thread([this, seconds, mode, callback]() {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    capture(mode, callback);
  }).detach();
}

bool CanonSDKCamera::setSettings(const CameraSettings &settings) {
  if (!connected_)
    return false;

  syncFromBasicSettings(settings);
  return setExtendedSettings(extendedSettings_);
}

CameraSettings CanonSDKCamera::getSettings() const {
  return syncToBasicSettings();
}

void CanonSDKCamera::syncFromBasicSettings(const CameraSettings &settings) {
  // Find matching ISO code
  for (const auto &[code, label] : iso_table) {
    if (std::to_string(settings.iso) == label) {
      extendedSettings_.isoCode = code;
      break;
    }
  }

  // Find matching aperture code
  std::string apStr = settings.aperture;
  if (apStr.substr(0, 2) == "f/")
    apStr = apStr.substr(2);
  for (const auto &[code, label] : av_table) {
    if (apStr == label) {
      extendedSettings_.apertureCode = code;
      break;
    }
  }

  // Find matching shutter speed code
  for (const auto &[code, label] : tv_table) {
    if (settings.shutterSpeed == label ||
        ("1/" + settings.shutterSpeed) == std::string(label)) {
      extendedSettings_.shutterSpeedCode = code;
      break;
    }
  }

  // Find matching white balance code
  for (const auto &[code, label] : whitebalance_table) {
    if (settings.whiteBalance == label) {
      extendedSettings_.whiteBalanceCode = code;
      break;
    }
  }

  extendedSettings_.mirror = settings.mirror;
  extendedSettings_.rotation = settings.rotation;
}

CameraSettings CanonSDKCamera::syncToBasicSettings() const {
  CameraSettings settings;

  // Convert ISO code to value
  auto isoIt = iso_table.find(extendedSettings_.isoCode);
  if (isoIt != iso_table.end()) {
    try {
      settings.iso = std::stoi(isoIt->second);
    } catch (...) {
      settings.iso = 400;
    }
  }

  // Convert aperture code to string
  auto avIt = av_table.find(extendedSettings_.apertureCode);
  if (avIt != av_table.end()) {
    settings.aperture = "f/" + std::string(avIt->second);
  }

  // Convert shutter speed code to string
  auto tvIt = tv_table.find(extendedSettings_.shutterSpeedCode);
  if (tvIt != tv_table.end()) {
    settings.shutterSpeed = tvIt->second;
  }

  // Convert white balance code to string
  auto wbIt = whitebalance_table.find(extendedSettings_.whiteBalanceCode);
  if (wbIt != whitebalance_table.end()) {
    settings.whiteBalance = wbIt->second;
  }

  settings.mirror = extendedSettings_.mirror;
  settings.rotation = extendedSettings_.rotation;

  return settings;
}

std::vector<int> CanonSDKCamera::getSupportedISO() const {
  std::vector<int> result;
  EdsPropertyDesc desc = {0};

  if (EdsGetPropertyDesc(cameraRef_, kEdsPropID_ISOSpeed, &desc) ==
      EDS_ERR_OK) {
    for (int i = 0; i < desc.numElements; i++) {
      auto it = iso_table.find(desc.propDesc[i]);
      if (it != iso_table.end()) {
        try {
          int value = std::stoi(it->second);
          result.push_back(value);
        } catch (...) {
        }
      }
    }
  }
  return result;
}

std::vector<std::string> CanonSDKCamera::getSupportedApertures() const {
  std::vector<std::string> result;
  EdsPropertyDesc desc = {0};

  if (EdsGetPropertyDesc(cameraRef_, kEdsPropID_Av, &desc) == EDS_ERR_OK) {
    for (int i = 0; i < desc.numElements; i++) {
      auto it = av_table.find(desc.propDesc[i]);
      if (it != av_table.end() && strlen(it->second) > 0) {
        result.push_back("f/" + std::string(it->second));
      }
    }
  }
  return result;
}

std::vector<std::string> CanonSDKCamera::getSupportedShutterSpeeds() const {
  std::vector<std::string> result;
  EdsPropertyDesc desc = {0};

  if (EdsGetPropertyDesc(cameraRef_, kEdsPropID_Tv, &desc) == EDS_ERR_OK) {
    for (int i = 0; i < desc.numElements; i++) {
      auto it = tv_table.find(desc.propDesc[i]);
      if (it != tv_table.end()) {
        result.push_back(it->second);
      }
    }
  }
  return result;
}

std::vector<std::string> CanonSDKCamera::getSupportedWhiteBalances() const {
  std::vector<std::string> result;
  EdsPropertyDesc desc = {0};

  if (EdsGetPropertyDesc(cameraRef_, kEdsPropID_WhiteBalance, &desc) ==
      EDS_ERR_OK) {
    for (int i = 0; i < desc.numElements; i++) {
      auto it = whitebalance_table.find(desc.propDesc[i]);
      if (it != whitebalance_table.end()) {
        result.push_back(it->second);
      }
    }
  }
  return result;
}

bool CanonSDKCamera::setPropertyByCode(EdsPropertyID propertyID,
                                       EdsUInt32 code) {
  if (!connected_)
    return false;

  std::lock_guard<std::mutex> lock(mutex_);

  cameraModel_->UILock();
  EdsError err =
      EdsSetPropertyData(cameraRef_, propertyID, 0, sizeof(code), &code);
  cameraModel_->UIUnLock();

  if (err == EDS_ERR_OK) {
    // Update local settings cache
    switch (propertyID) {
    case kEdsPropID_ISOSpeed:
      extendedSettings_.isoCode = code;
      break;
    case kEdsPropID_Av:
      extendedSettings_.apertureCode = code;
      break;
    case kEdsPropID_Tv:
      extendedSettings_.shutterSpeedCode = code;
      break;
    case kEdsPropID_WhiteBalance:
      extendedSettings_.whiteBalanceCode = code;
      break;
    case kEdsPropID_ExposureCompensation:
      extendedSettings_.exposureCompCode = code;
      break;
    case kEdsPropID_ImageQuality:
      extendedSettings_.imageQualityCode = code;
      break;
    case kEdsPropID_AFMode:
      extendedSettings_.afModeCode = code;
      break;
    case kEdsPropID_DriveMode:
      extendedSettings_.driveModeCode = code;
      break;
    case kEdsPropID_PictureStyle:
      extendedSettings_.pictureStyleCode = code;
      break;
    case kEdsPropID_AEModeSelect:
      extendedSettings_.aeModeCode = code;
      break;
    }
    return true;
  }
  return false;
}

EdsUInt32 CanonSDKCamera::getPropertyCode(EdsPropertyID propertyID) const {
  EdsUInt32 value = 0;
  EdsGetPropertyData(cameraRef_, propertyID, 0, sizeof(value), &value);
  return value;
}

CanonSupportedValues CanonSDKCamera::getAllSupportedValues() {
  CanonSupportedValues values;

  values.iso = getPropertyDescWithLabels(kEdsPropID_ISOSpeed, iso_table);
  values.aperture = getPropertyDescWithLabels(kEdsPropID_Av, av_table);
  values.shutterSpeed = getPropertyDescWithLabels(kEdsPropID_Tv, tv_table);
  values.exposureComp = getPropertyDescWithLabels(
      kEdsPropID_ExposureCompensation, ExposureComp_table);
  values.whiteBalance =
      getPropertyDescWithLabels(kEdsPropID_WhiteBalance, whitebalance_table);
  values.pictureStyle =
      getPropertyDescWithLabels(kEdsPropID_PictureStyle, PictureStyle_table);
  values.afMode = getPropertyDescWithLabels(kEdsPropID_AFMode, AFmode_table);
  values.imageQuality =
      getPropertyDescWithLabels(kEdsPropID_ImageQuality, ImageQuality_table);
  values.driveMode =
      getPropertyDescWithLabels(kEdsPropID_DriveMode, drivemode_table);
  values.aeMode =
      getPropertyDescWithLabels(kEdsPropID_AEModeSelect, AEmode_table);

  return values;
}

std::vector<SDKOption> CanonSDKCamera::getPropertyDescWithLabels(
    EdsPropertyID propertyID, const std::map<EdsUInt32, const char *> &table) {

  std::vector<SDKOption> result;
  EdsPropertyDesc desc = {0};

  if (EdsGetPropertyDesc(cameraRef_, propertyID, &desc) == EDS_ERR_OK) {
    for (int i = 0; i < desc.numElements; i++) {
      EdsUInt32 code = desc.propDesc[i];
      auto it = table.find(code);
      if (it != table.end()) {
        SDKOption option;
        option.code = code;
        option.label = it->second;
        result.push_back(option);
      }
    }
  }
  return result;
}

CanonCameraSettings CanonSDKCamera::getExtendedSettings() const {
  return extendedSettings_;
}

bool CanonSDKCamera::setExtendedSettings(const CanonCameraSettings &settings) {
  if (!connected_)
    return false;

  bool success = true;

  // Set all properties
  success &= setPropertyByCode(kEdsPropID_ISOSpeed, settings.isoCode);
  success &= setPropertyByCode(kEdsPropID_Av, settings.apertureCode);
  success &= setPropertyByCode(kEdsPropID_Tv, settings.shutterSpeedCode);
  success &=
      setPropertyByCode(kEdsPropID_WhiteBalance, settings.whiteBalanceCode);
  success &= setPropertyByCode(kEdsPropID_ExposureCompensation,
                               settings.exposureCompCode);
  success &=
      setPropertyByCode(kEdsPropID_ImageQuality, settings.imageQualityCode);
  success &= setPropertyByCode(kEdsPropID_DriveMode, settings.driveModeCode);

  // Update app-level settings
  extendedSettings_.mirror = settings.mirror;
  extendedSettings_.rotation = settings.rotation;

  return success;
}

std::string CanonSDKCamera::generateFilename(const std::string &prefix) {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;

  std::ostringstream oss;
  oss << prefix << "_" << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S")
      << "_" << std::setfill('0') << std::setw(3) << ms.count() << ".jpg";
  return oss.str();
}

// Static event handlers
EdsError EDSCALLBACK CanonSDKCamera::handleObjectEvent(EdsObjectEvent event,
                                                       EdsBaseRef object,
                                                       EdsVoid *context) {

  CanonSDKCamera *camera = static_cast<CanonSDKCamera *>(context);
  if (!camera)
    return EDS_ERR_OK;

  if (event == kEdsObjectEvent_DirItemRequestTransfer) {
    // Image ready for download
    EdsDirectoryItemRef dirItem = static_cast<EdsDirectoryItemRef>(object);

    CaptureResult result;
    result.success = false;

    // Get directory item info
    EdsDirectoryItemInfo dirItemInfo;
    EdsError err = EdsGetDirectoryItemInfo(dirItem, &dirItemInfo);

    if (err == EDS_ERR_OK) {
      // Create save directory if needed
      fs::create_directories(camera->saveDirectory_);

      // Generate filename
      std::string filename = camera->generateFilename("capture");
      std::string fullPath = camera->saveDirectory_ + "/" + filename;

      // Create file stream
      EdsStreamRef stream = nullptr;
      err = EdsCreateFileStream(fullPath.c_str(),
                                kEdsFileCreateDisposition_CreateAlways,
                                kEdsAccess_ReadWrite, &stream);

      if (err == EDS_ERR_OK) {
        // Download image
        err = EdsDownload(dirItem, dirItemInfo.size, stream);

        if (err == EDS_ERR_OK) {
          err = EdsDownloadComplete(dirItem);

          result.success = true;
          result.filePath = fullPath;
          camera->lastCapturedPath_ = fullPath;

          // Read file to get image data
          std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
          if (file.is_open()) {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            result.imageData.resize(size);
            file.read(reinterpret_cast<char *>(result.imageData.data()), size);
          }
        }

        EdsRelease(stream);
      }
    }

    if (!result.success) {
      result.errorMessage = "Failed to download image: " + std::to_string(err);
    }

    // Call capture callback
    if (camera->captureCallback_) {
      camera->captureCallback_(result);
      camera->captureCallback_ = nullptr;
    }

    EdsRelease(dirItem);
  }

  return EDS_ERR_OK;
}

EdsError EDSCALLBACK CanonSDKCamera::handlePropertyEvent(EdsPropertyEvent event,
                                                         EdsPropertyID property,
                                                         EdsUInt32 param,
                                                         EdsVoid *context) {

  CanonSDKCamera *camera = static_cast<CanonSDKCamera *>(context);
  if (!camera)
    return EDS_ERR_OK;

  // Update local cache when camera property changes
  if (event == kEdsPropertyEvent_PropertyChanged) {
    EdsUInt32 value = 0;
    EdsGetPropertyData(camera->cameraRef_, property, 0, sizeof(value), &value);

    switch (property) {
    case kEdsPropID_ISOSpeed:
      camera->extendedSettings_.isoCode = value;
      break;
    case kEdsPropID_Av:
      camera->extendedSettings_.apertureCode = value;
      break;
    case kEdsPropID_Tv:
      camera->extendedSettings_.shutterSpeedCode = value;
      break;
    case kEdsPropID_WhiteBalance:
      camera->extendedSettings_.whiteBalanceCode = value;
      break;
    }
  }

  return EDS_ERR_OK;
}

EdsError EDSCALLBACK CanonSDKCamera::handleStateEvent(EdsStateEvent event,
                                                      EdsUInt32 param,
                                                      EdsVoid *context) {

  CanonSDKCamera *camera = static_cast<CanonSDKCamera *>(context);
  if (!camera)
    return EDS_ERR_OK;

  if (event == kEdsStateEvent_Shutdown) {
    // Camera disconnected
    camera->connected_ = false;
    camera->stopLiveView();
  }

  return EDS_ERR_OK;
}

} // namespace photobooth
