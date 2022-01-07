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
    FILE* logFile;
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
        time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        string init_str = "--------------------------\nWriting results of "  + string(ctime(&time)) + " --- Injected Object : " + inj.back().object.getName() + " ---\n--------------------------\n";
#ifdef linux
        logFile = fopen("../logs/logFile.txt", "a");
        string cmd = "flock " + to_string(fileno(logFile));
        system(cmd.c_str());
        fprintf(logFile, "%s", init_str.c_str());
        for(const Injection& i : inj){
            string s_inj = "Address : " + to_string(i.object.getAddress()) + " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType + "\n";
            fprintf(logFile, "%s", s_inj.c_str());
        }
        fclose(logFile);
#endif
#ifdef _WIN32
        HANDLE h = CreateFile(file,                // name of the write
                       GENERIC_READ,          // open for reading
                       0,                      // do not share
                       NULL,                   // default security
                       OPEN_EXISTING,             //
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);
        if(h == INVALID_HANDLE_VALUE)
        {
            printf("hFile is NULL\n");
            printf("Could not open golden\n");
    // return error
            return 4;
        }
        //TODO: create lock on file and then write --> use LockFileEx
#endif
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

};