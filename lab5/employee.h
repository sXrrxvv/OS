#include "defs.h"

class employee{
private:
    int id;
    char name[maxNameLenght];
    double hours;

public:
    employee() : id(-1), name(), hours() {}

    void clear(){
        id = -1;
        for (int i = 0; i < 10; i++) name[i] = 0;
        hours = 0.0;
    }

    int getId(){
        return id;
    }

    void setId(int rId){
        id = rId;
    }
    
    friend std::fstream& operator<<(std::fstream& fs, const employee& emp);
    friend std::ostream& operator << (std::ostream& os, const employee& emp);
    friend std::istream& operator >> (std::istream& is, employee& em);
    friend std::fstream& operator>>(std::fstream& fs, employee& emp);

};

std::fstream &operator<<(std::fstream &fs, const employee &emp){
    fs.write((const char*)&emp, sizeof(employee));
    return fs;
}

std::fstream &operator>>(std::fstream &fs, employee &emp){
    fs.read((char*)&emp, sizeof(employee));
    return fs;
}

std::ostream& operator << (std::ostream& os, const employee& em){
    os << "id " << em.id << " name " << em.name << " hours " << em.hours << '\n';
    return os;
}

std::istream& operator >> (std::istream& is, employee& em){
    is >> em.id >> em.name >> em.hours;
    return is;
}