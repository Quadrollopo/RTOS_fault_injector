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

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

#define WRITE_ON_FILE 1

using namespace std;
Logger logger;
//TODO: addresses need to be updated, only the 16th (xTickCount) is correct
vector<Target> objects = {{"xActiveTimerList2", 0x00961c1c, 40, false}, {"xActiveTimerList1", 0x00961c08, 40, false}, {"uxBlockingCycles", 0x00961260, 8, false}, {"uxPollingCycles", 0x00961264, 8, false}, {"uxControllingCycles", 0x0096125c, 8, false}, {"xBlockingIsSuspended", 0x00961258, 8, false}, {"xControllingIsSuspended", 0x00961254, 8, false}, {"xErrorOccurred", 0x0096154c, 8, false}, {"pxReadyTasksLists", 0x00961aa8, 280, false}, {"xDelayedTaskList1", 0x00961a94, 40, false}, {"xDelayedTaskList2", 0x00961b34, 40, false}, {"xPendingReadyList", 0x00961b50, 40, false}, {"xSuspendedTaskList", 0x00961b7c, 40, false}, {"xTasksWaitingTermination", 0x00961b64, 40, false}, {"uxCurrentNumberOfTasks", 0x00961b90, 8, false}, {"uxTopReadyPriority", 0x00961b98, 8, false}, {"xTickCount", 0x001b1b94, 8, false}, {"xPendedTicks", 0x00961ba0, 8, false}, {"xYieldPending", 0x00961ba4, 8, false}, {"uxSchedulerSuspended", 0x00961bb8, 8, false}, {"xNextTaskUnblockTime", 0x00961bb0, 8, false}, {"xSchedulerRunning", 0x00961b9c, 8, false}, {"uxTaskNumber", 0x00961bac, 8, false}, {"xTimerTaskHandle", 0x00961c3c, 176, true}, {"xTimerQueue", 0x00961c38, 168, true}, {"pxOverflowTimerList", 0x00961c34, 40, true}, {"pxCurrentTimerList", 0x00961c30, 40, true}, {"xBlockingTaskHandle", 0x0096126c, 176, true}, {"xControllingTaskHandle", 0x00961268, 176, true}, {"xMutex", 0x0096124c, 168, true}, {"pxCurrentTCB", 0x00961a90, 176, true}, {"pxDelayedTaskList", 0x00961b48, 40, true}, {"xIdleTaskHandle", 0x00961bb4, 176, true}};

static volatile int cnt = 0;

using namespace std;

int injector(PROCESS_INFORMATION &pi, const Target &t, long *chosenAddr, int timer_range) {
    random_device generator;
    uniform_int_distribution<int> time_dist(0, timer_range);
    this_thread::sleep_for(chrono::milliseconds(time_dist(generator)));
    cout << "Child starting injector" << endl;

    uniform_int_distribution<int> bit_distribution(0, 7);
    uint8_t byte, mask = bit_distribution(generator);
    uniform_int_distribution<long> address_distribution;

    if(!t.isPointer())
        address_distribution = uniform_int_distribution<long>(t.getAddress(), t.getAddress() + t.getSize());
    else {
        SIZE_T length_read = 0;
        uint8_t h[4];
        ReadProcessMemory(pi.hProcess, reinterpret_cast<LPVOID>(t.getAddress()), h, (size_t)4,
                          &length_read);
        if(length_read == 0)
            cerr<<"Houston we have a problem..." <<endl;
        long heapAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        address_distribution = uniform_int_distribution<long>(heapAddress, heapAddress + t.getSize());
    }

    DWORD addr = address_distribution(generator);
    SIZE_T length_read = 0;
    int err = 0;
    if(!ReadProcessMemory(pi.hProcess, (LPVOID)(addr), &byte, (SIZE_T)1,
                      &length_read)) {
        err = GetLastError();
        cout << err;
    }

    //Flipbit
    byte ^= 1 << mask;
    DWORD oldprotect;

    //VirtualProtectEx(pi.hProcess, (LPVOID)addr, 1, PAGE_EXECUTE_READWRITE, &oldprotect);
    if(!WriteProcessMemory(pi.hProcess, reinterpret_cast<LPVOID>(addr), (LPVOID)&byte, (SIZE_T)1,
                       &length_read)){
        err = GetLastError();
        cout << err;
    }
    //VirtualProtectEx(pi.hProcess, (LPVOID)addr, 1, oldprotect, &oldprotect);

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
                       GENERIC_READ,          // open for reading
                       1,                      // do not share
                       NULL,                   // default security
                       OPEN_EXISTING,             //
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template
    if(h == INVALID_HANDLE_VALUE)
    {
        printf("hFile is NULL\n");
        printf("Could not open golden\n");
// return error
        return 4;
    }
    long len = (long) GetFileSize( h, NULL);
    return len;
}

void menu(int &c, int &range, int &numInjection) {
    cout << "Type what do you want to inject:" << endl;
    for(int i = 0; i < objects.size(); i++)
        cout << i << " - " << objects[i].getName() << endl;
    do {
        cin >> c;
    }while(c >= objects.size() || c < 0);

    // Sta frase non ha molto senso, magari qualcosa in inglese non sarebbe male
    cout << "Insert a range in millisecond to randomly inject:" << endl;
    cin >> range;
    cout << "Number of injection:" << endl;
    cin >> numInjection;
}

int checkFiles(string name, int pid_rtos, long addr, chrono::duration<long, std::ratio<1, 1000>> elapsed) {
    ifstream golden_output("../files/Golden_execution.txt");
    ifstream rtos_output("../files/Falso_Dante_" + to_string(pid_rtos) + ".txt");
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

    if (s2 == 0) { // Crash
        cout << endl << "Files differ in size" << endl << "golden = " << s1 << "; falso = " << s2 << endl;
        logger.addInjection(name, addr, elapsed, "Crash");
        error = 2;
    } else {
        if(s1 == s2) {
            for (string g_line, f_line; getline(golden_output, g_line), getline(rtos_output, f_line);) {
                if (g_line != f_line) {
                    found = true;
                    error = 1;
                    cout << "The output should be" << endl << g_line << endl
                         << "instead I found" << endl << f_line << endl;
                }
            }
        }
        else
            found = true;
        if (!found) { //Masked
            cout << endl << "No differences have been found" << endl;
            logger.addInjection(name, addr, elapsed, "Masked");
        } else
            logger.addInjection(name, addr, elapsed, "SDC");
    }
    rtos_output.close();
    golden_output.close();
    return error;
}

void injectRTOS(PROCESS_INFORMATION& pi, int numInjection, int chosen, int timer_range, chrono::duration<long, ratio<1, 1000>> gtime){
    DWORD pid_rtos;
    string name = objects[chosen].getName();
    for(int iter = 0; iter < numInjection; iter++) {
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
            exit(-1);
        }

        pid_rtos = GetProcessId(pi.hProcess);
        //opzione1 ->

        long inj_addr;
        thread injection(injector, pi, objects[chosen], &inj_addr, timer_range);
        injection.join();

        chrono::duration<long, std::ratio<1, 1000>> elapsed = chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - begin);

        //hang handling
        DWORD hang;
        hang = WaitForSingleObject(pi.hProcess, gtime.count()*2);
        if (hang == WAIT_TIMEOUT) { //HANG
            LPDWORD type_error = nullptr;
            cout << endl << "Timeot expired, killing process " << endl;
            TerminateProcess(pi.hProcess, GetExitCodeProcess(pi.hProcess, type_error));

            logger.addInjection(name, inj_addr, elapsed, "Hang");
        }


        chrono::steady_clock::time_point end = chrono::steady_clock::now();

        chrono::duration<long, std::ratio<1, 1000>> rtime = chrono::duration_cast<std::chrono::milliseconds>(end - begin);
        cout << endl << "RTOS iter time : " << rtime.count() << endl;

        int err = 0;
        if(hang!=WAIT_TIMEOUT)
            err = checkFiles(objects[chosen].getName(), (unsigned int)pid_rtos, inj_addr, elapsed);

        long timeDifference = chrono::duration_cast<chrono::milliseconds>(rtime - gtime).count();

        if(abs(timeDifference) > 1100 && hang != WAIT_TIMEOUT && err!=2) //delay detected when there was not a crash or a hang
            logger.addInjection(name, inj_addr, elapsed, "Delay");

        cout << endl << "Time difference = " << to_string(timeDifference) << "[ms]" << endl;

    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void execGolden(PROCESS_INFORMATION& pi, chrono::duration<long, ratio<1, 1000>> &gtime){
    DWORD pid_golden;
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
        exit(-1);
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
}

int main(int argc, char **argv) {
    int status, status2;
    int chosen, numInjection, timer_range;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    chrono::duration<long, ratio<1, 1000>> gtime{};
    menu(chosen, timer_range, numInjection);

    //If already exist a golden execution, dont start another one
    ifstream gold("../files/Golden_execution.txt");
    if(!gold) {
        execGolden(pi, gtime);
    }
    else{
        cout << "Found another golden execution output, skipping execution..." << endl;
        ifstream time_golden("../files/Time_golden.txt");
        if (time_golden){
            long time;
            time_golden >> time;
            gtime = chrono::milliseconds(time);
            time_golden.close();
        }
        else {
            cout << "Can't open Time_golden.txt: re-exec the golden execution";
            execGolden(pi, gtime);
        }
    }
    gold.close();
    injectRTOS(pi, numInjection, chosen, timer_range, gtime);

#if WRITE_ON_FILE
    logger.logOnfile();
#else
    logger.printInj();
#endif

    return 0;
}