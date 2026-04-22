#pragma once

#include "../deps/MyTinySTL/string"
#include <string_view>
#include "../deps/MyTinySTL/vector"
#include "../deps/MyTinySTL/memory"
#include <cstdint>
#include "../deps/MyTinySTL/type_traits"

#ifdef CONFIG_ENABLE_NVS
#include "NVSHandle.hpp"
#endif
#ifdef CONFIG_ENABLE_FLASHDB
#include "KVHandle.hpp"
#endif

/**
 * @brief 统一键值数据库接口
 * 支持选择后端: NVS 或 Flashdb
 *
 * 使用示例:
 * @code
 *   // 使用NVS后端
 *   KVDB db(KVDB::Backend::NVS, "mynamespace");
 *   if (db.open()) {
 *       db["key"] = 123;
 *       int v = db["key"];
 *   }
 *
 *   // 使用Flashdb后端
 *   KVDB db(KVDB::Backend::Flashdb, &my_partition);
 *   if (db.open()) {
 *       db["key"] = 123;
 *       int v = db["key"];
 *   }
 * @endcode
 */
class KVDB {
public:
    /**
     * @brief 后端类型选择
     */
    enum class Backend {
        NVS,        ///< 使用ESP-IDF原生NVS
        Flashdb     ///< 使用Flashdb
    };

    // 前置声明实现类
    class ImplBase;

    /**
     * @brief 构造函数 - 使用NVS后端
     * @param ns_name NVS命名空间名称
     * @param open_mode 打开模式
     */
    explicit KVDB(const char* ns_name, nvs_open_mode_t open_mode = NVS_READWRITE);

    /**
     * @brief 构造函数 - 选择后端类型
     * @param type 后端类型
     * @param arg 根据后端类型不同:
     *            - NVS: const char* 命名空间名称
     *            - Flashdb: fdb_partition_t* Flash分区指针
     */
    KVDB(Backend type, void* arg);

    /**
     * @brief 析构
     */
    ~KVDB();

    // 不允许拷贝
    KVDB(const KVDB&) = delete;
    KVDB& operator=(const KVDB&) = delete;

    // 允许移动
    KVDB(KVDB&& other) noexcept;
    KVDB& operator=(KVDB&& other) noexcept;

    /**
     * @brief 打开/初始化数据库
     * @return true 成功
     */
    bool open();

    /**
     * @brief 关闭/反初始化
     */
    void close();

    /**
     * @brief 提交更改
     * @return true 成功
     */
    bool commit();

    /**
     * @brief 检查是否已打开
     * @return true 已打开
     */
    bool is_open() const;

    /**
     * @brief 检查键是否存在
     * @param key 键名
     * @return true 存在
     */
    bool has(const char* key) const;

    /**
     * @brief 删除键
     * @param key 键名
     * @return true 删除成功
     */
    bool erase(const char* key);

    /**
     * @brief 擦除所有数据
     * @return true 成功
     */
    bool erase_all();

    // === 类型化存取 ===

    bool set_i32(const char* key, int32_t value);
    int32_t get_i32(const char* key, int32_t default_val = 0) const;

    bool set_u32(const char* key, uint32_t value);
    uint32_t get_u32(const char* key, uint32_t default_val = 0) const;

    bool set_u8(const char* key, uint8_t value);
    uint8_t get_u8(const char* key, uint8_t default_val = 0) const;

    bool set_bool(const char* key, bool value);
    bool get_bool(const char* key, bool default_val = false) const;

    bool set_string(const char* key, std::string_view value);
    std::string get_string(const char* key, const std::string& default_val = "") const;

    bool set_blob(const char* key, const void* data, size_t len);
    bool get_blob(const char* key, std::vector<uint8_t>& out) const;

    /**
     * @brief 写入任意POD类型
     */
    template<typename T>
    typename std::enable_if<std::is_pod<T>::value, bool>::type
    set(const char* key, const T& value) {
        return set_blob(key, &value, sizeof(T));
    }

    /**
     * @brief 读取任意POD类型
     */
    template<typename T>
    typename std::enable_if<std::is_pod<T>::value, T>::type
    get(const char* key, const T& default_val = T{}) const {
        if (!is_open()) {
            return default_val;
        }
        T value;
        size_t len = sizeof(T);
        if (!get_blob(key, reinterpret_cast<uint8_t*>(&value), len)) {
            return default_val;
        }
        if (len != sizeof(T)) {
            return default_val;
        }
        return value;
    }

    /**
     * @brief 获取可用空间 (仅Flashdb支持)
     */
    size_t available_size() const;

    /**
     * @brief 获取已用空间 (仅Flashdb支持)
     */
    size_t used_size() const;

    /**
     * @brief 获取当前后端类型
     */
    Backend backend_type() const;

    // === 代理类支持 [] 操作符 ===

    class Proxy {
    public:
        Proxy(KVDB* db, const char* key) : m_db(db), m_key(key) {}

        Proxy& operator=(int32_t value) {
            m_db->set_i32(m_key, value);
            return *this;
        }

        Proxy& operator=(uint32_t value) {
            m_db->set_u32(m_key, value);
            return *this;
        }

        Proxy& operator=(uint8_t value) {
            m_db->set_u8(m_key, value);
            return *this;
        }

        Proxy& operator=(bool value) {
            m_db->set_bool(m_key, value);
            return *this;
        }

        Proxy& operator=(std::string_view value) {
            m_db->set_string(m_key, value);
            return *this;
        }

        Proxy& operator=(const std::string& value) {
            m_db->set_string(m_key, value);
            return *this;
        }

        Proxy& operator=(const char* value) {
            m_db->set_string(m_key, value);
            return *this;
        }

        Proxy& operator=(double value) {
            m_db->set(m_key, value);
            return *this;
        }

        Proxy& operator=(float value) {
            m_db->set(m_key, value);
            return *this;
        }

        operator int32_t() const { return m_db->get_i32(m_key); }
        operator uint32_t() const { return m_db->get_u32(m_key); }
        operator uint8_t() const { return m_db->get_u8(m_key); }
        operator bool() const { return m_db->get_bool(m_key); }
        operator std::string() const { return m_db->get_string(m_key); }
        operator double() const { return m_db->get<double>(m_key); }
        operator float() const { return m_db->get<float>(m_key); }

    private:
        KVDB* m_db;
        const char* m_key;
    };

    Proxy operator[](const char* key) {
        return Proxy(this, key);
    }

    /**
     * @brief 布尔转换
     */
    explicit operator bool() const {
        return is_open();
    }

private:
    // 私有实现基类
    class ImplBase {
    public:
        virtual ~ImplBase() = default;
        virtual bool open() = 0;
        virtual void close() = 0;
        virtual bool commit() = 0;
        virtual bool is_open() const = 0;
        virtual bool has(const char* key) const = 0;
        virtual bool erase(const char* key) = 0;
        virtual bool erase_all() = 0;

        virtual bool set_i32(const char* key, int32_t value) = 0;
        virtual int32_t get_i32(const char* key, int32_t default_val) const = 0;
        virtual bool set_u32(const char* key, uint32_t value) = 0;
        virtual uint32_t get_u32(const char* key, uint32_t default_val) const = 0;
        virtual bool set_u8(const char* key, uint8_t value) = 0;
        virtual uint8_t get_u8(const char* key, uint8_t default_val) const = 0;
        virtual bool set_bool(const char* key, bool value) = 0;
        virtual bool get_bool(const char* key, bool default_val) const = 0;
        virtual bool set_string(const char* key, std::string_view value) = 0;
        virtual std::string get_string(const char* key, const std::string& default_val) const = 0;
        virtual bool set_blob(const char* key, const void* data, size_t len) = 0;
        virtual bool get_blob(const char* key, uint8_t* out, size_t& len) const = 0;
        virtual size_t available_size() const { return 0; }
        virtual size_t used_size() const { return 0; }
        virtual Backend backend_type() const = 0;
    };

#ifdef CONFIG_ENABLE_NVS
    class NVSImpl;
#endif
#ifdef CONFIG_ENABLE_FLASHDB
    class FlashdbImpl;
#endif

    bool get_blob(const char* key, uint8_t* out, size_t& len) const;

    std::unique_ptr<ImplBase> m_impl;
};

// === 实现部分 - 内联在头文件中 ===

#ifdef CONFIG_ENABLE_NVS
class KVDB::NVSImpl : public KVDB::ImplBase {
public:
    NVSImpl(const char* ns_name, nvs_open_mode_t mode)
        : m_nvs(ns_name, mode)
    {}

    bool open() override {
        return m_nvs.open();
    }

    void close() override {
        m_nvs.close();
    }

    bool commit() override {
        return m_nvs.commit();
    }

    bool is_open() const override {
        return m_nvs.is_open();
    }

    bool has(const char* key) const override {
        return m_nvs.has(key);
    }

    bool erase(const char* key) override {
        return m_nvs.erase(key);
    }

    bool erase_all() override {
        return m_nvs.erase_all();
    }

    bool set_i32(const char* key, int32_t value) override {
        return m_nvs.set_i32(key, value);
    }

    int32_t get_i32(const char* key, int32_t default_val) const override {
        return m_nvs.get_i32(key, default_val);
    }

    bool set_u32(const char* key, uint32_t value) override {
        return m_nvs.set_u32(key, value);
    }

    uint32_t get_u32(const char* key, uint32_t default_val) const override {
        return m_nvs.get_u32(key, default_val);
    }

    bool set_u8(const char* key, uint8_t value) override {
        return m_nvs.set_u8(key, value);
    }

    uint8_t get_u8(const char* key, uint8_t default_val) const override {
        return m_nvs.get_u8(key, default_val);
    }

    bool set_bool(const char* key, bool value) override {
        // NVS没有原生bool，用u8存储
        return m_nvs.set_u8(key, value ? 1 : 0);
    }

    bool get_bool(const char* key, bool default_val) const override {
        uint8_t v = m_nvs.get_u8(key, default_val ? 1 : 0);
        return v != 0;
    }

    bool set_string(const char* key, std::string_view value) override {
        return m_nvs.set_string(key, value);
    }

    std::string get_string(const char* key, const std::string& default_val) const override {
        return m_nvs.get_string(key, default_val);
    }

    bool set_blob(const char* key, const void* data, size_t len) override {
        return m_nvs.set_blob(key, data, len);
    }

    bool get_blob(const char* key, uint8_t* out, size_t& len) const override {
        // NVS需要先获取长度
        size_t stored_len = len;
        esp_err_t err = nvs_get_blob(m_nvs.handle(), key, out, &stored_len);
        if (err != ESP_OK) {
            return false;
        }
        len = stored_len;
        return true;
    }

    Backend backend_type() const override {
        return Backend::NVS;
    }

private:
    NVSHandle m_nvs;
};
#endif

#ifdef CONFIG_ENABLE_FLASHDB
class KVDB::FlashdbImpl : public KVDB::ImplBase {
public:
    FlashdbImpl(fdb_partition_t* part)
        : m_part(part)
    {}

    bool open() override {
        return m_kv.init(m_part);
    }

    void close() override {
        m_kv.deinit();
    }

    bool commit() override {
        // Flashdb自动提交，这里总是成功
        return true;
    }

    bool is_open() const override {
        return m_kv.is_initialized();
    }

    bool has(const char* key) const override {
        return m_kv.has(key);
    }

    bool erase(const char* key) override {
        return m_kv.erase(key);
    }

    bool erase_all() override {
        return m_kv.clear();
    }

    bool set_i32(const char* key, int32_t value) override {
        return m_kv.set_i32(key, value);
    }

    int32_t get_i32(const char* key, int32_t default_val) const override {
        return m_kv.get_i32(key, default_val);
    }

    bool set_u32(const char* key, uint32_t value) override {
        return m_kv.set_u32(key, value);
    }

    uint32_t get_u32(const char* key, uint32_t default_val) const override {
        return m_kv.get_u32(key, default_val);
    }

    bool set_u8(const char* key, uint8_t value) override {
        return m_kv.set_u8(key, value);
    }

    uint8_t get_u8(const char* key, uint8_t default_val) const override {
        return m_kv.get_u8(key, default_val);
    }

    bool set_bool(const char* key, bool value) override {
        return m_kv.set_bool(key, value);
    }

    bool get_bool(const char* key, bool default_val) const override {
        return m_kv.get_bool(key, default_val);
    }

    bool set_string(const char* key, std::string_view value) override {
        return m_kv.set_string(key, value);
    }

    std::string get_string(const char* key, const std::string& default_val) const override {
        return m_kv.get_string(key, default_val);
    }

    bool set_blob(const char* key, const void* data, size_t len) override {
        return m_kv.set_blob(key, data, len);
    }

    bool get_blob(const char* key, uint8_t* out, size_t& len) const override {
        std::vector<uint8_t> tmp;
        if (!m_kv.get_blob(key, tmp)) {
            return false;
        }
        if (tmp.size() <= len) {
            memcpy(out, tmp.data(), tmp.size());
            len = tmp.size();
            return true;
        }
        len = tmp.size();
        return false;
    }

    size_t available_size() const override {
        return m_kv.available_size();
    }

    size_t used_size() const override {
        return m_kv.used_size();
    }

    Backend backend_type() const override {
        return Backend::Flashdb;
    }

private:
    fdb_partition_t* m_part;
    KVHandle m_kv;
};
#endif

// === 外部接口实现 ===

inline KVDB::KVDB(const char* ns_name, nvs_open_mode_t open_mode) {
#ifdef CONFIG_ENABLE_NVS
    m_impl = std::make_unique<NVSImpl>(ns_name, open_mode);
#else
    #error "NVS backend not enabled, please define CONFIG_ENABLE_NVS"
#endif
}

inline KVDB::KVDB(Backend type, void* arg) {
    if (type == Backend::NVS) {
#ifdef CONFIG_ENABLE_NVS
        m_impl = std::make_unique<NVSImpl>(static_cast<const char*>(arg), NVS_READWRITE);
#else
        #error "NVS backend not enabled"
#endif
    } else if (type == Backend::Flashdb) {
#ifdef CONFIG_ENABLE_FLASHDB
        m_impl = std::make_unique<FlashdbImpl>(static_cast<fdb_partition_t*>(arg));
#else
        #error "Flashdb backend not enabled"
#endif
    }
}

inline KVDB::~KVDB() = default;

inline KVDB::KVDB(KVDB&& other) noexcept
    : m_impl(std::move(other.m_impl))
{}

inline KVDB& KVDB::operator=(KVDB&& other) noexcept {
    m_impl = std::move(other.m_impl);
    return *this;
}

inline bool KVDB::open() {
    return m_impl->open();
}

inline void KVDB::close() {
    m_impl->close();
}

inline bool KVDB::commit() {
    return m_impl->commit();
}

inline bool KVDB::is_open() const {
    return m_impl->is_open();
}

inline bool KVDB::has(const char* key) const {
    return m_impl->has(key);
}

inline bool KVDB::erase(const char* key) {
    return m_impl->erase(key);
}

inline bool KVDB::erase_all() {
    return m_impl->erase_all();
}

inline bool KVDB::set_i32(const char* key, int32_t value) {
    return m_impl->set_i32(key, value);
}

inline int32_t KVDB::get_i32(const char* key, int32_t default_val) const {
    return m_impl->get_i32(key, default_val);
}

inline bool KVDB::set_u32(const char* key, uint32_t value) {
    return m_impl->set_u32(key, value);
}

inline uint32_t KVDB::get_u32(const char* key, uint32_t default_val) const {
    return m_impl->get_u32(key, default_val);
}

inline bool KVDB::set_u8(const char* key, uint8_t value) {
    return m_impl->set_u8(key, value);
}

inline uint8_t KVDB::get_u8(const char* key, uint8_t default_val) const {
    return m_impl->get_u8(key, default_val);
}

inline bool KVDB::set_bool(const char* key, bool value) {
    return m_impl->set_bool(key, value);
}

inline bool KVDB::get_bool(const char* key, bool default_val) const {
    return m_impl->get_bool(key, default_val);
}

inline bool KVDB::set_string(const char* key, std::string_view value) {
    return m_impl->set_string(key, value);
}

inline std::string KVDB::get_string(const char* key, const std::string& default_val) const {
    return m_impl->get_string(key, default_val);
}

inline bool KVDB::set_blob(const char* key, const void* data, size_t len) {
    return m_impl->set_blob(key, data, len);
}

inline bool KVDB::get_blob(const char* key, uint8_t* out, size_t& len) const {
    return m_impl->get_blob(key, out, len);
}

inline bool KVDB::get_blob(const char* key, std::vector<uint8_t>& out) const {
    size_t len = 0;
    if (!m_impl->get_blob(key, nullptr, len)) {
        return false;
    }
    out.resize(len);
    return m_impl->get_blob(key, out.data(), len);
}

inline size_t KVDB::available_size() const {
    return m_impl->available_size();
}

inline size_t KVDB::used_size() const {
    return m_impl->used_size();
}

inline KVDB::Backend KVDB::backend_type() const {
    return m_impl->backend_type();
}
