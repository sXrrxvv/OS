#include "defs.h"

static std::vector<int> sharedVector;
static int pCounter = 0;

struct markerParams{
private:
    const std::vector<int>* pSharedVector;
    const unsigned int markerId;
    unsigned int markedCount;
    unsigned int cantMark;
    int* ptrCounter;

public:
    explicit markerParams(int id) : markerId(id), pSharedVector(&sharedVector), markedCount(0), cantMark(INT_MAX), ptrCounter(&pCounter) {
        ++(*ptrCounter);
    };

    ~markerParams() {
        if(*ptrCounter)
            --(*ptrCounter);
        else {
            delete pSharedVector;
            delete ptrCounter;
        }
    }

    static void initializeStartVector(){
        std::cout << "Enter array size: ";
        int arrSize;
        std::cin >> arrSize;
        if(arrSize <= 0)
            throw std::length_error("non positive value");
        sharedVector = std::vector<int>(arrSize);
    }

    unsigned int getId () const {return markerId;}

    unsigned int getMarkedCount() const {return markedCount;}

    void increaseMarkedCount() {++markedCount;}

    size_t getVectorSize() const {return pSharedVector->size();}

    void setUnmarked(const unsigned int ind) {cantMark = ind;}

    unsigned int getUnmarked() const {return cantMark;}

    bool setMarkersElements(int randomNum){

        randomNum %= sharedVector.size();
        if(sharedVector[randomNum] == 0){
            Sleep(5);
            sharedVector[randomNum] = markerId;
            Sleep(5);
            ++markedCount;
            return true;
        }
        cantMark = randomNum;
        return false;
    }

    void printInfo() const{
        std::cout <<"Marker number: " << this->getId() << '\t' << "Marked elements: " << this->getMarkedCount() <<
                  "\t" << "Can't mark: " << this->getUnmarked() <<'\n';
    }

    void deleteMarkedElements(){
        for(size_t i = 0; i < pSharedVector->size(); ++i){
            if(sharedVector[i] == markerId)
                sharedVector[i] = 0;
        }
    }

    static void printVector() {
        for(size_t i = 0; i < sharedVector.size(); ++i){
            std::cout << sharedVector[i] << " ";
        }
        std::cout << '\n';
    }
};