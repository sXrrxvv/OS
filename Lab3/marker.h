#include "markerParams.h"

bool stopWorking(const markerParams& params){
    HANDLE *possibleOptions = new HANDLE[2];
    possibleOptions[0] = continueEventHandler;
    possibleOptions[1] = closeThreadEventsHandlers[params.getId() - 1];
    DWORD option = WaitForMultipleObjects(2, possibleOptions, FALSE,
                                          INFINITE);
    return (WAIT_OBJECT_0 + 1 == option);

}

DWORD WINAPI marker(void* paramsSettings){
    markerParams params = *static_cast<markerParams*>(paramsSettings);
    srand(params.getId());
    EnterCriticalSection(&markersVectorAccessCritSection);
    while(true) {
        int randomNum = rand();
        if(!params.setMarkersElements(randomNum))
        {
            params.printInfo();
            LeaveCriticalSection(&markersVectorAccessCritSection);
            SetEvent(markedEventsHandlers[params.getId() - 1]);
            if(stopWorking(params)){
                EnterCriticalSection(&markersVectorAccessCritSection);
                params.deleteMarkedElements();
                LeaveCriticalSection(&markersVectorAccessCritSection);
                return 0;
            }
        }
    }
}
