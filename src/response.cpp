#include <bit>
#include <cstring>

#include "response.hpp"
#include "writer.hpp"

std::vector<uint8_t> kafka::response::serialize(const Response &response) {
  std::vector<uint8_t> ret;
  writer::write_be(ret, 10);
  writer::write_be(ret, response.correlation_id);
  writer::write_be(ret, static_cast<uint16_t>(response.error));

  return ret;
}