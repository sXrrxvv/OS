#include "marker.h"

std::vector<HANDLE> startThreads(int count){
    std::vector<HANDLE> threadsHandles(count);
    for (int i = 0; i < count; i++){
        HANDLE hThread;
        DWORD IDThread;
        hThread = CreateThread(
                NULL,
                0,
                marker,
                (void*)new markerParams(i + 1),
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

HANDLE* CreateEvents(int count, bool manualReset, bool initialState){
    HANDLE* events = new HANDLE[count];
    for (int i = 0; i < count; i++){
        events[i] = CreateEvent(NULL, manualReset, initialState, NULL);
    }
    return events;
}

void print(const std::vector<unsigned int>& v){
    for (int i = 0; i < v.size(); i++)
        std::cout << v[i] << " ";
    std::cout << '\n';
}

void SetRemovedEvents(const std::vector<HANDLE>& removedEvents){
    for (int i = 0; i < removedEvents.size(); i++){
        SetEvent(removedEvents[i]);
    }
}

int main() {

    InitializeCriticalSection(&markersVectorAccessCritSection);

    std::cout << "Enter array size: ";
    size_t arrSize;
    std::cin >> arrSize;
    try {
        sharedVector = std::vector<unsigned int>(arrSize, 0);https://github.com/sXrrxvv/OS
    }
    catch (std::bad_alloc&) {
        std::cout << "Cant allocate too much memory";
        return 0;
    }
    catch (std::length_error&){
        std::cout << "size must be positive";
        return 0;
    }
    std::cout << "Enter markers count: ";
    unsigned int markerCount;
    std::cin >> markerCount;
    markedEventsHandlers = CreateEvents(markerCount, FALSE, FALSE);
    continueEventHandler = CreateEvent(NULL, TRUE, FALSE, NULL);
    closeThreadEventsHandlers = CreateEvents(markerCount, TRUE, FALSE);
    std::vector<HANDLE> threadsHandles = startThreads(markerCount);

    std::vector<HANDLE> removedMarkedEventsHandlers;

    unsigned int activeMarkers = markerCount;
    while (activeMarkers != 0){
        SetRemovedEvents(removedMarkedEventsHandlers);
        WaitForMultipleObjects(markerCount, markedEventsHandlers, TRUE, INFINITE);
        print(sharedVector);
        std::cout << "Enter # of marker to be closed: \n";
        unsigned int num;
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
    std::cout << "result array : \n";
    print(sharedVector);
    std::cout << "\n";

    removedMarkedEventsHandlers.clear();
    for(int i = 0; i < markerCount; ++i){
        CloseHandle(closeThreadEventsHandlers[i]);
        CloseHandle(markedEventsHandlers[i]);
        CloseHandle(removedMarkedEventsHandlers[i]);
        CloseHandle(threadsHandles[i]);
    }
    CloseHandle(continueEventHandler);
    DeleteCriticalSection(&markersVectorAccessCritSection);
    sharedVector.clear();
    std::cout <<"Done cleaning";

}
