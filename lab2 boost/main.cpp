#include <iostream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/container/vector.hpp>
#include <boost/compressed_pair.hpp>
#include <boost/core/ref.hpp>

static const double dZero = 0.0;
static const int sleepValueAverage = 7;
static const int sleepValueMinMax = 12;

void sleepFor(int milliseconds){
    boost::this_thread::sleep_for(boost::chrono::milliseconds(milliseconds));
}

double findAverage(const boost::container::vector<double>& v){
    double average = dZero;
    boost::for_each(v, [&](double a) {return average += a/v.size();});
    sleepFor(sleepValueAverage);
    return average;
}

boost::compressed_pair<int,int> findMinMax(const boost::container::vector<double>& v){
    auto max = DBL_MIN;
    auto min = DBL_MAX;
    boost::for_each(v, [&](double a) {
        if(a > max)
            max = a;
        sleepFor(sleepValueMinMax);
        if(a < min)
            min = a;
        sleepFor(sleepValueMinMax);
    });
    boost::compressed_pair<int,int> minMaxPair(min,max);
    return  minMaxPair;
}

void minMaxThread(const boost::container::vector<double>& v, boost::compressed_pair<int, int>& minMax){
    minMax = findMinMax(v);
    std::cout << "minimal value = " << minMax.first() << "\nmaximal value = " << minMax.second() << '\n';
}

void averageThread(const boost::container::vector<double>& v, double& averageVal){
    averageVal = findAverage(v);
    std::cout << "average value = " << averageVal << '\n';
}

void initializeStartVector(boost::container::vector<double>& v){
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

void printVector(const boost::container::vector<double>& v){
    std::cout << "result vector : \n";
    boost::for_each(v, [](double a) {std::cout << a << " ";});
}

void replaceMinMaxValues(boost::container::vector<double>& v, const boost::compressed_pair<int,int>& minMax, double average){
    for(auto& i : v)
        if(i == minMax.first() || i == minMax.second())
            i = average;
}

void createAndJoinThreads(const boost::container::vector<double>& v, double& averageVal,
                          boost::compressed_pair<int, int>& minMaxVal){
    auto minMax = boost::thread(minMaxThread, boost::cref(v), boost::ref(minMaxVal));
    minMax.join();
    auto average = boost::thread(averageThread, boost::cref(v), boost::ref(averageVal));
    average.join();
}

int main() {
    boost::container::vector<double> v;
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
    boost::compressed_pair<int, int> minMaxVal;
    double averageVal;
    createAndJoinThreads(v, averageVal, minMaxVal);
    replaceMinMaxValues(v, minMaxVal, averageVal);
    printVector(v);
    return 0;
}
