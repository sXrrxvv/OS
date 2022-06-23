#include "windows.h"
#include <iostream>
#include <vector>

CRITICAL_SECTION markersVectorAccessCritSection;
std::vector<HANDLE> markedEventsHandlers;
std::vector<HANDLE> closeThreadEventsHandlers;
std::vector<HANDLE> removedMarkedEventsHandlers;
std::vector<HANDLE> threadsHandles;
HANDLE continueEventHandler;