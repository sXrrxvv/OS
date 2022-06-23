#include "marker.h"
#include "markerParams.h"

CRITICAL_SECTION markersVectorAccessCritSection;
std::vector<HANDLE> markedEventsHandlers;
std::vector<HANDLE> closeThreadEventsHandlers;
std::vector<HANDLE> removedMarkedEventsHandlers;
std::vector<HANDLE> threadsHandles;
HANDLE continueEventHandler;

std::vector<HANDLE> startThreads(int count){
    threadsHandles = std::vector<HANDLE>(count);
    for (int i = 0; i < count; i++){
        HANDLE hThread;
        DWORD IDThread;
        hThread = CreateThread(
                NULL,
                0,
                marker,
                static_cast<void*> (new markerParams(i + 1)),
                0,
                &IDThread);
        if(hThread != NULL){
            std::cout << "Marker thread #" << i + 1 << " created successfully\n";
            threadsHandles[i] = hThread;
        }
        else{
            std::cout << "Something went wrong. Error code: " << GetLastError() << '\n';
        }
    }
    return threadsHandles;
}

std::vector<HANDLE> CreateEvents(int count ,bool manualReset, bool initialState){
    std::vector<HANDLE> events = new std::vector<HANDLE>(count);
    for (int i = 0; i < count; i++){
        events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(NULL == events[i]){
            throw ;
        }
    }
    return events;
}

void initializeHandles(uInt markerCount){
    InitializeCriticalSection(&markersVectorAccessCritSection);
    if(NULL == markersVectorAccessCritSection){
        std::cerr << "ERROR: can't create crit section\n";
        DeleteCriticalSection(markersVectorAccessCritSection);
        throw ;
    }
    try {
        markedEventsHandlers = CreateEvents(markerCount, FALSE, FALSE);
    }
    catch(...){
        std::cerr << "ERROR: can't create marked events handlers\n";
        throw;
    }
    continueEventHandler = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(NULL == continueEventHandler){
        std::cerr << "ERROR: can't create continue event handle\n";
        CloseHandle(continueEventHandler);
        throw ;
    }
    try {
        closeThreadEventsHandlers = CreateEvents(markerCount, TRUE, FALSE);
    }
    catch(...) {
        std::cerr << "ERROR: can't create close events handlers\n";
        throw;
    }

}

void closeAll(uInt markersCount){
    DeleteCriticalSection(markersVectorAccessCritSection);
    CloseHandle(continueEventHandler);
    for(int i = 0; i < markersCount; ++i){
        CloseHandle(closeThreadEventsHandlers[i]);
        CloseHandle(markedEventsHandlers);
        CloseHandle(removedMarkedEventsHandlers[i]);
    }
}

void SetRemovedEvents(const std::vector<HANDLE>& removedEvents){
    for (int i = 0; i < removedEvents.size(); i++){
        SetEvent(removedEvents[i]);
    }
}

void manageMarkers(uInt markerCount){
    threadsHandles = startThreads(markerCount);
    uInt activeMarkers = markerCount;
    while (activeMarkers != 0){
        SetRemovedEvents(removedMarkedEventsHandlers);
        WaitForMultipleObjects(markerCount, markedEventsHandlers, TRUE, INFINITE);
        markerParams::printVector();
        std::cout << "Enter # of marker to be closed: \n";
        uInt num;
        std::cin >> num;
        if(num > markerCount){
            std::cout << "wrong index\n";
        }
        SetEvent(closeThreadEventsHandlers[num - 1]);
        WaitForSingleObject(threadsHandles[num - 1], INFINITE);
        removedMarkedEventsHandlers.push_back(markedEventsHandlers[num - 1]);
        activeMarkers--;
        PulseEvent(continueEventHandler);
    }
}

int main() {
    try{
        markerParams::initializeStartVector();
    }
    catch (std::bad_alloc&){
        std::cerr << "ERROR: can't allocate too much memory";
        return -1;
    }
    catch(std::length_error&){
        std::cerr << "ERROR: size must be positive";
        return -1;
    }
    std::cout << "Enter markers count: ";
    uInt markerCount;
    std::cin >> markerCount;
    try {
        initializeHandles(markerCount);
    }
    catch(...){
        std::cerr << "ERROR: initialization error";
        closeAll(markerCount);
        return -1;
    }
    manageMarkers(markerCount);
    std::cout << "result array : \n";
    markerParams::printVector();
    std::cout << "\n";
    closeAll(markerCount);
    return 0;

}
