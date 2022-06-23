#include <string>
typedef unsigned int uInt;
enum optionType{
    readMessage,
    stop,
    writeMessage
};

const int msgMaxCapacity = 20;
const std::string senderExe = "Sender.exe";
const std::string fileMutexName = "mutex";
const std::string readSemName = "readS";
const std::string maxAmountSemName = "maxS";
const std::ostringstream eventName("event");
