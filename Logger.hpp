#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <list>
#include <utility>

using namespace std;

//per ogni esecuzione stampare:
//indirizzo + tempo iniezione + tipo di fault

//TODO: add an address-object map to identify the injected object

class Injection{
public:
    Injection(long address, const chrono::duration<long, std::ratio<1, 1000>> &elapsed, string faultType
              ) : address(address), elapsed(elapsed), faultType(std::move(faultType)) {}

    long address{};
    chrono::duration<long, std::ratio<1, 1000>> elapsed{};
    string faultType;
};

class Logger{
private:
    list<Injection> inj;
    fstream logFile;
public:

    Logger() = default;

    void addInjection(long addr, chrono::duration<long, std::ratio<1, 1000>> elapsed, string faultType){
        Injection *i = new Injection(addr, elapsed, std::move(faultType));
        inj.push_back(*i);
        cout << "Logger addr = " << hex << inj.back().address << endl;
    }
    void logOnfile(int pid){
        logFile.open("../logs/logFile_" + to_string(pid) + ".txt", ios::in | ios::out);
        for(Injection i : inj){
            string s_inj = "Address : " + to_string(i.address) + " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType + "\n";
            logFile.write(s_inj.c_str(), (int) s_inj.length());
        }
    }
    void printInj(){
        for(const Injection i : inj){
            cout << "Address : 0x" << hex << i.address << " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType << endl;
        }
    }

    virtual ~Logger() {
        if(logFile.is_open())
            logFile.close();
    }

};