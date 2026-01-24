{
    "targets": [
        {
            "target_name": "camera",
            "sources": [
                "backend/src/napi/camera_module.cpp",
                "backend/src/camera/WebcamCamera.cpp",
            ],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
                "backend/include",
                "C:/opencv/build/include",
            ],
            "libraries": ["C:/opencv/build/x64/vc16/lib/opencv_world4100.lib"],
            "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS", "USE_OPENCV"],
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "msvs_settings": {
                "VCCLCompilerTool": {
                    "ExceptionHandling": 1,
                    "AdditionalOptions": ["/std:c++17"],
                }
            },
            "conditions": [["OS=='win'", {"defines": ["WIN32"]}]],
        }
    ]
}
