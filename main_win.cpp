#include <iostream>
#include <string>
#include <bitset>
#include <random>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <limits>
#include "Logger.hpp"
#include <windows.h>
#include <tchar.h>
#include <processthreadsapi.h>
#include <memoryapi.h>
#include <fileapi.h>
#include <signal.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

using namespace std;
Logger logger;

static volatile int cnt = 0;

using namespace std;

int injector(PROCESS_INFORMATION pi, long startAddr, long endAddr, long *chosenAddr) {
    this_thread::sleep_for(chrono::milliseconds(rand() % 9000));
    cout << "Child starting injector" << endl;
    random_device generator;
    uniform_int_distribution<int> bit_distribution(0, 7);
    uint8_t byte, mask = bit_distribution(generator);
    uniform_int_distribution<long> address_distribution(startAddr, endAddr);

    long addr = address_distribution(generator);
    SIZE_T* length_read = nullptr;
    ReadProcessMemory(pi.hProcess, reinterpret_cast<LPCVOID>(addr), &byte, (SIZE_T)1,
                      length_read);


    //Flipbit
    byte ^= 1 << mask;
    WriteProcessMemory(pi.hProcess, reinterpret_cast<LPVOID>(addr), reinterpret_cast<LPCVOID>(byte), (SIZE_T)1,
                       reinterpret_cast<SIZE_T *>(length_read));

    cout << "Modified " <<
         // put red color for the bit flipped
         bitset<8>(byte).to_string().insert(8 - mask, COLOR_RESET).insert(7 - mask, COLOR_RED)
         << " at 0x" << hex << addr << endl;
    *chosenAddr = addr;
    cout << hex << *chosenAddr << endl;
    return 0;
}

long getFileLen(const char* file) {
    HANDLE h = CreateFile(file,                // name of the write
                       GENERIC_READ,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       OPEN_EXISTING,             //
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template
    long len = (long) GetFileSize( h, NULL);
    return len;
}

int checkFiles(unsigned int pid_rtos, long addr, chrono::duration<long, std::ratio<1, 1000>> elapsed) {
    ifstream golden_output("../files/Golden_execution.txt");
    ifstream rtos_output( "../files/Falso_Dante_" + to_string(pid_rtos) + ".txt");
    bool found = false;
    long s1, s2;
    int error = 0; // 0 --> Masked, 1 --> SDC, 2 --> Crash

    if (!golden_output.is_open()) {
        cout << pid_rtos << endl;
        cout << "Can't open the golden execution output" << endl;
        return -1;
    }
    if (!rtos_output.is_open()) {
        cout << pid_rtos << endl;
        cout << "Can't open the rtos execution output" << endl;
        return -1;
    }
    s1 = getFileLen("../files/Golden_execution.txt");
    s2 = getFileLen(("../files/Falso_Dante_" + to_string(pid_rtos) + ".txt").c_str());
    if(s2==0) { // Crash
        cout << endl << "Files differ in size" << endl << "golden = " << s1 << "; falso = " << s2 << endl;
        logger.addInjection(addr, elapsed, "Crash");
        error = 2;
    }
    else {
        for (string g_line, f_line; getline(golden_output, g_line), getline(rtos_output, f_line);) {
            if (g_line != f_line) {
                found = true;
                error = 1;
                cout << "The output should be" << endl << g_line << endl
                     << "instead I found" << endl << f_line << endl;
            }
        }
        if (!found) { //Masked
            cout << endl << "No differences have been found" << endl;
            logger.addInjection(addr, elapsed, "Masked");
        }
        else
            logger.addInjection(addr, elapsed, "SDC");
    }
    rtos_output.close();
    golden_output.close();
    return error;
}

int main(int argc, char **argv){
    int status, status2;
    int chosen;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD pid_golden, pid_rtos;
    chrono::duration<long, ratio<1, 1000>> gtime{};
    if (argc < 2)
        chosen = 1;
    else
        chosen= (int) strtol(argv[1], nullptr, 10);
    ifstream gold("../files/Golden_execution.txt");
    if(!gold) {
        chrono::steady_clock::time_point bgold = chrono::steady_clock::now();
        STARTUPINFO si = {sizeof(si)};

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        // Start the child process.
        if (!CreateProcess(NULL,   // No module name (use command line)
                           "./RTOSDemo.exe",        // Command line
                           NULL,           // Process handle not inheritable
                           NULL,           // Thread handle not inheritable
                           FALSE,          // Set handle inheritance to FALSE
                           0,              // No creation flags
                           NULL,           // Use parent's environment block
                           NULL,           // Use parent's starting directory
                           &si,            // Pointer to STARTUPINFO structure
                           &pi)           // Pointer to PROCESS_INFORMATION structure
                ) {
            printf("CreateProcess failed (%d).\n", GetLastError());
            return -1;
        }
        pid_golden = GetProcessId(pi.hProcess);
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);
        gtime = chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now() - bgold);
        cout << endl << "Golden time : " << gtime.count() << endl;
        ofstream time_golden("..\\files\\Time_golden.txt");
        if (time_golden)
            time_golden << gtime.count();
        else
            cout << "Can't create Time_golden.txt";
        time_golden.close();
        cnt = 0;

        const string cmd = "rename ..\\files\\Falso_Dante_" + to_string(pid_golden) + ".txt Golden_execution.txt";
        system((const char *) cmd.c_str());


        // Close process and thread handles.

    }else{
        cout << "Found another golden execution output, skipping execution..." << endl;
        ifstream time_golden("../files/Time_golden.txt");
        if (time_golden){
            long time;
            time_golden >> time;
            gtime = chrono::milliseconds(time);
            time_golden.close();
        }
        else
            cout << "Can't open Time_golden.txt";
    }
    gold.close();
    int iter = 0;
    while (iter < 4) {
        cout << endl << "Itering injections, iteration : " << iter << endl;
        chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        STARTUPINFO si = {sizeof(si)};

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        if (!CreateProcess(NULL,   // No module name (use command line)
                           "./RTOSDemo.exe",        // Command line
                           NULL,           // Process handle not inheritable
                           NULL,           // Thread handle not inheritable
                           FALSE,          // Set handle inheritance to FALSE
                           0,              // No creation flags
                           NULL,           // Use parent's environment block
                           NULL,           // Use parent's starting directory
                           &si,            // Pointer to STARTUPINFO structure
                           &pi)           // Pointer to PROCESS_INFORMATION structure
                ) {
            printf("CreateProcess failed (%d).\n", GetLastError());
            return -1;
        }
        long addr1, addr2;
        pid_rtos = GetProcessId(pi.hProcess);
        //opzione1 ->

        switch(chosen){
            case 0:
                addr1 = 0x431208; //DELAY!!
                addr2 = 0x431208 + 7;
                break;
            case 1:
                addr1 = 0x431720; //xStaticQueue
                addr2 = 0x431720 + 168;
                break;
            case 2:
                addr1 = 0x434380;//xTimerBuffer
                addr2 = 0x434380 + 88;
                break;
            case 3:
                addr1 = 0x4344a0; //xStack1
                addr2 = 0x4344a0 + 200;
                break;
            case 4:
                addr1 = 0x433d40; //xStack2
                addr2 = 0x433d40 + 200;
                break;
            case 5:
                addr1 = 0x431ac0; //xActiveTimerList2
                addr2 = 0x431af0;
                break;
            case 6:
                addr1 = 0x431b00; //xTimerTaskHandle - a lot of hangs and crashes
                addr2 = 0x431b10;
                break;
            default:
                break;
        }

        long inj_addr;
        thread injection(injector, pi, 0x00f71a90, 0x00f71b90, &inj_addr);
        injection.join();

        chrono::duration<long, std::ratio<1, 1000>> elapsed = chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - begin);

        //hang handling
        struct timeval timeout = {40,0};
        DWORD hang;
        hang = WaitForSingleObject(pi.hProcess, gtime.count()*2);
        if (hang == WAIT_TIMEOUT) { //HANG
            LPDWORD type_error = nullptr;
            cout << endl << "Timeot expired, killing process " << endl;
            TerminateProcess(pi.hProcess, GetExitCodeProcess(pi.hProcess, type_error));

            logger.addInjection(inj_addr, elapsed, "Hang");
        }


        chrono::steady_clock::time_point end = chrono::steady_clock::now();

        chrono::duration<long, std::ratio<1, 1000>> rtime = chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        cout << endl << "RTOS iter time : " << rtime.count() << endl;

        int err = 0;
        if(hang!=WAIT_TIMEOUT)
            err = checkFiles((unsigned int)pid_rtos, inj_addr, elapsed);

        long timeDifference = chrono::duration_cast<chrono::milliseconds>(rtime - gtime).count();

        if(abs(timeDifference) > 800 && hang != WAIT_TIMEOUT && err!=2) //delay detected when there was not a crash or a hang
            logger.addInjection(inj_addr, elapsed, "Delay");

        cout << endl << "Time difference = " << to_string(timeDifference) << "[ms]" << endl;

        iter++;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}