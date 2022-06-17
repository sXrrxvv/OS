#include <windows.h>
#include <vector>
#include <iostream>

static std::vector<unsigned int> sharedVector;

CRITICAL_SECTION markersVectorAccessCritSection;
HANDLE* markedEventsHandlers;
HANDLE* closeThreadEventsHandlers;
HANDLE continueEventHandler;

struct markerParams{
private:
    const std::vector<unsigned int>* pSharedVector;
    const unsigned int markerId;
    unsigned int markedCount;
    unsigned int cantMark;

public:
    explicit markerParams(int id) : markerId(id), pSharedVector(&sharedVector), markedCount(0), cantMark(INT_MAX) {};

    unsigned int getId () const {return markerId;}

    unsigned int getMarkedCount() const {return markedCount;}

    void increaseMarkedCount() {++markedCount;}

    size_t getVectorSize() const {return pSharedVector->size();}

    void setUnmarked(const unsigned int ind) {cantMark = ind;}

    unsigned int getUnmarked() const {return cantMark;}
};

bool setMarkedElements(markerParams& params){
    size_t vectorSize = params.getVectorSize();
    unsigned int id = params.getId();
    unsigned int randomNum = rand();
    randomNum %= vectorSize;
    if(sharedVector[randomNum] == 0){
        Sleep(5);
        sharedVector[randomNum] = id;
        Sleep(5);
        params.increaseMarkedCount();
        return true;
    }
    params.setUnmarked(randomNum);
    return false;
}

void deleteMarkedElements(const markerParams& params){
    for(size_t i = 0; i < params.getVectorSize(); ++i){
        if(sharedVector[i] == params.getId())
            sharedVector[i] = 0;
    }
}

DWORD WINAPI marker(void* paramsSettings){
    markerParams params = *static_cast<markerParams*>(paramsSettings);
    srand(params.getId());
    EnterCriticalSection(&markersVectorAccessCritSection);
    while(true) {
        if(!setMarkedElements(params))
        {
            std::cout <<"Marker number: " << params.getId() << '\t' << "Marked elements: " << params.getMarkedCount() <<
            "\t" << "Can't mark: " << params.getUnmarked() <<'\n';
            LeaveCriticalSection(&markersVectorAccessCritSection);
            SetEvent(markedEventsHandlers[params.getId() - 1]);
            HANDLE *possibleOptions = new HANDLE[2];
            possibleOptions[0] = continueEventHandler;
            possibleOptions[1] = closeThreadEventsHandlers[params.getId() - 1];
            DWORD option = WaitForMultipleObjects(2, possibleOptions, FALSE,
                                                  INFINITE);
            if (option == WAIT_OBJECT_0 + 1) {
                EnterCriticalSection(&markersVectorAccessCritSection);
                deleteMarkedElements(params);
                LeaveCriticalSection(&markersVectorAccessCritSection);
                return 0;
            }
        }
    }
}
