#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <list>
#include <utility>
#include "Target.hpp"
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#include <memoryapi.h>
#include <fileapi.h>
#endif

#define PARALLELIZATION 0

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

#ifdef _WIN32
static void writeLog(const string& log, HANDLE& h){
    bool bErrorFlag = WriteFile(
            h,           // open file handle
            log.c_str(),      // start of data to write
            log.length(),  // number of bytes to write
            NULL, // number of bytes that were written
            NULL);            // no overlapped structure

    if (FALSE == bErrorFlag)
    {
        printf("Terminal failure: Unable to write to file.\n");
    }
}

#endif

    void logOnfile() {
        time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        string init_str =
                "--------------------------\nWriting results of " + string(ctime(&time)) + " --- Injected Object : " +
                inj.back().object.getName() + " ---\n--------------------------\n";
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
        HANDLE h;
        long err;
        do {
            h = CreateFile("../logs/logFile.txt",                // name of the write
                              FILE_APPEND_DATA,          // append
                              0,                      // do not share
                              NULL,                   // default security
                              OPEN_ALWAYS,             //
                              FILE_ATTRIBUTE_NORMAL,  // normal file
                              NULL);
            err = GetLastError();
            cout << "Creating log file\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }while(err == 32);

        if (h == INVALID_HANDLE_VALUE) {
            printf("hFile is NULL\n");
            printf("Could not open log\n");

            // return error
            exit(-1);
        }

        writeLog(init_str, h);
        for (const Injection &i : inj) {
            string s_inj =
                    "Address : " + to_string(i.object.getAddress()) + " --- Time : " + to_string(i.elapsed.count()) +
                    " --- Fault type : " + i.faultType + "\n";
            writeLog(s_inj, h);
        }
            CloseHandle(h);
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