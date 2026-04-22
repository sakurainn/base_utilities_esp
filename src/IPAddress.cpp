#include "IPAddress.hpp"
#include <cstdio>
#include <string>

std::string IPAddress::toString() const {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d",
             (int)((m_addr >> 24) & 0xFF),
             (int)((m_addr >> 16) & 0xFF),
             (int)((m_addr >> 8) & 0xFF),
             (int)(m_addr & 0xFF));
    return std::string(buf);
}

IPAddress IPAddress::fromString(const char* str) {
    int a, b, c, d;
    if (sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        return IPAddress((uint32_t)a << 24 | (uint32_t)b << 16 | (uint32_t)c << 8 | (uint32_t)d);
    }
    return IPAddress();
}
