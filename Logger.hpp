#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <list>

using namespace std;

//per ogni esecuzione stampare:
//indirizzo + tempo iniezione + tipo di fault

class Injection{
public:
    Injection(long address, const chrono::duration<long, std::ratio<1, 1000>> &elapsed, const string &faultType
              ) : address(address), elapsed(elapsed), faultType(faultType) {}

    Injection(Injection *pInjection) {

    }

    long address;
    chrono::duration<long, std::ratio<1, 1000>> elapsed;
    string faultType;
};

class Logger{
private:
    list<Injection> inj;
    fstream logFile;
public:

    Logger() = default;

    void addInjection(long addr, chrono::duration<long, std::ratio<1, 1000>> elapsed, string faultType){
        inj.push_back(new Injection(addr, elapsed, faultType));
    }
    void logOnfile(int pid){
        logFile.open("../logs/logFile_" + to_string(pid) + ".txt", ios::in | ios::out);
        for(Injection i : inj){
            string s_inj = "Address : " + to_string(i.address) + " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType + "\n";
            logFile.write(s_inj.c_str(), (int) s_inj.length());
        }
    }
    void printInj(){
        for(Injection i : inj){
            string s_inj = "Address : " + to_string(i.address) + " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType + "\n";
            cout << s_inj;
        }
    }

    virtual ~Logger() {
        if(logFile.is_open())
            logFile.close();
    }

};