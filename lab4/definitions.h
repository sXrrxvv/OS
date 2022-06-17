#include <string>
typedef unsigned int uInt;
enum optionType{
    readMessage,
    stop,
    writeMessage
};

const std::string senderExe = "Sender.exe";
const std::string fileMutexName = "mutex";
const std::string readSemName = "readS";
const std::string maxAmountSemName = "maxS";
const std::ostringstream eventName("event");
