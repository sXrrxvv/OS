#include <iostream>

struct employee
{
private:
    int num;
    char name[10];
    double hours;

public:
    employee();

    friend std::ostream& operator <<(std::ofstream& of, const employee& em);

    friend std::fstream& operator << (std::fstream& fs, const employee& em);

    friend std::fstream& operator >> (std::fstream& fs, employee& em);
};
