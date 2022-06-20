#include <string>
#include <sstream>

typedef unsigned int uInt;
enum optionType{
    modification,
    read,
    stop
};

const std::string clientExeName = "client.exe";
const std::string pipeName = "pipe";
const std::ostringstream eventName("event");
