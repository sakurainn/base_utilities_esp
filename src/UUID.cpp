#include "UUID.hpp"

#include "simple_uuid.h"

std::string UUID::generate() {
  char buf[37];
  simple_uuid(buf);
  return std::string(buf);
}

void UUID::generate(char* buffer) { simple_uuid(buffer); }
