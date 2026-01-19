#include "core/Application.h"
#include <iostream>
#include <csignal>

using namespace photobooth;

Application* g_app = nullptr;

void signalHandler(int signal) {
    std::cout << "\nShutting down gracefully..." << std::endl;
    if (g_app) {
        g_app->shutdown();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    std::cout << "==================================" << std::endl;
    std::cout << "  Photobooth Pro Server v1.0.0   " << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;

    // Setup signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Create application
    Application app;
    g_app = &app;

    // Initialize
    std::cout << "Initializing application..." << std::endl;
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return 1;
    }

    std::cout << "Application initialized successfully!" << std::endl;
    std::cout << "HTTP Server: http://localhost:8080" << std::endl;
    std::cout << "WebSocket Server: ws://localhost:8081" << std::endl;
    std::cout << std::endl;
    std::cout << "Press Ctrl+C to stop the server..." << std::endl;
    std::cout << std::endl;

    // Run application
    app.run();

    // Cleanup
    app.shutdown();
    std::cout << "Server stopped." << std::endl;

    return 0;
}
