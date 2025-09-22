#include "../docker.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Docker C++ Client Example - Hello World Container\n";
    std::cout << "=================================================\n\n";

    try {
        // Create Docker client (connects to local Docker daemon by default)
        Docker client;
        
        // 1. Check Docker connection
        std::cout << "1. Connecting to Docker daemon...\n";
        JSON_DOCUMENT system_info = client.system_info();
        if (system_info.HasMember("success") && system_info["success"].GetBool()) {
            std::cout << "   Connected successfully\n\n";
        } else {
            std::cerr << "   Failed to connect. Make sure Docker is running.\n";
            return 1;
        }

        // 2. Run container and capture logs with separated API
        std::cout << "2. Running hello world container...\n";
        
        std::string container_id = client.run_container_async(
            "ubuntu:latest",
            {"echo", "Hello from Docker C++ Client!"},
            "docker-cpp-example"
        );
        
        if (!container_id.empty()) {
            std::cout << "   Container started with ID: " << container_id.substr(0, 12) << "...\n";
            
            // Wait for container to complete
            JSON_DOCUMENT wait_result = client.wait_container(container_id);
            
            // Capture output to strings using log attachment
            std::string captured_output;
            auto capture_stdout = [&captured_output](const std::string& data) {
                captured_output += data;
            };
            
            // Attach log stream to capture output
            if (client.attach_log_stream(container_id, capture_stdout)) {
                std::cout << "   Container executed successfully\n";
                std::cout << "   Output: " << captured_output;
                client.detach_log_stream(container_id);
            }
            
            // Cleanup
            client.delete_container(container_id, false, true);
        } else {
            std::cout << "   Failed to start container\n";
            return 1;
        }

        // 3. Run container with real-time log streaming
        std::cout << "\n3. Running container with real-time log streaming...\n";
        
        container_id = client.run_container_async(
            "ubuntu:latest",
            {"sh", "-c", "echo 'This is stdout'; echo 'This is stderr' >&2"},
            "docker-cpp-streaming-example"
        );
        
        if (!container_id.empty()) {
            std::cout << "   Container started, attaching log stream...\n";
            
            auto stdout_callback = [](const std::string& data) {
                std::cout << "   [STDOUT] " << data;
                if (!data.empty() && data.back() != '\n') std::cout << "\n";
            };
            
            auto stderr_callback = [](const std::string& data) {
                std::cout << "   [STDERR] " << data;
                if (!data.empty() && data.back() != '\n') std::cout << "\n";
            };
            
            // Attach log stream
            if (client.attach_log_stream(container_id, stdout_callback, stderr_callback)) {
                // Wait for completion
                client.wait_container(container_id);
                std::cout << "   Container completed with log streaming\n";
                
                // Detach and cleanup
                client.detach_log_stream(container_id);
            }
            
            client.delete_container(container_id, false, true);
        } else {
            std::cout << "   Failed to start container\n";
        }

        std::cout << "\nDone.\n";

    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}