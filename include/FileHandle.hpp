#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstring>
#include <dirent.h>
#include <cstddef>
#include <cstdint>
#include <cctype>
#include <climits>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <type_traits>

/**
 * @brief 基于UNIX文件操作的文件句柄封装
 * 支持流风格的读写操作
 */
class FileHandle {
public:
    // 打开模式
    enum class OpenMode : int {
        ReadOnly = O_RDONLY,
        WriteOnly = O_WRONLY,
        ReadWrite = O_RDWR,
        Create = O_CREAT,
        Truncate = O_TRUNC,
        Append = O_APPEND,
        Exclusive = O_EXCL
    };

    // 起始位置
    enum class SeekOrigin {
        Begin = SEEK_SET,
        Current = SEEK_CUR,
        End = SEEK_END
    };

public:
    /**
     * @brief 默认构造，空文件句柄
     */
    FileHandle() = default;

    /**
     * @brief 构造并打开文件
     * @param path 文件路径
     * @param mode 打开模式，可以按位或组合
     */
    explicit FileHandle(const std::string& path, int mode)
        : m_fd(-1)
    {
        open(path, mode);
    }

    /**
     * @brief 析构，自动关闭文件
     */
    ~FileHandle() {
        close();
    }

    // 不允许拷贝
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    // 允许移动
    FileHandle(FileHandle&& other) noexcept
        : m_fd(other.m_fd)
    {
        other.m_fd = -1;
    }

    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            close();
            m_fd = other.m_fd;
            other.m_fd = -1;
        }
        return *this;
    }

    /**
     * @brief 打开文件
     * @param path 文件路径
     * @param mode 打开模式，OpenMode按位或组合
     * @param perm 文件权限（创建时使用）
     * @return true 打开成功，false 打开失败
     */
    bool open(const std::string& path, int mode, mode_t perm = 0644) {
        close();
        int flags = 0;
        if (mode & static_cast<int>(OpenMode::ReadOnly)) flags |= O_RDONLY;
        if (mode & static_cast<int>(OpenMode::WriteOnly)) flags |= O_WRONLY;
        if (mode & static_cast<int>(OpenMode::ReadWrite)) flags |= O_RDWR;
        if (mode & static_cast<int>(OpenMode::Create)) flags |= O_CREAT;
        if (mode & static_cast<int>(OpenMode::Truncate)) flags |= O_TRUNC;
        if (mode & static_cast<int>(OpenMode::Append)) flags |= O_APPEND;
        if (mode & static_cast<int>(OpenMode::Exclusive)) flags |= O_EXCL;

        if (flags & O_CREAT) {
            m_fd = ::open(path.c_str(), flags, perm);
        } else {
            m_fd = ::open(path.c_str(), flags);
        }

        return is_open();
    }

    /**
     * @brief 关闭文件
     */
    void close() {
        if (is_open()) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    /**
     * @brief 检查文件是否打开
     * @return true 已打开，false 未打开
     */
    bool is_open() const {
        return m_fd >= 0;
    }

    /**
     * @brief 获取文件描述符
     * @return int 文件描述符
     */
    int fd() const {
        return m_fd;
    }

    /**
     * @brief 获取文件大小
     * @return off_t 文件大小，失败返回-1
     */
    off_t size() const {
        if (!is_open()) {
            return -1;
        }
        struct stat st;
        if (::fstat(m_fd, &st) != 0) {
            return -1;
        }
        return st.st_size;
    }

    /**
     * @brief 移动文件指针
     * @param offset 偏移量
     * @param origin 起始位置
     * @return off_t 新位置，失败返回-1
     */
    off_t seek(off_t offset, SeekOrigin origin) {
        if (!is_open()) {
            return -1;
        }
        return ::lseek(m_fd, offset, static_cast<int>(origin));
    }

    /**
     * @brief 获取当前文件指针位置
     * @return off_t 当前位置，失败返回-1
     */
    off_t tell() const {
        if (!is_open()) {
            return -1;
        }
        return ::lseek(m_fd, 0, SEEK_CUR);
    }

    /**
     * @brief 获取当前可读取的字节数（类似Arduino File.available()）
     * @return int 可读取字节数，0表示无数据可读，-1表示出错
     * @note 返回文件剩余字节数 = 文件大小 - 当前位置
     */
    int available() const {
        if (!is_open()) {
            return -1;
        }

        off_t file_size = size();
        off_t current_pos = tell();

        if (file_size < 0 || current_pos < 0) {
            return -1;
        }

        off_t remaining = file_size - current_pos;
        if (remaining <= 0) {
            return 0;
        }

        // 返回剩余字节数，超出int范围时截断
        return static_cast<int>(remaining > INT_MAX ? INT_MAX : remaining);
    }

    /**
     * @brief 检查是否还有数据可读
     * @return true 有数据可读，false 无数据可读
     */
    bool has_available() const {
        return available() > 0;
    }

    /**
     * @brief 同步文件数据到磁盘
     * @return true 成功，false 失败
     */
    bool sync() {
        if (!is_open()) {
            return false;
        }
        return ::fsync(m_fd) == 0;
    }

    /**
     * @brief 读取指定字节数
     * @param buffer 输出缓冲区
     * @param count 要读取的字节数
     * @return ssize_t 实际读取的字节数，失败返回-1
     */
    ssize_t read(void* buffer, size_t count) {
        if (!is_open()) {
            return -1;
        }
        return ::read(m_fd, buffer, count);
    }

    /**
     * @brief 写入指定字节数
     * @param buffer 输入数据
     * @param count 要写入的字节数
     * @return ssize_t 实际写入的字节数，失败返回-1
     */
    ssize_t write(const void* buffer, size_t count) {
        if (!is_open()) {
            return -1;
        }
        return ::write(m_fd, buffer, count);
    }

    /**
     * @brief 读取整行
     * @param line 输出字符串
     * @param delimiter 分隔符，默认为换行符
     * @return true 读取成功，false 读取失败或到达文件尾
     */
    bool getline(std::string& line, char delimiter = '\n') {
        if (!is_open() || size() <= 0) {
            return false;
        }

        line.clear();
        char c;
        ssize_t n;

        while ((n = read(&c, 1)) == 1) {
            if (c == delimiter) {
                break;
            }
            line.push_back(c);
        }

        return !line.empty() || n == 1;
    }

    /**
     * @brief 读取整个文件到字符串
     * @return std::string 文件内容，失败返回空串
     */
    std::string read_all() {
        if (!is_open()) {
            return {};
        }

        off_t file_size = size();
        if (file_size <= 0) {
            return {};
        }

        std::string content;
        content.resize(static_cast<size_t>(file_size));

        seek(0, SeekOrigin::Begin);
        ssize_t n = read(content.data(), static_cast<size_t>(file_size));

        if (n <= 0) {
            return {};
        }

        content.resize(static_cast<size_t>(n));
        return content;
    }

    /**
     * @brief 写入字符串
     * @param str 字符串
     * @return ssize_t 实际写入的字节数
     */
    ssize_t write_string(std::string_view str) {
        return write(str.data(), str.size());
    }

    /**
     * @brief 写入换行
     * @return ssize_t 实际写入的字节数
     */
    ssize_t newline() {
        const char nl = '\n';
        return write(&nl, 1);
    }

    /**
     * @brief 写入一行
     * @param str 字符串
     * @return ssize_t 实际写入的字节数（包括换行）
     */
    ssize_t writeline(std::string_view str) {
        ssize_t n = write_string(str);
        if (n < 0) {
            return n;
        }
        ssize_t nl = newline();
        return n + nl;
    }

    // === 流操作支持 ===

    /**
     * @brief 流读取操作符
     * @tparam T 支持的类型：整数类型、浮点类型、std::string
     * @param value 输出值
     * @return FileHandle& 引用自指
     */
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, FileHandle&>::type
    operator>>(T& value) {
        read(&value, sizeof(T));
        return *this;
    }

    /**
     * @brief 流写入操作符
     * @tparam T 支持的类型：整数类型、浮点类型
     * @param value 输入值
     * @return FileHandle& 引用自指
     */
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, FileHandle&>::type
    operator<<(T value) {
        write(&value, sizeof(T));
        return *this;
    }

    /**
     * @brief 流写入字符串
     * @param str 字符串
     * @return FileHandle& 引用自指
     */
    FileHandle& operator<<(std::string_view str) {
        write_string(str);
        return *this;
    }

    /**
     * @brief 流写入std::string
     * @param str 字符串
     * @return FileHandle& 引用自指
     */
    FileHandle& operator<<(const std::string& str) {
        write_string(str);
        return *this;
    }

    /**
     * @brief 流写入C字符串
     * @param str C字符串
     * @return FileHandle& 引用自指
     */
    FileHandle& operator<<(const char* str) {
        if (str) {
            write(str, std::char_traits<char>::length(str));
        }
        return *this;
    }

    /**
     * @brief 读取字符串
     * @param str 字符串
     * @return FileHandle& 引用自指
     */
    FileHandle& operator>>(std::string& str) {
        str.clear();
        char c;
        // 跳过空白符
        while (true) {
            ssize_t n = read(&c, 1);
            if (n != 1) {
                return *this;
            }
            if (!std::isspace(c)) {
                str.push_back(c);
                break;
            }
        }
        // 读取直到下一个空白符
        while (true) {
            ssize_t n = read(&c, 1);
            if (n != 1) {
                break;
            }
            if (std::isspace(c)) {
                break;
            }
            str.push_back(c);
        }
        return *this;
    }

    /**
     * @brief 布尔转换，检查文件是否有效打开
     * @return true 已打开，false 未打开
     */
    explicit operator bool() const {
        return is_open();
    }

    /**
     * @brief 获取最后一次错误
     * @return int errno
     */
    int error() const {
        return errno;
    }

    /**
     * @brief 获取错误信息字符串
     * @return const char* 错误信息
     */
    const char* strerror() const {
        return ::strerror(errno);
    }

    // === 静态工具函数 ===

    /**
     * @brief 目录条目信息
     */
    struct DirEntry {
        std::string name;       ///< 文件名
        bool is_directory;      ///< 是否是目录
        bool is_regular_file;   ///< 是否是普通文件
    };

    /**
     * @brief 列出目录中的所有文件条目
     * @param path 目录路径
     * @return std::vector<DirEntry> 目录条目列表，失败返回空
     */
    static std::vector<DirEntry> listdir(const std::string& path) {
        std::vector<DirEntry> entries;

        DIR* dir = ::opendir(path.c_str());
        if (!dir) {
            return entries;
        }

        struct dirent* entry;
        while ((entry = ::readdir(dir)) != nullptr) {
            // 跳过 . 和 ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            DirEntry ent;
            ent.name = entry->d_name;

            // 判断文件类型
            ent.is_directory = false;
            ent.is_regular_file = false;

        #if defined(DT_DIR)
            if (entry->d_type == DT_DIR) {
                ent.is_directory = true;
            } else if (entry->d_type == DT_REG) {
                ent.is_regular_file = true;
            }
        #else
            // 如果不支持 d_type，通过 stat 判断
            std::string full_path = path + "/" + entry->d_name;
            struct stat st;
            if (::stat(full_path.c_str(), &st) == 0) {
                ent.is_directory = S_ISDIR(st.st_mode);
                ent.is_regular_file = S_ISREG(st.st_mode);
            }
        #endif

            entries.push_back(std::move(ent));
        }

        ::closedir(dir);
        return entries;
    }

    /**
     * @brief 检查路径是否是目录
     * @param path 路径
     * @return true 是目录，false 不是目录或出错
     */
    static bool is_directory(const std::string& path) {
        struct stat st;
        if (::stat(path.c_str(), &st) != 0) {
            return false;
        }
        return S_ISDIR(st.st_mode);
    }

    /**
     * @brief 检查路径是否是普通文件
     * @param path 路径
     * @return true 是普通文件，false 不是或出错
     */
    static bool is_regular_file(const std::string& path) {
        struct stat st;
        if (::stat(path.c_str(), &st) != 0) {
            return false;
        }
        return S_ISREG(st.st_mode);
    }

    /**
     * @brief 检查文件/目录是否存在
     * @param path 路径
     * @return true 存在，false 不存在
     */
    static bool exists(const std::string& path) {
        return ::access(path.c_str(), F_OK) == 0;
    }

    /**
     * @brief 删除文件
     * @param path 文件路径
     * @return true 删除成功，false 删除失败
     */
    static bool remove(const std::string& path) {
        return ::unlink(path.c_str()) == 0;
    }

    /**
     * @brief 重命名文件
     * @param old_path 原路径
     * @param new_path 新路径
     * @return true 成功，false 失败
     */
    static bool rename(const std::string& old_path, const std::string& new_path) {
        return ::rename(old_path.c_str(), new_path.c_str()) == 0;
    }

private:
    int m_fd = -1;
};

// 方便组合打开模式
inline FileHandle::OpenMode operator|(FileHandle::OpenMode a, FileHandle::OpenMode b) {
    return static_cast<FileHandle::OpenMode>(static_cast<int>(a) | static_cast<int>(b));
}

inline FileHandle::OpenMode operator&(FileHandle::OpenMode a, FileHandle::OpenMode b) {
    return static_cast<FileHandle::OpenMode>(static_cast<int>(a) & static_cast<int>(b));
}
