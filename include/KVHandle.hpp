#pragma once

#include "../deps/MyTinySTL/string"
#include <string_view>
#include "../deps/MyTinySTL/vector"
#include <cstdint>
#include <cstddef>
#include "../deps/MyTinySTL/type_traits"
#include <flashdb.h>

/**
 * @brief Flashdb 键值数据库封装
 * 支持简单的键值对存取，类似字典操作 []
 *
 * 使用示例:
 * @code
 *   KVHandle kv;
 *   if (kv.init(&your_flash_partition)) {
 *       kv["counter"] = 42;          // 写入整数
 *       int value = kv["counter"];   // 读取整数
 *       kv["name"] = "hello";        // 写入字符串
 *       std::string name = kv["name"]; // 读取字符串
 *   }
 * @endcode
 *
 * Flashdb 需要先定义一个 fdb_kvdb 分区，参考:
 * https://github.com/armink/Flashdb
 */
class KVHandle {
public:
    /**
     * @brief 默认构造
     */
    KVHandle() = default;

    /**
     * @brief 析构，会自动反初始化
     */
    ~KVHandle() {
        deinit();
    }

    // 不允许拷贝
    KVHandle(const KVHandle&) = delete;
    KVHandle& operator=(const KVHandle&) = delete;

    // 允许移动
    KVHandle(KVHandle&& other) noexcept
        : m_initialized(other.m_initialized)
    {
        m_kvdb = other.m_kvdb;
        other.m_initialized = false;
    }

    KVHandle& operator=(KVHandle&& other) noexcept {
        if (this != &other) {
            deinit();
            m_kvdb = other.m_kvdb;
            m_initialized = other.m_initialized;
            other.m_initialized = false;
        }
        return *this;
    }

    /**
     * @brief 初始化键值数据库
     * @param part Flash分区信息
     * @return true 成功，false 失败
     */
    bool init(fdb_partition_t *part) {
        if (m_initialized) {
            return true;
        }
        fdb_err_t err = fdb_kvdb_init(&m_kvdb, "kvdb", part, nullptr, nullptr);
        if (err == FDB_NO_ERR) {
            m_initialized = true;
            return true;
        }
        return false;
    }

    /**
     * @brief 使用默认名称初始化
     * @return true 成功
     */
    bool init(fdb_kvdb_t* existing_kvdb) {
        if (m_initialized) {
            return true;
        }
        m_kvdb = *existing_kvdb;
        m_initialized = true;
        return true;
    }

    /**
     * @brief 反初始化
     */
    void deinit() {
        if (m_initialized) {
            fdb_kvdb_deinit(&m_kvdb);
            m_initialized = false;
        }
    }

    /**
     * @brief 检查是否已初始化
     * @return true 已初始化
     */
    bool is_initialized() const {
        return m_initialized;
    }

    /**
     * @brief 获取原生fdb_kvdb句柄
     * @return fdb_kvdb_t*
     */
    fdb_kvdb_t* handle() {
        return &m_kvdb;
    }

    /**
     * @brief 获取最后错误码
     * @return fdb_err_t
     */
    fdb_err_t last_error() const {
        return m_last_error;
    }

    /**
     * @brief 检查键是否存在
     * @param key 键名
     * @return true 存在
     */
    bool has(const char* key) const {
        if (!m_initialized) {
            return false;
        }
        fdb_kv_t kv;
        fdb_err_t err = fdb_kv_get(&m_kvdb, key, &kv);
        return err == FDB_NO_ERR;
    }

    /**
     * @brief 删除一个键
     * @param key 键名
     * @return true 删除成功
     */
    bool erase(const char* key) {
        if (!m_initialized) {
            return false;
        }
        fdb_err_t err = fdb_kv_del(&m_kvdb, key);
        m_last_error = err;
        return err == FDB_NO_ERR;
    }

    /**
     * @brief 清空整个数据库
     * @return true 成功
     */
    bool clear() {
        if (!m_initialized) {
            return false;
        }
        fdb_err_t err = fdb_kvdb_clean(&m_kvdb);
        m_last_error = err;
        return err == FDB_NO_ERR;
    }

    /**
     * @brief 获取数据库已使用大小
     * @return size_t 已使用字节数
     */
    size_t used_size() const {
        if (!m_initialized) {
            return 0;
        }
        return fdb_kvdb_used_size(&m_kvdb);
    }

    /**
     * @brief 获取数据库剩余空间
     * @return size_t 剩余字节数
     */
    size_t available_size() const {
        if (!m_initialized) {
            return 0;
        }
        return m_kvdb.part->size - fdb_kvdb_used_size(&m_kvdb);
    }

    // === 整数存取 ===

    /**
     * @brief 写入32位整数
     * @param key 键名
     * @param value 整数值
     * @return true 成功
     */
    bool set_i32(const char* key, int32_t value) {
        return set_blob(key, &value, sizeof(value));
    }

    /**
     * @brief 读取32位整数
     * @param key 键名
     * @param default_val 默认值
     * @return 读取到的值或默认值
     */
    int32_t get_i32(const char* key, int32_t default_val = 0) const {
        if (!m_initialized) {
            return default_val;
        }
        int32_t value;
        size_t len = sizeof(value);
        fdb_err_t err = fdb_kv_get_blob(&m_kvdb, key, &value, &len);
        if (err != FDB_NO_ERR || len != sizeof(value)) {
            m_last_error = err;
            return default_val;
        }
        return value;
    }

    /**
     * @brief 写入32位无符号整数
     * @param key 键名
     * @param value 整数值
     * @return true 成功
     */
    bool set_u32(const char* key, uint32_t value) {
        return set_blob(key, &value, sizeof(value));
    }

    /**
     * @brief 读取32位无符号整数
     * @param key 键名
     * @param default_val 默认值
     * @return 读取到的值或默认值
     */
    uint32_t get_u32(const char* key, uint32_t default_val = 0) const {
        uint32_t value;
        size_t len = sizeof(value);
        fdb_err_t err = fdb_kv_get_blob(&m_kvdb, key, &value, &len);
        if (err != FDB_NO_ERR || len != sizeof(value)) {
            m_last_error = err;
            return default_val;
        }
        return value;
    }

    /**
     * @brief 写入8位无符号整数
     * @param key 键名
     * @param value 值
     * @return true 成功
     */
    bool set_u8(const char* key, uint8_t value) {
        return set_blob(key, &value, sizeof(value));
    }

    /**
     * @brief 读取8位无符号整数
     * @param key 键名
     * @param default_val 默认值
     * @return 值
     */
    uint8_t get_u8(const char* key, uint8_t default_val = 0) const {
        uint8_t value;
        size_t len = sizeof(value);
        fdb_err_t err = fdb_kv_get_blob(&m_kvdb, key, &value, &len);
        if (err != FDB_NO_ERR || len != sizeof(value)) {
            m_last_error = err;
            return default_val;
        }
        return value;
    }

    // === 布尔类型特殊处理 ===

    /**
     * @brief 写入布尔值
     * @param key 键名
     * @param value 值
     * @return true 成功
     */
    bool set_bool(const char* key, bool value) {
        uint8_t v = value ? 1 : 0;
        return set_blob(key, &v, sizeof(v));
    }

    /**
     * @brief 读取布尔值
     * @param key 键名
     * @param default_val 默认值
     * @return bool
     */
    bool get_bool(const char* key, bool default_val = false) const {
        uint8_t value;
        size_t len = sizeof(value);
        fdb_err_t err = fdb_kv_get_blob(&m_kvdb, key, &value, &len);
        if (err != FDB_NO_ERR || len != sizeof(value)) {
            m_last_error = err;
            return default_val;
        }
        return value != 0;
    }

    // === 字符串存取 ===

    /**
     * @brief 写入字符串
     * @param key 键名
     * @param value 字符串
     * @return true 成功
     */
    bool set_string(const char* key, std::string_view value) {
        return set_blob(key, value.data(), value.size());
    }

    /**
     * @brief 读取字符串
     * @param key 键名
     * @param default_val 默认值
     * @return std::string
     */
    std::string get_string(const char* key, const std::string& default_val = "") const {
        if (!m_initialized) {
            return default_val;
        }

        size_t len = 0;
        fdb_err_t err = fdb_kv_get(&m_kvdb, key, nullptr, &len);
        if (err != FDB_NO_ERR) {
            m_last_error = err;
            return default_val;
        }

        std::string result;
        result.resize(len);
        err = fdb_kv_get_blob(&m_kvdb, key, result.data(), &len);
        if (err != FDB_NO_ERR) {
            m_last_error = err;
            return default_val;
        }
        result.resize(len);
        return result;
    }

    // === Blob 二进制存取 ===

    /**
     * @brief 写入二进制数据
     * @param key 键名
     * @param data 数据指针
     * @param len 数据长度
     * @return true 成功
     */
    bool set_blob(const char* key, const void* data, size_t len) {
        if (!m_initialized) {
            return false;
        }
        fdb_err_t err = fdb_kv_set_blob(&m_kvdb, key, data, len);
        m_last_error = err;
        return err == FDB_NO_ERR;
    }

    /**
     * @brief 读取二进制数据
     * @param key 键名
     * @param out 输出向量
     * @return true 成功
     */
    bool get_blob(const char* key, std::vector<uint8_t>& out) const {
        if (!m_initialized) {
            return false;
        }

        size_t len = 0;
        fdb_err_t err = fdb_kv_get(&m_kvdb, key, nullptr, &len);
        if (err != FDB_NO_ERR) {
            m_last_error = err;
            return false;
        }

        out.resize(len);
        err = fdb_kv_get_blob(&m_kvdb, key, out.data(), &len);
        if (err != FDB_NO_ERR) {
            m_last_error = err;
            return false;
        }
        out.resize(len);
        return true;
    }

    /**
     * @brief 写入任意POD类型
     * @tparam T POD类型
     * @param key 键名
     * @param value 值
     * @return true 成功
     */
    template<typename T>
    typename std::enable_if<std::is_pod<T>::value, bool>::type
    set(const char* key, const T& value) {
        return set_blob(key, &value, sizeof(T));
    }

    /**
     * @brief 读取任意POD类型
     * @tparam T POD类型
     * @param key 键名
     * @param default_val 默认值
     * @return 读取的值
     */
    template<typename T>
    typename std::enable_if<std::is_pod<T>::value, T>::type
    get(const char* key, const T& default_val = T{}) const {
        if (!m_initialized) {
            return default_val;
        }
        T value;
        size_t len = sizeof(T);
        fdb_err_t err = fdb_kv_get_blob(&m_kvdb, key, &value, &len);
        if (err != FDB_NO_ERR || len != sizeof(T)) {
            m_last_error = err;
            return default_val;
        }
        return value;
    }

    /**
     * @brief 遍历所有键值对
     * @tparam Func 回调函数 void func(const char* key, const void* data, size_t len)
     * @param callback 回调函数
     */
    template<typename Func>
    void foreach(Func callback) {
        if (!m_initialized) {
            return;
        }
        fdb_kv_t kv;
        fdb_node_t *node;

        fdb_kvdb_control(&m_kvdb, FDB_KVDB_CTRL_GET_HEAD_NODE, &node);
        while (node != nullptr) {
            fdb_err_t err = fdb_kv_get_node_kv(node, &kv);
            if (err == FDB_NO_ERR) {
                callback(kv.key, kv.value, kv.len);
            }
            node = node->next;
        }
    }

    // === 代理类支持 [] 操作符 ===

    /**
     * @brief 代理类，用于支持 [] 运算符存取
     */
    class Proxy {
    public:
        Proxy(KVHandle* handle, const char* key)
            : m_handle(handle)
            , m_key(key)
        {}

        // 赋值: kv["key"] = value
        Proxy& operator=(int32_t value) {
            m_handle->set_i32(m_key, value);
            return *this;
        }

        Proxy& operator=(uint32_t value) {
            m_handle->set_u32(m_key, value);
            return *this;
        }

        Proxy& operator=(uint8_t value) {
            m_handle->set_u8(m_key, value);
            return *this;
        }

        Proxy& operator=(bool value) {
            m_handle->set_bool(m_key, value);
            return *this;
        }

        Proxy& operator=(std::string_view value) {
            m_handle->set_string(m_key, value);
            return *this;
        }

        Proxy& operator=(const std::string& value) {
            m_handle->set_string(m_key, value);
            return *this;
        }

        Proxy& operator=(const char* value) {
            m_handle->set_string(m_key, value);
            return *this;
        }

        // 浮点数支持
        Proxy& operator=(double value) {
            m_handle->set(m_key, value);
            return *this;
        }

        Proxy& operator=(float value) {
            m_handle->set(m_key, value);
            return *this;
        }

        // 类型转换读取
        operator int32_t() const {
            return m_handle->get_i32(m_key);
        }

        operator uint32_t() const {
            return m_handle->get_u32(m_key);
        }

        operator uint8_t() const {
            return m_handle->get_u8(m_key);
        }

        operator bool() const {
            return m_handle->get_bool(m_key);
        }

        operator std::string() const {
            return m_handle->get_string(m_key);
        }

        operator double() const {
            return m_handle->get<double>(m_key);
        }

        operator float() const {
            return m_handle->get<float>(m_key);
        }

    private:
        KVHandle* m_handle;
        const char* m_key;
    };

    /**
     * @brief [] 操作符，方便存取
     * @param key 键名
     * @return Proxy 代理对象
     */
    Proxy operator[](const char* key) {
        return Proxy(this, key);
    }

    /**
     * @brief 布尔转换
     * @return true 已初始化成功
     */
    explicit operator bool() const {
        return is_initialized();
    }

private:
    fdb_kvdb_t m_kvdb;
    bool m_initialized = false;
    mutable fdb_err_t m_last_error = FDB_NO_ERR;
};
