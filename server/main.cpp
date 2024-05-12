// built off of:
// https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa
#include "main.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <any>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
using json = nlohmann::json;

#define PORT 8080
#define BUFFER_LEN 3000

// next steps:
//  - convert chars to vector of std::byte (this is how we serialize/deserialize
//  datatypes):
//  https://stackoverflow.com/questions/46150738/how-to-use-new-stdbyte-type-in-places-where-old-style-unsigned-char-is-needed
//  - create opcode mapping + function execution threads
//  - create thread-safe KV store
//  - write to the KV store from the function execution threads
//  - create telemetry thread (serializing KV store "state" and sending out)
//  - create python client
//  - create pytests
//  - nice to have: logging

int main() {
  // this sets up a socket to communicate on port 8080
  // this current setup to test the TCP connection is to just respond with
  // "Hello from server"

  // using C-style arrays, memcpy, C-style strings (char *), etc. is obviously
  // not ideal in modern C++, but it is a necessary evil for using the Linux
  // socket API

  std::string file_path = "data/test.json";
  JsonData defn = parseDefinitionFile(file_path);

  int server_fd{};
  int new_socket{};
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  const char *hello = "Hello from server";

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    std::cerr << "Error opening socket" << std::endl;
    return EXIT_FAILURE;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  memset(address.sin_zero, '\0', sizeof address.sin_zero);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "Error in bind" << std::endl;
    return EXIT_FAILURE;
  }

  if (listen(server_fd, 10) < 0) {
    std::cerr << "Error in listen" << std::endl;
    return EXIT_FAILURE;
  }

  while (true) {
    printf("\n+++Waiting for new connection+++\n\n");

    // wait for a new connection (a blocking operation!)
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      std::cerr << "Error in accepting connection" << std::endl;
      return EXIT_FAILURE;
    }

    // read in bytes from the socket (a blocking operation!)
    char buffer[BUFFER_LEN] = {0};
    long num_bytes_read = read(new_socket, buffer, BUFFER_LEN);

    // remove trailing newline, if exists
    if (buffer[num_bytes_read - 1] == '\n') buffer[num_bytes_read - 1] = '\0';
    printf("Message: \"%s\"\nNumber of bytes: %ld\n", buffer, num_bytes_read);
    std::string message(buffer);
    handleCommand(message);

    // Respond over the same socket
    write(new_socket, hello, strlen(hello));
    printf("---Hello message sent---\n");
    close(new_socket);
  }

  return 0;
}

// TODO: why did making this argument std::string &message cause a linker error?
// Make this by reference to avoid unneccessary copies
void handleCommand(std::string message) {
  std::cout << "Command to handle: " << message << std::endl;
  // TODO: define a command packet structure
  //
  //  - PACKET_LENGTH (2 bytes)
  //  - OPCODE (2 bytes)
  //  - PAYLOAD (n bytes)
  //  - checksum (2 bytes, protocol TBD)
  //
  // TODO: how to handle looking up the opcode?
  //
  // cmd definition file: JSON (use JSON library: reference nlohmann/json using
  // http_archive or git_repository and depend on @nlohmann_json//:json.) cmd
  // definition file format: {
  //    command: {
  //      name: "name",
  //      opcode: "0xFF"
  //      // How to handle hex literal here? just a string we parse into hex?
  //      arguments: {
  //          arg_name_1: "type"
  //      }
  //    }
  // }
  //
  // where the argument data type is:
  // "string" | "(u)int(8/16/32/64)" | "double" | "float" | "bool"
  //
  // TODO: (low-priority) look into supporting enums:
  // https://stackoverflow.com/questions/7163069/c-string-to-enum
  //
  //
  // TODO: create a map of function names -> functions
  // is there a way to programmatically apply a generic argument list to its
  // corresponding function? might be able to do some funky template stuff
  // here...
  //
  // TODO: implement running functions on their own threads, and joining the
  // threads after completion
  //
  // TODO: implement the mutexed unordered_map (or another solution?) and test
  // for race conditions
  //
  // TODO: implement telemetry processing (vague, for now)
  //
  // TODO: implement python client for sending cmds through a CLI and reading a
  // stream of tlm over a socket
  //
  // TODO: use pytest to validate the commands being functional
}

JsonData parseDefinitionFile(std::string file_path) {
  std::ifstream file(file_path);
  if (!file) {
    std::cerr << "Cannot open file: " << file_path << std::endl;
  }

  JsonData jsonData{};

  json data = json::parse(file);
  // std::cout << data;
  auto commands = data["commands"];
  for (auto &command : commands) {
    std::cout << command.dump(2) << std::endl;

    // goal: map string (command argument from JSON) to type (uint32, float64,
    // etc.) how:
    //  - store type as string
    //  - store size of the type (from a handmade map)
    //  - store tuple<name, type, size> as part of the command data object
    //  - when parsing data, parse size bytes & save into a variant
    //  - then, unpack that variant using a mega switch statement mapping the
    //  type string to a type
    std::vector<Argument> args{};
    for (const auto &[key, value] : command["arguments"].items()) {
      args.push_back({key, value, type_to_size(value)});
      // std::cout << "arg type " << argument.type
      // << " has size: " << argument.size << std::endl;
    }

    CommandData cmdData{command["name"], args};
    jsonData.map.insert({{command["opcode"], cmdData}});
  }

  std::cout << jsonData.map.size() << std::endl;
  return jsonData;
}
