#include <string>
#include <sstream>

typedef unsigned int uInt;
enum optionType{
    modification,
    read,
    unused,
    stop
};

const int msgMaxCapacity = 20;
const std::string clientExeName = "client.exe";
const std::string pipeName = "\\\\.\\pipe\\pipeName";
const std::ostringstream readyEventName("revent");
const std::string startAllEventName("event");
const std::string fileName = "file.bin";