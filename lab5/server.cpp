#include <iostream>
#include "windows.h"
#include "defs.h"
#include "employee.h"
#include <vector>

CRITICAL_SECTION csFileWrite;
CRITICAL_SECTION csBlockedFlags;
HANDLE startAllEventHandle;
std::vector<HANDLE> readyEventsHandles;
std::vector<HANDLE> serverThreadsHandles;

bool createBinary(const std::string& fileName, int emplCount){

}

bool writeToBin(const std::string& fileName, const employee& em){

}

bool createPipe(){

}

bool initializeHandles(){

};

employee& findEmployeeInFile(const std::string& fileName, int num){

}

void startMessageConnection(){

}

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
