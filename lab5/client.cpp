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

bool initializeHandles(){

}

bool changeEmployee(employee& empl){

}

void connectToServer(){

}
