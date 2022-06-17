class message{
private:
    char msg[20];

public:
    message() : msg() {;}

    char* getMessage();

    friend std::fstream& operator << (std::fstream &buf, const message& msg);

    friend std::fstream& operator >> (std::fstream &ifs, message &message);

    friend std::istream& operator>>(std::istream &is, message &msg);
};
