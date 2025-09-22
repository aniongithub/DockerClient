#define RAPIDJSON_HAS_STDSTRING 1
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <vector>
#include <map>
#include <curl/curl.h>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#define JSON_DOCUMENT rapidjson::Document
#define JSON_VALUE rapidjson::Value

typedef enum{
    GET,
    POST,
    DELETE,
    PUT
} Method;

std::string param( const std::string& param_name, const std::string& param_value);
std::string param( const std::string& param_name, const char* param_value);
std::string param( const std::string& param_name, bool param_value);
std::string param( const std::string& param_name, int param_value);
std::string param( const std::string& param_name, JSON_DOCUMENT& param_value);

std::string jsonToString(JSON_VALUE & doc);

// Callback types for container execution
typedef std::function<void(const std::string& data)> OutputCallback;
typedef std::function<void(const std::string& data)> ErrorCallback;
typedef std::function<std::string()> InputCallback;

class Docker{
    public :
        Docker();
        explicit Docker(std::string host);
        ~Docker();

        /*
        * System
        */
        JSON_DOCUMENT system_info();
        JSON_DOCUMENT docker_version();

        /*
        * Images
        */
        JSON_DOCUMENT list_images();

        /*
        * Containers
        */
        JSON_DOCUMENT list_containers(bool all=false, int limit=-1, const std::string& since="", const std::string& before="", int size=-1, JSON_DOCUMENT& filters=emptyDoc);
        JSON_DOCUMENT inspect_container(const std::string& container_id);
        JSON_DOCUMENT top_container(const std::string& container_id);
        JSON_DOCUMENT logs_container(const std::string& container_id, bool follow=false, bool o_stdout=true, bool o_stderr=false, bool timestamps=false, const std::string& tail="all");
        JSON_DOCUMENT create_container(JSON_DOCUMENT& parameters, const std::string& name="");
        JSON_DOCUMENT start_container(const std::string& container_id);
        JSON_DOCUMENT get_container_changes(const std::string& container_id);
        JSON_DOCUMENT stop_container(const std::string& container_id, int delay=-1);
        JSON_DOCUMENT kill_container(const std::string& container_id, int signal=-1);
        JSON_DOCUMENT pause_container(const std::string& container_id);
        JSON_DOCUMENT wait_container(const std::string& container_id);
        JSON_DOCUMENT delete_container(const std::string& container_id, bool v=false, bool force=false);
        JSON_DOCUMENT unpause_container(const std::string& container_id);
        JSON_DOCUMENT restart_container(const std::string& container_id, int delay=-1);
        JSON_DOCUMENT attach_to_container(const std::string& container_id, bool logs=false, bool stream=false, bool o_stdin=false, bool o_stdout=false, bool o_stderr=false);
        //void copy_from_container(const std::string& container_id, const std::string& file_path, const std::string& dest_tar_file);

        /*
        * High-level container execution and log streaming
        */
        
        // Convenience method: create and start container in one call
        std::string run_container_async(
            const std::string& image,
            const std::vector<std::string>& command,
            const std::string& container_name = ""
        );
        
        // Attach live log streaming (non-blocking, runs callbacks in background)
        bool attach_log_stream(
            const std::string& container_id,
            OutputCallback on_stdout = nullptr,
            ErrorCallback on_stderr = nullptr
        );
        
        // Detach log streaming
        bool detach_log_stream(const std::string& container_id);

    private:
        std::string host_uri;
        bool is_remote;
        CURL *curl{};
        CURLcode res{};

        static JSON_DOCUMENT emptyDoc;
        
        // Log streaming state
        std::map<std::string, bool> active_log_streams;

        JSON_DOCUMENT requestAndParse(Method method, const std::string& path, unsigned success_code = 200, JSON_DOCUMENT& param=emptyDoc, bool isReturnJson=false);
        JSON_DOCUMENT requestAndParseJson(Method method, const std::string& path, unsigned success_code = 200, JSON_DOCUMENT& param=emptyDoc);

        // Helper method to parse Docker's multiplexed log stream
        void parse_docker_logs(const std::string& raw_logs, OutputCallback on_stdout, ErrorCallback on_stderr);

        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }
};

