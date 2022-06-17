class message{
private:
    char msg[20];

public:
    message() : msg() {;}

    explicit message(const std::string &s) {
        if(s.size() > 20)
            throw std::length_error("bad capacity");
        std::copy(s.begin(), s.end(), msg);
    }

    char* getMessage() {return msg;}

    friend std::fstream& operator << (std::fstream &buf, const message& msg) {
        buf.write((const char*)&msg , sizeof(msg));
        return buf;
    }

    friend std::fstream& operator >> (std::fstream &ifs, message &message)
    {
        ifs.read((char*)& message, sizeof(message));
        return ifs;
    }

    friend std::istream& operator>>(std::istream &is, message &msg) {
        is >> msg.msg;
        return is;
    }
};
