#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <system_error>
#include <functional>
#include <cctype>
#include <map>
#include <codecvt>
#include <locale>
#include <cstring>

// 平台相关头文件和定义
#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <io.h>
    #pragma comment(lib, "ws2_32.lib")
    #define SOCKET_HANDLE SOCKET
    #define CLOSE_SOCKET closesocket
    #define INVALID_SOCKET_VALUE (INVALID_SOCKET)
    #define SOCKET_ERROR_VALUE (SOCKET_ERROR)
    #define GET_SOCKET_ERRNO WSAGetLastError()
    using ssize_t = long long;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <cstring>
    #include <dirent.h>
    #include <sys/stat.h>
    #define SOCKET_HANDLE int
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET_VALUE (-1)
    #define SOCKET_ERROR_VALUE (-1)
    #define GET_SOCKET_ERRNO errno
#endif

// 全局配置变量
int PORT = 8080;
std::string ROOT_DIR = ".";  // 默认网站根目录
const int BUFFER_SIZE = 4096;
const int THREAD_POOL_SIZE = 4;

// 初始化网络库（仅Windows需要）
void init_networking() {
#if defined(_WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
}

// 清理网络库（仅Windows需要）
void cleanup_networking() {
#if defined(_WIN32)
    WSACleanup();
#endif
}

// 线程池类
class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers) {
            if (worker.joinable()) worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// 安全的gmtime实现（解决C4996警告）
std::tm safe_gmtime(const time_t* time) {
#if defined(_WIN32)
    std::tm tm_result;
    gmtime_s(&tm_result, time);
    return tm_result;
#else
    std::tm tm_result;
    gmtime_r(time, &tm_result);
    return tm_result;
#endif
}

// 获取当前时间的GMT字符串
std::string get_gmt_time() {
    std::time_t now = std::time(nullptr);
    std::tm gmt_tm = safe_gmtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &gmt_tm);
    return std::string(buffer);
}

// URL解码函数 - 支持UTF-8
std::string url_decode(const std::string& str) {
    std::string bytes;
    bytes.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 < str.size()) {
                int value = 0;
                std::istringstream is(str.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    bytes += static_cast<char>(value);
                    i += 2;
                } else {
                    bytes += str[i];
                }
            } else {
                bytes += str[i];
            }
        } else if (str[i] == '+') {
            bytes += ' ';
        } else {
            bytes += str[i];
        }
    }
    // bytes已经是UTF-8编码，直接返回
    return bytes;
}

// 生成RFC 5987兼容的文件名
std::string generate_content_disposition(const std::string& filename) {
    // 尝试UTF-8编码
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        std::wstring wide = conv.from_bytes(filename);
        
        std::ostringstream oss;
        oss << "attachment;";
        oss << " filename=\"" << filename << "\";";  // 简单ASCII名称
        oss << " filename*=UTF-8''";  // RFC 5987扩展
        
        // URL编码UTF-8文件名
        for (char c : filename) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                oss << c;
            } else {
                oss << '%' << std::uppercase << std::setw(2) << std::setfill('0')
                    << std::hex << (static_cast<int>(c) & 0xff);
            }
        }
        
        return oss.str();
    } catch (...) {
        // 如果UTF-8转换失败，回退到简单ASCII
        return "attachment; filename=\"" + filename + "\"";
    }
}

// 发送HTTP响应
void send_response(SOCKET_HANDLE client_socket, const std::string& status, 
                  const std::string& content_type, const std::string& content,
                  const std::string& headers = "") {
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "Date: " << get_gmt_time() << "\r\n";
    if (!headers.empty()) {
        response << headers;
    }
    response << "\r\n";
    response << content;

    std::string response_str = response.str();
    send(client_socket, response_str.c_str(), static_cast<int>(response_str.size()), 0);
}

// 发送文件内容（支持大文件）
void send_file(SOCKET_HANDLE client_socket, const std::string& file_path, const std::string& content_type, 
              bool download = false) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        send_response(client_socket, "404 Not Found", "text/plain", "File Not Found");
        return;
    }

    // 获取文件大小
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // 构建响应头
    std::ostringstream header;
    header << "HTTP/1.1 200 OK\r\n";
    header << "Content-Type: " << content_type << "\r\n";
    header << "Content-Length: " << size << "\r\n";
    header << "Connection: close\r\n";
    header << "Date: " << get_gmt_time() << "\r\n";
    
    // 如果是下载，添加Content-Disposition
    if (download) {
        // 跨平台文件名提取
        std::string filename;
        size_t pos = file_path.find_last_of("/\\");
        if (pos != std::string::npos) {
            filename = file_path.substr(pos + 1);
        } else {
            filename = file_path;
        }
        
        // 添加编码后的Content-Disposition
        header << "Content-Disposition: " << generate_content_disposition(filename) << "\r\n";
    }
    
    header << "\r\n";
    std::string header_str = header.str();
    send(client_socket, header_str.c_str(), static_cast<int>(header_str.size()), 0);

    // 分块发送文件内容
    char buffer[BUFFER_SIZE];
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        ssize_t bytes_read = file.gcount();
        if (bytes_read > 0) {
            send(client_socket, buffer, static_cast<int>(bytes_read), 0);
        }
    }
}

// MIME类型映射
std::string get_content_type(const std::string& extension) {
    static const std::map<std::string, std::string> mime_types = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".txt", "text/plain"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".ico", "image/x-icon"}
    };
    
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), 
        [](unsigned char c){ return std::tolower(c); });
    
    auto it = mime_types.find(ext);
    if (it != mime_types.end()) {
        return it->second;
    }
    
    return "application/octet-stream";
}

// 检查是否为目录
bool is_directory(const std::string& path) {
#if defined(_WIN32)
    DWORD attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat info;
    if (stat(path.c_str(), &info)) return false;
    return S_ISDIR(info.st_mode);
#endif
}

// 检查文件是否存在
bool file_exists(const std::string& path) {
#if defined(_WIN32)
    DWORD attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES);
#else
    struct stat info;
    return stat(path.c_str(), &info) == 0;
#endif
}

#if defined(_WIN32)
// UTF-8转本地多字节
std::string utf8_to_local(const std::string& utf8_str) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
    if (wlen == 0) return "";
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wstr[0], wlen);
    int mblen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (mblen == 0) return "";
    std::string local_str(mblen, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &local_str[0], mblen, nullptr, nullptr);
    if (!local_str.empty() && local_str.back() == '\0') local_str.pop_back();
    return local_str;
}
#endif

// 处理HTTP请求
void handle_request(SOCKET_HANDLE client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        CLOSE_SOCKET(client_socket);
        return;
    }
    
    buffer[bytes_read] = '\0';
    std::string request(buffer);
    
    // 检查是否为GET请求
    if (request.substr(0, 3) != "GET") {
        send_response(client_socket, "405 Method Not Allowed", "text/plain", "Method Not Allowed");
        CLOSE_SOCKET(client_socket);
        return;
    }
    
    // 提取请求路径
    size_t start = request.find(' ') + 1;
    size_t end = request.find(' ', start);
    std::string path = request.substr(start, end - start);
    
    // URL解码
    path = url_decode(path);
    
    // 检查路径遍历攻击
    if (path.find("..") != std::string::npos || 
        path.find("//") != std::string::npos ||
        path.find("\\") != std::string::npos) {
        send_response(client_socket, "403 Forbidden", "text/plain", "Forbidden");
        CLOSE_SOCKET(client_socket);
        return;
    }
    
    // 处理下载请求
    if (path.find("/download/") == 0) {
        std::string file_path = ROOT_DIR + path.substr(9);
        #if defined(_WIN32)
        file_path = utf8_to_local(file_path);
        #endif
        send_file(client_socket, file_path, "application/octet-stream", true);
        CLOSE_SOCKET(client_socket);
        return;
    }
    
    // 默认文件为index.html
    if (path == "/" || path.empty()) path = "/index.html";
    
    // 构造文件路径
    std::string file_path = ROOT_DIR + path;
    #if defined(_WIN32)
    file_path = utf8_to_local(file_path);
    #endif
    
    // 检查是否为目录
    if (is_directory(file_path)) {
        // 确保路径以斜杠结尾
        if (path.back() != '/') {
            std::string redirect = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + path + "/\r\n\r\n";
            send(client_socket, redirect.c_str(), static_cast<int>(redirect.size()), 0);
            CLOSE_SOCKET(client_socket);
            return;
        }
        
        // 生成目录列表
        std::ostringstream dir_list;
        dir_list << "<html><head><title>Directory Listing</title>"
                 << "<style>"
                 << "body { font-family: Arial, sans-serif; margin: 20px; }"
                 << "h1 { color: #333; }"
                 << "ul { list-style-type: none; padding: 0; }"
                 << "li { margin: 5px 0; }"
                 << "a { text-decoration: none; color: #0066cc; }"
                 << "a:hover { text-decoration: underline; }"
                 << "</style></head>"
                 << "<body><h1>Directory Listing: " << path << "</h1><ul>";
        
#if defined(_WIN32)
        // Windows目录遍历
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA((file_path + "\\*").c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::string filename = findData.cFileName;
                if (filename == "." || filename == "..") continue;
                
                std::string item_path = path + (path.back() == '/' ? "" : "/") + filename;
                std::string full_path = file_path + "\\" + filename;
                
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    dir_list << "<li><a href=\"" << item_path << "/\">" << filename << "/</a></li>";
                } else {
                    dir_list << "<li><a href=\"" << item_path << "\">" << filename << "</a> "
                             << "(<a href=\"/download" << item_path << "\">Download</a>)</li>";
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
#else
        // POSIX目录遍历
        DIR* dir = opendir(file_path.c_str());
        if (dir) {
            struct dirent* ent;
            while ((ent = readdir(dir)) != nullptr) {
                std::string filename = ent->d_name;
                if (filename == "." || filename == "..") continue;
                
                std::string item_path = path + (path.back() == '/' ? "" : "/") + filename;
                std::string full_path = file_path + "/" + filename;
                
                struct stat st;
                if (stat(full_path.c_str(), &st)) continue;
                
                if (S_ISDIR(st.st_mode)) {
                    dir_list << "<li><a href=\"" << item_path << "/\">" << filename << "/</a></li>";
                } else {
                    dir_list << "<li><a href=\"" << item_path << "\">" << filename << "</a> "
                             << "(<a href=\"/download" << item_path << "\">Download</a>)</li>";
                }
            }
            closedir(dir);
        }
#endif
        
        dir_list << "</ul></body></html>";
        send_response(client_socket, "200 OK", "text/html", dir_list.str());
        CLOSE_SOCKET(client_socket);
        return;
    }
    
    // 检查文件是否存在
    if (!file_exists(file_path)) {
        send_response(client_socket, "404 Not Found", "text/plain", "File Not Found");
        CLOSE_SOCKET(client_socket);
        return;
    }
    
    // 获取文件扩展名
    std::string ext;
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        ext = file_path.substr(dot_pos);
    }
    
    // 获取Content-Type
    std::string content_type = get_content_type(ext);
    
    // 发送文件
    send_file(client_socket, file_path, content_type);
    CLOSE_SOCKET(client_socket);
}

// 显示帮助信息
void print_help() {
    std::cout << "Usage: LAN_HTTP [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -p <port>      Specify server port (default: 8080)\n";
    std::cout << "  -www <dir>     Specify web root directory (default: .)\n";
    std::cout << "  -h, --help     Show this help message\n";
}

// 解析命令行参数
void parse_arguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-p" && i + 1 < argc) {
            try {
                PORT = std::stoi(argv[i + 1]);
                i++; // 跳过下一个参数
            } catch (...) {
                std::cerr << "Invalid port number: " << argv[i + 1] << std::endl;
                exit(1);
            }
        } 
        else if (arg == "-www" && i + 1 < argc) {
            ROOT_DIR = argv[i + 1];
            
            // 规范化路径：确保不以斜杠结尾
            if (!ROOT_DIR.empty()) {
                char last_char = ROOT_DIR.back();
                if (last_char == '/' || last_char == '\\') {
                    ROOT_DIR.pop_back();
                }
            }
            i++; // 跳过下一个参数
        }
        else if (arg == "-h" || arg == "--help") {
            print_help();
            exit(0);
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            print_help();
            exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        // 解析命令行参数
        parse_arguments(argc, argv);
        
        init_networking();
        
        // 创建线程池
        ThreadPool pool(THREAD_POOL_SIZE);
        
        // 创建服务器socket
        SOCKET_HANDLE server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == INVALID_SOCKET_VALUE) {
            std::cerr << "Failed to create socket. Error: " << GET_SOCKET_ERRNO << "\n";
            cleanup_networking();
            return 1;
        }
        
        // 设置socket选项
        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, 
                  reinterpret_cast<const char*>(&opt), sizeof(opt));
        
        // 绑定地址和端口
        sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(PORT);
        
        if (bind(server_socket, reinterpret_cast<sockaddr*>(&server_address), 
                sizeof(server_address)) == SOCKET_ERROR_VALUE) {
            std::cerr << "Bind failed. Error: " << GET_SOCKET_ERRNO << "\n";
            CLOSE_SOCKET(server_socket);
            cleanup_networking();
            return 1;
        }
        
        // 开始监听
        if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR_VALUE) {
            std::cerr << "Listen failed. Error: " << GET_SOCKET_ERRNO << "\n";
            CLOSE_SOCKET(server_socket);
            cleanup_networking();
            return 1;
        }
        
        std::cout << "Server running on port " << PORT << "\n";
        std::cout << "Web root directory: " << ROOT_DIR << "\n";
        std::cout << "Thread pool size: " << THREAD_POOL_SIZE << "\n";
        std::cout << "Press Ctrl+C to stop the server\n";
        
        while (true) {
            // 接受客户端连接
            sockaddr_in client_address{};
            socklen_t client_addr_len = sizeof(client_address);
            SOCKET_HANDLE client_socket = accept(server_socket, 
                reinterpret_cast<sockaddr*>(&client_address), &client_addr_len);
            
            if (client_socket == INVALID_SOCKET_VALUE) {
                std::cerr << "Accept failed. Error: " << GET_SOCKET_ERRNO << "\n";
                continue;
            }
            
            // 获取客户端IP
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);

            // 读取请求的第一行，提取URL
            char req_buf[BUFFER_SIZE] = {0};
            ssize_t req_len = recv(client_socket, req_buf, sizeof(req_buf) - 1, MSG_PEEK);
            std::string url_path;
            if (req_len > 0) {
                std::istringstream iss(req_buf);
                std::string method, path;
                iss >> method >> path;
                url_path = path;
            }

            std::cout << "New connection from: " << client_ip;
            if (!url_path.empty()) {
                std::cout << " To: " << url_path;
            }
            std::cout << std::endl;
            
            // 将任务加入线程池
            pool.enqueue([client_socket] {
                handle_request(client_socket);
            });
        }
        
        CLOSE_SOCKET(server_socket);
        cleanup_networking();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        cleanup_networking();
        return 1;
    }
    
    return 0;
}