// #include "camera/CanonCamera.h"  // TODO: Add when EDSDK is configured
#include "camera/WebcamCamera.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <napi.h>

using namespace Napi;
using namespace photobooth;

// ============================================
// Global State
// ============================================

std::unique_ptr<ICamera> g_activeCamera;
std::vector<uint8_t> g_latestFrame;
std::mutex g_frameMutex;
bool g_liveViewActive = false;

// ============================================
// Helper Functions
// ============================================

/**
 * Get list of available cameras
 */
Napi::Value GetCameras(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Array cameras = Napi::Array::New(env);

  try {
    // List webcams
    auto webcams = WebcamCamera::listAvailableWebcams();

    for (size_t i = 0; i < webcams.size(); i++) {
      Napi::Object cam = Napi::Object::New(env);
      cam.Set("type", Napi::String::New(env, "webcam"));
      cam.Set("name", Napi::String::New(env, webcams[i].second));
      cam.Set("index", Napi::Number::New(env, webcams[i].first));
      cam.Set("connected", Napi::Boolean::New(env, false));
      cameras[i] = cam;
    }

    std::cout << "Found " << webcams.size() << " webcam(s)" << std::endl;

  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }

  return cameras;
}

/**
 * Select and connect to a camera
 */
Napi::Value SelectCamera(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Expected 2 arguments: type and index")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string type = info[0].As<Napi::String>().Utf8Value();
  int index = info[1].As<Napi::Number>().Int32Value();

  std::cout << "Selecting camera: " << type << " at index " << index
            << std::endl;

  try {
    // Stop live view if active
    if (g_liveViewActive && g_activeCamera) {
      g_activeCamera->stopLiveView();
      g_liveViewActive = false;
    }

    // Disconnect existing camera
    if (g_activeCamera) {
      g_activeCamera->disconnect();
      g_activeCamera.reset();
    }

    // Create new camera instance
    if (type == "webcam") {
      g_activeCamera = std::make_unique<WebcamCamera>(index);
    } else if (type == "canon") {
      // TODO: Implement Canon camera
      Napi::Error::New(env, "Canon camera not yet implemented")
          .ThrowAsJavaScriptException();
      return env.Null();
    } else {
      Napi::Error::New(env, "Unknown camera type: " + type)
          .ThrowAsJavaScriptException();
      return env.Null();
    }

    // Connect to camera
    bool connected = g_activeCamera->connect();

    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, connected));

    if (!connected) {
      result.Set("error",
                 Napi::String::New(env, "Failed to connect to camera"));
    }

    std::cout << "Camera " << (connected ? "connected" : "failed to connect")
              << std::endl;

    return result;

  } catch (const std::exception &e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, false));
    result.Set("error", Napi::String::New(env, e.what()));
    return result;
  }
}

/**
 * Start live view
 */
Napi::Value StartLiveView(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!g_activeCamera) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, false));
    result.Set("error", Napi::String::New(env, "No camera selected"));
    return result;
  }

  try {
    // Set up live view callback
    bool started = g_activeCamera->startLiveView(
        [](const std::vector<uint8_t> &frame, int width, int height) {
          // Store frame in global state
          std::lock_guard<std::mutex> lock(g_frameMutex);
          g_latestFrame = frame;
        });

    g_liveViewActive = started;

    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, started));

    if (!started) {
      result.Set("error", Napi::String::New(env, "Failed to start live view"));
    }

    std::cout << "Live view " << (started ? "started" : "failed to start")
              << std::endl;

    return result;

  } catch (const std::exception &e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, false));
    result.Set("error", Napi::String::New(env, e.what()));
    return result;
  }
}

/**
 * Get latest frame from live view
 * Returns a Buffer containing JPEG data
 */
Napi::Value GetFrame(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  std::lock_guard<std::mutex> lock(g_frameMutex);

  if (g_latestFrame.empty()) {
    return env.Null();
  }

  // Create a copy of the frame data
  // This is important because the Buffer will be sent to the renderer process
  Napi::Buffer<uint8_t> buffer = Napi::Buffer<uint8_t>::Copy(
      env, g_latestFrame.data(), g_latestFrame.size());

  return buffer;
}

/**
 * Stop live view
 */
Napi::Value StopLiveView(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (g_activeCamera && g_liveViewActive) {
    try {
      g_activeCamera->stopLiveView();
      g_liveViewActive = false;
      std::cout << "Live view stopped" << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "Error stopping live view: " << e.what() << std::endl;
    }
  }

  // Clear frame buffer
  {
    std::lock_guard<std::mutex> lock(g_frameMutex);
    g_latestFrame.clear();
  }

  return env.Null();
}

/**
 * Capture a photo
 */
Napi::Value Capture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!g_activeCamera) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, false));
    result.Set("error", Napi::String::New(env, "No camera selected"));
    return result;
  }

  try {
    // For now, return a simple success
    // TODO: Implement actual capture logic with callback
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, true));
    result.Set("path", Napi::String::New(env, "data/captures/photo.jpg"));

    std::cout << "Photo captured" << std::endl;

    return result;

  } catch (const std::exception &e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", Napi::Boolean::New(env, false));
    result.Set("error", Napi::String::New(env, e.what()));
    return result;
  }
}

// ============================================
// Module Initialization
// ============================================

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  // Export functions
  exports.Set("getCameras", Napi::Function::New(env, GetCameras));
  exports.Set("selectCamera", Napi::Function::New(env, SelectCamera));
  exports.Set("startLiveView", Napi::Function::New(env, StartLiveView));
  exports.Set("getFrame", Napi::Function::New(env, GetFrame));
  exports.Set("stopLiveView", Napi::Function::New(env, StopLiveView));
  exports.Set("capture", Napi::Function::New(env, Capture));

  std::cout << "✅ Camera N-API module initialized" << std::endl;

  return exports;
}

NODE_API_MODULE(camera, Init)
