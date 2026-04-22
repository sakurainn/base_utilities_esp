#pragma once

#include <string>
#include <cstdint>
#include <format>

/**
 * @brief IPv4 address class, similar to Arduino's IPAddress
 */
class IPAddress {
public:
    IPAddress() : m_addr(0) {}
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        m_addr = (uint32_t)a << 24 | (uint32_t)b << 16 | (uint32_t)c << 8 | (uint32_t)d;
    }
    explicit IPAddress(uint32_t addr) : m_addr(addr) {}

    uint32_t operator*() const { return m_addr; }
    uint32_t raw() const { return m_addr; }

    uint8_t operator[](int index) const {
        return (uint8_t)(m_addr >> (8 * (3 - index)));
    }

    bool operator==(const IPAddress& other) const {
        return m_addr == other.m_addr;
    }

    bool operator!=(const IPAddress& other) const {
        return m_addr != other.m_addr;
    }

    std::string toString() const;
    static IPAddress fromString(const char* str);

private:
    uint32_t m_addr;
};

template<>
struct std::formatter<IPAddress> : std::formatter<std::string> {
    template<typename FormatContext>
    auto format(const IPAddress& ip, FormatContext& ctx) const {
        return formatter<std::string>::format(ip.toString(), ctx);
    }
};
