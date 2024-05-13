#include <any>
#include <cassert>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct Argument {
  std::string name;
  std::string type;
  std::size_t size;
};

struct CommandData {
  std::string name;
  std::vector<Argument> args;

  [[nodiscard]] size_t hash() const { return std::hash<std::string>()(name); }
};

struct JsonData {
  // mapping between opcode and pair of name & function pointer
  // function pointer is std::any because of this:
  // https://stackoverflow.com/questions/55520876/creating-an-unordered-map-of-stdfunctions-with-any-arguments
  std::unordered_map<std::string, CommandData> map;
};

std::size_t type_to_size(std::string type) {
  // TODO: how to support strings as arguments?
  // TODO: how to make this not be re-allocated each time... without getting the
  // static storage error
  std::unordered_map<std::string, std::size_t> map{
      // unsigned integers:
      {"uint8_t", sizeof(uint8_t)},
      {"uint16_t", sizeof(uint16_t)},
      {"uint32_t", sizeof(uint32_t)},
      {"uint64_t", sizeof(uint64_t)},
      // signed integers
      {"int8_t", sizeof(int8_t)},
      {"int16_t", sizeof(int16_t)},
      {"int32_t", sizeof(int32_t)},
      {"int64_t", sizeof(int64_t)},
      // floating point
      {"double", sizeof(double)},
      {"float", sizeof(float)},

      {"bool", 1},  // extend bool to one full byte
      {"char", sizeof(char)}};

  assert(map.find(type) != map.end());  // type not valid!
  return map[type];
}

void handleCommand(char* buffer, JsonData& defn);
JsonData parseDefinitionFile(std::string file_path);
