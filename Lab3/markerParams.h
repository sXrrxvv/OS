#include <windows.h>
#include <vector>
#include <climits>
#include <iostream>

typedef unsigned int uInt;

struct markerParams{
private:
    static std::vector<uInt>* pSharedVector;
    const uInt markerId;
    uInt markedCount;
    uInt cantMark;
    static uInt ptrCounter;

public:
    static void initializeStartVector();

    ~markerParams();

    explicit markerParams(uInt id);

    uInt getId () const;

    uInt getMarkedCount() const;

    void increaseMarkedCount();

    static size_t getVectorSize() ;

    void setUnmarked(uInt ind);

    uInt getUnmarked() const;

    void printInfo() const;

    void deleteMarkedElements();

    static void printVector();

    bool setMarkersElements();
};

bool markerParams::setMarkersElements(){
    srand(markerId);
    uInt randomNum = rand();
    randomNum %= pSharedVector->size();
    if((*pSharedVector)[randomNum] == 0){
        Sleep(5);
        (*pSharedVector)[randomNum] = markerId;
        Sleep(5);
        ++markedCount;
        return true;
    }
    cantMark = randomNum;
    return false;
}

markerParams::~markerParams() {
    if(ptrCounter)
        --ptrCounter;
    else
        delete pSharedVector;
}

void markerParams::initializeStartVector() {
    if(!ptrCounter)
        return;
    std::cout << "Enter array size: ";
    int arrSize;
    std::cin >> arrSize;
    if(arrSize <= 0)
        throw std::length_error("non positive value");
    pSharedVector = new std::vector<uInt>(arrSize);
}

markerParams::markerParams(uInt id) : markerId(id), markedCount(0), cantMark(INT_MAX){
    ++ptrCounter;
}

uInt markerParams::getId() const {
    return markerId;
}

uInt markerParams::getMarkedCount() const {
    return markedCount;
}

void markerParams::increaseMarkedCount() {
    ++markedCount;
}

size_t markerParams::getVectorSize() {
    return pSharedVector->size();
}

void markerParams::setUnmarked(uInt ind){
    cantMark = ind;
}

uInt markerParams::getUnmarked() const {
    return cantMark;
}

void markerParams::printInfo() const{
    std::cout <<"Marker number: " << this->getId() << '\t' << "Marked elements: " << this->getMarkedCount() <<
              "\t" << "Can't mark: " << this->getUnmarked() <<'\n';
}

void markerParams::deleteMarkedElements(){
    for(size_t i = 0; i < pSharedVector->size(); ++i){
        if(((*pSharedVector)[i]) == markerId)
            (*pSharedVector)[i] = 0;
    }
}

void markerParams::printVector() {
    for(size_t i = 0; i < pSharedVector->size(); ++i){
        std::cout << (*pSharedVector)[i] << " ";
    }
    std::cout << '\n';
}
