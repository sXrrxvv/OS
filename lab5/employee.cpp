#include "employee.h"
#include <fstream>
#include "string.h"

std::fstream& operator<<(std::fstream &fs, const employee &em)
{
    fs.write((const char*)&em, sizeof(employee));
    return fs;
}

int employee::getNum() {return num;}

char* employee::getName() {return name;}

double employee::getHours() {return hours;}

void employee::setNum(int num) {
    this->num = num;
}

void employee::clear()
{
    num = -1;
    for (int i = 0; i < 10; i++)
        name[i] = 0;
    hours = 0.0;
}


std::fstream& operator>>(std::fstream &fs, employee &em)
{
    fs.read((char*)&em, sizeof(employee));
    return fs;
}

std::ostream& operator <<(std::ostream& of, const employee& em){
    of << "id : " << em.num << '\n' << "name : " << em.name << "\n" << "hours : " << em.hours << '\n';
    return of;
}

employee::employee(int rNum, char* rName, double rHours){
    num = rNum;
    strcpy(name, rName);
    hours = rHours;
}

std::istream &operator>>(std::istream &is, employee &em) {
    is >> em.num >> em.name >> em.hours;
    return is;
}
