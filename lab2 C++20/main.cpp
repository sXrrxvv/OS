#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <functional>

double findAverage(const std::vector<double>& v){
    double average = 0;
    for(auto& i : v) {
        average += (1.0)*i / v.size();
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
    return average;
}

std::pair<int,int> findMinMax(const std::vector<double>& v){
    int max = INT_MIN;
    int min = INT_MAX;
    for(auto& i : v){
        if(max < i)
            max = i;
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        if(min > i)
            min = i;
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
    }
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

void initialize(std::vector<double>& v){
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

void print(const std::vector<double>& v){
    std::cout << "result vector : \n";
    for(auto& i : v)
        std::cout << i << " ";
}

void replaceMinMaxValues(std::vector<double>& v, const std::pair<int,int>& minMax, double average){
    for(auto& i : v)
        if(i == minMax.first || i == minMax.second)
            i = average;
}

int main() {
    std::vector<double> v;
    try{
        initialize(v);
    }
    catch(std::length_error&){
        std::cerr << "ERROR : size must be a positive number\n";
        return -1;
    }
    catch(std::bad_alloc&){
        std::cerr << "ERROR : cannot allocate too much memory\n";
    }
    std::pair <int, int> minMaxVal;
    double averageVal;
    auto minMax = std::thread(minMaxThread, std::cref(v), std::ref(minMaxVal));
    minMax.join();
    auto average = std::thread(averageThread, std::cref(v), std::ref(averageVal));
    average.join();
    replaceMinMaxValues(v, minMaxVal, averageVal);
    print(v);
    return 0;
}
