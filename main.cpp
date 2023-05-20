#include <iostream>
#include <GLFW/glfw3.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "glfw3.lib")

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW window hints for DirectX 12
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW with DirectX 12", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Create DirectX 12 device
    ID3D12Device* device = nullptr;
    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(hr))
    {
        std::cerr << "Failed to create DirectX 12 device" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    auto consoleLogger = spdlog::stdout_logger_mt("console");

    // Set the log level to display all messages
    consoleLogger->set_level(spdlog::level::trace);

    // Log some messages with different log levels
    consoleLogger->trace("This is a trace message");
    consoleLogger->debug("This is a debug message");
    consoleLogger->info("This is an info message");
    consoleLogger->warn("This is a warning message");
    consoleLogger->error("This is an error message");
    consoleLogger->critical("This is a critical message");

    // Flush the logger to ensure all messages are written
    spdlog::drop_all();

    // Run the main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    // Cleanup
    device->Release();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


//#include "game_engine/window/window.h"
//#include "game_engine/window/glfw_window.h"
//#include "game_engine/renderer/render_api.h"
//#include "game_engine/renderer/render_api_factory.h"
//#include "game_engine/utils/error_codes.h"
//
//#include <GLFW/glfw3.h>
//#include <iostream>
//
//#define USE_VULKAN
//
//#include <vulkan/vulkan.h>
//
//int main() 
//{
//    // Initialize GLFW
//    if (!glfwInit()) {
//        std::cerr << "Failed to initialize GLFW" << std::endl;
//        return -1;
//    }
//
//    // Set GLFW window hints
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//
//    // Create GLFW window
//    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW Window", nullptr, nullptr);
//    if (!window) {
//        std::cerr << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//#ifdef USE_VULKAN
//    // Initialize Vulkan
//    VkInstance instance;
//
//    VkApplicationInfo appInfo = {};
//    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//    appInfo.pApplicationName = "My Vulkan App";
//    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//    appInfo.pEngineName = "My Vulkan Engine";
//    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//    appInfo.apiVersion = VK_API_VERSION_1_0;
//
//    VkInstanceCreateInfo createInfo = {};
//    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//    createInfo.pApplicationInfo = &appInfo;
//
//    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
//
//    if (result != VK_SUCCESS) {
//        std::cerr << "Failed to create Vulkan instance" << std::endl;
//        glfwTerminate();
//        return -1;
//}
//
//
//    // Do Vulkan stuff...
//#else
//    // Initialize DirectX 12
//    ID3D12Device* device;
//    HRESULT result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
//    if (FAILED(result)) {
//        std::cerr << "Failed to create DirectX 12 device" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    // Do DirectX 12 stuff...
//#endif
//
//    // Main loop
//    while (!glfwWindowShouldClose(window)) {
//        glfwPollEvents();
//    }
//
//    // Clean up
//#ifdef USE_VULKAN
//    vkDestroyInstance(instance, nullptr);
//#else
//    device->Release();
//#endif
//    glfwDestroyWindow(window);
//    glfwTerminate();
//    return 0;
//    //game_engine::window::GLFWWindow window(800, 600, "Game Engine");
//
//    //game_engine::utils::ErrorCode errorCode = window.Initialize();
//    //if (errorCode != game_engine::utils::ErrorCode::Success) {
//    //    // Handle the error
//    //    return static_cast<int>(errorCode);
//    //}
//
//    //auto renderAPI = game_engine::renderer::CreateRenderAPI<game_engine::renderer::DX11RenderAPI>();
//    //renderAPI->Initialize();
//    //if (errorCode != game_engine::utils::ErrorCode::Success) {
//    //    // Handle the error
//    //    return static_cast<int>(errorCode);
//    //}
//    //while (!window.ShouldClose()) {
//    //    window.PollEvents();
//
//    //    renderAPI->DrawTriangle();
//    //    renderAPI->Present();
//    //}
//
//    ////window.Shutdown();
//
//    //return static_cast<int>(game_engine::utils::ErrorCode::Success);
//}