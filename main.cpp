#include <GL/glew.h> // Include before GLFW
#include <GLFW/glfw3.h>
#include <iostream>

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL Test Window", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "Error initializing GLEW\n";

    /* Print out the OpenGL version */
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}



//#include <iostream>
//#include <GLFW/glfw3.h>
//#include <d3d12.h>
//#include <dxgi1_4.h>
//
////#include <spdlog/spdlog.h>
////#include <spdlog/sinks/stdout_sinks.h>
//
//#pragma comment(lib, "d3d12.lib")
//#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "glfw3.lib")
//
//int main()
//{
//    // Initialize GLFW
//    if (!glfwInit())
//    {
//        std::cerr << "Failed to initialize GLFW" << std::endl;
//        return -1;
//    }
//
//    // Set GLFW window hints for DirectX 12
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//
//    // Create GLFW window
//    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW with DirectX 12", nullptr, nullptr);
//    if (!window) 
//    {
//        std::cerr << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    // Create DirectX 12 device
//    ID3D12Device* device = nullptr;
//    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
//    if (FAILED(hr))
//    {
//        std::cerr << "Failed to create DirectX 12 device" << std::endl;
//        glfwDestroyWindow(window);
//        glfwTerminate();
//        return -1;
//    }
//
//    //auto consoleLogger = spdlog::stdout_logger_mt("console");
//
//    //// Set the log level to display all messages
//    //consoleLogger->set_level(spdlog::level::trace);
//
//    //// Log some messages with different log levels
//    //consoleLogger->trace("This is a trace message");
//    //consoleLogger->debug("This is a debug message");
//    //consoleLogger->info("This is an info message");
//    //consoleLogger->warn("This is a warning message");
//    //consoleLogger->error("This is an error message");
//    //consoleLogger->critical("This is a critical message");
//
//    //// Flush the logger to ensure all messages are written
//    //spdlog::drop_all();
//
//    // Run the main loop
//    while (!glfwWindowShouldClose(window))
//    {
//        glfwPollEvents();
//    }
//
//    // Cleanup
//    device->Release();
//    glfwDestroyWindow(window);
//    glfwTerminate();
//
//    return 0;
//}
