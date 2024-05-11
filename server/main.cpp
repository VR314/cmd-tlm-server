// built off of:
// https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#define PORT 8080
#define BUFFER_LEN 3000

int main() {
  // this sets up a socket to communicate on port 8080
  // this current setup to test the TCP connection is to just respond with
  // "Hello from server"

  int server_fd{};
  int new_socket{};
  long valread{};
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
    printf("\n+++++++ Waiting for new connection ++++++++\n\n");
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      std::cerr << "Error in accepting connection" << std::endl;
      return EXIT_FAILURE;
    }

    char buffer[BUFFER_LEN] = {0};
    valread = read(new_socket, buffer, 30000);
    printf("%s\n", buffer);
    write(new_socket, hello, strlen(hello));
    printf("------------------Hello message sent-------------------\n");
    close(new_socket);
  }
  return 0;
}
