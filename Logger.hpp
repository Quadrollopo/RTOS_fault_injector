#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <list>
#include <utility>
#include "Target.hpp"

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
public:

    Logger() = default;

    void addInjection(string name, long addr, chrono::duration<long, std::ratio<1, 1000>> elapsed, string faultType){
        Injection *i;
        Target *t;
        t = new Target(std::move(name), addr, 0);

        i = new Injection(elapsed, std::move(faultType), std::move(*t));
        inj.push_back(*i);
    }
    void logOnfile(int pid){
        logFile.open("../logs/logFile.txt"  /*+ to_string(pid) +*/ ".txt", ios::app);
        time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if(!logFile)
            logFile.open("../logs/logFile.txt" /* + to_string(pid) +*/  ".txt", ios::in | ios::out);
        string init_str = "--------------------------\nWriting results of "  + string(ctime(&time)) + " --- Injected Object : " + inj.back().object.getName() + "---\n--------------------------\n";
        logFile.write(init_str.c_str(), init_str.size());
        for(const Injection& i : inj){
            string s_inj = "Address : " + to_string(i.object.getAddress()) + " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType + "\n";
            logFile.write(s_inj.c_str(), (int) s_inj.length());
        }
    }
    void printInj(){
        cout << endl;
        cout << endl;
        for(const Injection& i : inj){
            cout << "Address : 0x" << hex << i.object.getAddress() << " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType << " --- Injected Object : " + i.object.getName() + "\n";
        }
    }

    virtual ~Logger() {
        if(logFile.is_open())
            logFile.close();
    }

};