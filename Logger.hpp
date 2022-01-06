#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <list>
#include <utility>
#include "Target.hpp"
#include <mutex>

#define PARALLELIZATION 1

using namespace std;

class Injection{
public:
    Injection(const chrono::duration<long, std::ratio<1, 1000>> &elapsed, string faultType, Target obj
              ) : elapsed(elapsed), faultType(std::move(faultType)), object(std::move(obj)) {}

    Target object;
    chrono::duration<long, std::ratio<1, 1000>> elapsed{};
    string faultType;
};

class Logger{
private:
    list<Injection> inj;
    fstream logFile;
#if PARALLELIZATION
    mutex m;
#endif
public:

    Logger() = default;

    void addInjection(string name, long addr, chrono::duration<long, std::ratio<1, 1000>> elapsed, string faultType){
        Injection *i;
        Target *t;
        t = new Target(std::move(name), addr, 0);

        i = new Injection(elapsed, std::move(faultType), std::move(*t));
#if PARALLELIZATION
        lock_guard<mutex> lock(m);
#endif
        inj.push_back(*i);
    }
    void logOnfile(){
        logFile.open("../logs/logFile.txt", ios::app);
        time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if(!logFile)
            logFile.open("../logs/logFile.txt", ios::in | ios::out);
        string init_str = "--------------------------\nWriting results of "  + string(ctime(&time)) + " --- Injected Object : " + inj.back().object.getName() + " ---\n--------------------------\n";
        logFile.write(init_str.c_str(), (long) init_str.size());
        for(const Injection& i : inj){
            string s_inj = "Address : " + to_string(i.object.getAddress()) + " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType + "\n";
            logFile.write(s_inj.c_str(), (int) s_inj.length());
        }
        logFile.close();
    }
    void printInj(){
        cout << endl;
        time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        string init_str = "--------------------------\nWriting results of "  + string(ctime(&time)) + " --- Injected Object : " + inj.back().object.getName() + " ---\n--------------------------\n";
        cout << init_str;
        for(const Injection& i : inj){
            string s_inj = "Address : " + to_string(i.object.getAddress()) + " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType + "\n";
            cout << s_inj;
        }
    }

    virtual ~Logger() {
        if(logFile.is_open())
            logFile.close();
    }

};