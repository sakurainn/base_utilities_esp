#pragma once
#ifndef OCPP_UUID_HPP
#define OCPP_UUID_HPP

#include "sdkconfig.h"

#include <string>


class UUID {
public:
    /// @brief Generate a new random UUID string
    /// @return UUID string in canonical format (36 characters)
    static std::string generate();
    /// @brief Generate a new random UUID into a buffer
    /// @param buffer Buffer of at least 37 bytes to hold the UUID
    static void generate(char* buffer);
};



#endif // OCPP_UUID_HPP
