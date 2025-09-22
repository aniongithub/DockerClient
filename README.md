C++ Docker Client
===========================

A C++ client library for the Docker Engine API.

## Build Requirements
- CMake 3.14 or higher
- C++11 compatible compiler
- Git (for fetching dependencies)

## Dependencies
All dependencies are automatically downloaded and built using CMake's FetchContent:
- [libcurl](https://curl.haxx.se/libcurl/) - For HTTP communication with Docker API
- [RapidJSON](https://github.com/Tencent/rapidjson/) - For JSON parsing (header-only)

## Building

### Quick Start
```bash
git clone https://github.com/aniongithub/DockerClient.git
cd DockerClient
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### CMake Options
The build system will automatically:
- Download and compile libcurl with OpenSSL support
- Download RapidJSON headers
- Build the shared library (`liblibdocker-cpp.so`)
- Build the test executable

### Installation
```bash
# From the build directory
sudo make install
```

This installs:
- Library: `/usr/local/lib/liblibdocker-cpp.so`
- Headers: `/usr/local/include/docker.h`
- CMake config: `/usr/local/lib/cmake/libdocker-cpp/`

### Using in Your Project
After installation, you can use the library in your CMake project:

```cmake
find_package(libdocker-cpp REQUIRED)
target_link_libraries(your_target libdocker-cpp::libdocker-cpp)
```

## Example

See the [`example/`](example/) directory for a complete working example that demonstrates:
- Connecting to Docker daemon
- High-level container execution with `run_container_async()`
- Attaching and detaching log streams
- Real-time stdout/stderr callbacks
- Integration with low-level API methods
- Proper cleanup and error handling

To build and run the example:
```bash
# From the build directory
make
./example/docker-example
```

## API Usage

### Low-Level API (Docker Engine REST API)
The library provides direct access to Docker's REST API:

```cpp
#include "docker.h"
int main(){
    Docker client = Docker();

    // List containers
    JSON_DOCUMENT doc = client.list_containers(true); 
    std::cout << jsonToString(doc) << std::endl;

    // Create and start container manually
    JSON_DOCUMENT create_params(rapidjson::kObjectType);
    create_params.AddMember("Image", "ubuntu:latest", create_params.GetAllocator());
    
    JSON_DOCUMENT result = client.create_container(create_params, "my-container");
    std::string container_id = result["data"]["Id"].GetString();
    
    client.start_container(container_id);
    client.wait_container(container_id);
    client.delete_container(container_id);

    return 0;
}
```

### High-Level API (Container Execution & Log Streaming)
For common use cases, the library provides convenient high-level methods:

```cpp
#include "docker.h"
int main(){
    Docker client = Docker();

    // 1. Start container asynchronously
    std::string container_id = client.run_container_async(
        "ubuntu:latest",
        {"echo", "Hello World!"},
        "my-container"
    );

    if (!container_id.empty()) {
        // 2. Attach log stream with callbacks
        auto stdout_callback = [](const std::string& data) {
            std::cout << "Container output: " << data;
        };
        
        auto stderr_callback = [](const std::string& data) {
            std::cerr << "Container error: " << data;
        };

        client.attach_log_stream(container_id, stdout_callback, stderr_callback);

        // 3. Wait for completion using low-level API
        client.wait_container(container_id);

        // 4. Detach logs and cleanup
        client.detach_log_stream(container_id);
        client.delete_container(container_id, false, true);
    }

    return 0;
}
```

### Capturing Output to Strings
```cpp
std::string captured_stdout, captured_stderr;

auto capture_stdout = [&captured_stdout](const std::string& data) {
    captured_stdout += data;
};

auto capture_stderr = [&captured_stderr](const std::string& data) {
    captured_stderr += data;
};

std::string container_id = client.run_container_async("ubuntu", {"echo", "test"});
client.attach_log_stream(container_id, capture_stdout, capture_stderr);
client.wait_container(container_id);

std::cout << "Captured output: " << captured_stdout << std::endl;
```

## Return of Each Methods
Every return type is **JSON_OBJECT** (alias of **rapidjson::Document**)

Refer to [RapidJSON](https://github.com/Tencent/rapidjson/) for the detailed usage
```
Object o;
 - success        [bool]                  : if succeeded to request
 - data           [Object/Array/string]   : actual data by server
 - code(optional) [int]                   : http status code if 'success' is false
```
Data type of 'data' depends on API, but it would be Object if 'success' is false.
#### e.g.
```JSON
{
  "success": false,
  "code": 404,
  "data": {
      "message": "No such container: 5d271b3a52263330348b71948bd25cda455a49f7e7d69cfc73e6b2f3b5b41a4c" 
  }
} 
```
```JSON
{
  "success": true ,
  "data": {
      "Architecture": "x86_64",
      "SystemTime": "2018-05-23T19:26:54.357768797+09:00" 
 }
}
```

## Accessing Remote Docker Server
For remote access, you sould first bind Docker Server to a port.
You can bind by adding **-H tcp://0.0.0.0:\<port\>** in service daemon.
```bash
# cat /lib/systemd/system/docker.service
....
[Service]
Type=notify
ExecStart=/usr/bin/docker daemon -H fd:// -H tcp://0.0.0.0:4000
....
```

```C++
#include "docker.h"
int main(){
    Docker client = Docker("http://<ip>:<port>");

    JSON_DOCUMENT doc = client.list_containers(true); 
    std::cout << jsonToString(doc) << std::endl;

    return 0;
}
```

## API List

### High-Level Container Execution
- **run_container_async** - Create and start container, returns container ID
- **attach_log_stream** - Attach callbacks to container logs (stdout/stderr)  
- **detach_log_stream** - Detach log callbacks from container

### Low-Level Docker Engine API
#### System
- system_info
- docker_version
#### Image
- list_images
#### Containers
- list_containers
- inspect_container
- top_container
- logs_container
- create_container
- start_container
- get_container_changes
- stop_container
- kill_container
- pause_container
- wait_container
- delete_container
- unpause_container
- restart_container
- attach_to_container

## Design Philosophy

This library provides both **low-level** and **high-level** APIs:

- **Low-level API**: Direct mapping to Docker Engine REST API for full control
- **High-level API**: Convenient methods for common container execution patterns
- **Composable**: Mix and match both APIs as needed
- **Separated concerns**: Container lifecycle separate from log streaming
- **Docker-native**: Follows Docker's design patterns and workflows
