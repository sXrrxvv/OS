#include <string>
#include "message.h"
#include "fstream"

char* message::getMessage() {
    return msg;
}

std::fstream &operator<<(std::fstream &buf, const message &msg) {
    buf.write((const char*)&msg , sizeof(msg));
    return buf;
}

std::fstream &operator>>(std::fstream &ifs, message &message){
    ifs.read((char*)& message, sizeof(message));
    return ifs;
}

std::istream &operator>>(std::istream &is, message &msg) {
    is >> msg.msg;
    return is;
}








