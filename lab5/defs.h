#include <string>

enum EntryState {
    IS_FREE,
    IS_BEING_READ,
    IS_BEING_MODIFIED
};

const int offset = 2;
const int notExistIndex = -1;
const int MESSAGE_MAX_SIZE = 16;
const int maxNameLenght = 10;
const std::string pipeName = "\\\\.\\pipe\\lab5";
const std::string clientExeName = "client.exe";
const std::string startAllEventName ="startAll";