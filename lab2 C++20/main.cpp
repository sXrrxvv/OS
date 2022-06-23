#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <algorithm>
#include <cfloat>

static const double dZero = 0.0;
static const int sleepValueAverage = 7;
static const int sleepValueMinMax = 12;

void sleepFor(int milliseconds){
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double findAverage(const std::vector<double>& v){
    double average = dZero;
    std::for_each(v.begin(), v.end(), [&](double a) {return average += a/v.size();});
    sleepFor(sleepValueAverage);
    return average;
}

std::pair<int,int> findMinMax(const std::vector<double>& v){
    auto max = DBL_MIN;
    auto min = DBL_MAX;
    std::for_each(v.begin(), v.end(), [&](double a) {
        if(a > max)
            max = a;
        sleepFor(sleepValueMinMax);
        if(a < min)
            min = a;
        sleepFor(sleepValueMinMax);
    });
    return std::make_pair(min, max);
}

void minMaxThread(const std::vector<double>& v, std::pair<int, int>& minMax){
    minMax = findMinMax(v);
    std::cout << "minimal value = " << minMax.first << "\nmaximal value = " << minMax.second << '\n';
}

void averageThread(const std::vector<double>& v, double& averageVal){
    averageVal = findAverage(v);
    std::cout << "average value = " << averageVal << '\n';
}

void initializeStartVector(std::vector<double>& v){
    std::cout << "enter array size and elements\n";
    size_t size;
    std::cin >> size;
    if(!size){
        throw std::length_error("non positive length");
    }
    v.resize(size);
    for(auto& i : v){
        int tmp;
        std::cin >> tmp;
        i = tmp;
    }
    v.shrink_to_fit();
}

void printVector(const std::vector<double>& v){
    std::cout << "result vector : \n";
    std::for_each(v.begin(), v.end(), [](double a) {std::cout << a << " ";});
}

void replaceMinMaxValues(std::vector<double>& v, const std::pair<int,int>& minMax, double average){
    for(auto& i : v)
        if(i == minMax.first || i == minMax.second)
            i = average;
}

void createAndJoinThreads(const std::vector<double>& v, double& averageVal,
                          std::pair<int, int>& minMaxVal){
    auto minMax = std::thread(minMaxThread, std::cref(v), std::ref(minMaxVal));
    minMax.join();
    auto average = std::thread(averageThread, std::cref(v), std::ref(averageVal));
    average.join();
}

int main() {
    std::vector<double> v;
    try{
        initializeStartVector(v);
    }
    catch(std::length_error&){
        std::cerr << "ERROR : size must be a positive number\n";
        v.clear();
        return -1;
    }
    catch(std::bad_alloc&){
        std::cerr << "ERROR : can not allocate too much memory";
        v.clear();
        return -1;
    }
    std::pair<int, int> minMaxVal;
    double averageVal;
    createAndJoinThreads(v, averageVal, minMaxVal);
    replaceMinMaxValues(v, minMaxVal, averageVal);
    printVector(v);
    return 0;
}
