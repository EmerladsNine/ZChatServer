#include <vector>

typedef struct ClientData {
  int fd;
  int head;
  int expectedSize;
  std::vector<char> buf;
  bool operator==(const ClientData &other) const {
    return fd == other.fd; // or whatever defines equality
  }
} ClientData;
