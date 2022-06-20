#include "employee.h"
#include <fstream>

std::fstream& operator<<(std::fstream &fs, const employee &em)
{
    fs.write((const char*)&em, sizeof(employee));
    return fs;
}

std::fstream& operator>>(std::fstream &fs, employee &em)
{
    fs.read((char*)&em, sizeof(employee));
    return fs;
}

std::ostream& operator <<(std::ofstream& of, const employee& em){
    of << "id : " << em.num << '\n' << "name : " << em.name << "\n" << "hours : " << em.hours << '\n';
    return of;
}
