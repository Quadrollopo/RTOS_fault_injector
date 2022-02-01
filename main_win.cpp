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

#define WRITE_ON_FILE 0

using namespace std;
Logger logger;
//TODO: addresses need to be updated, only the 16th (xTickCount) is correct
vector<Target> objects = {{"pxReadyTasksLists", 0x00711aa8, 280, false}, {"xDelayedTaskList1", 0x00711a94, 40, false}, {"xPendingReadyList", 0x00711b50, 40, false}, {"xSuspendedTaskList", 0x00711b7c, 40, false}, {"uxTopReadyPriority", 0x00711b98, 8, false}, {"xTickCount", 0x00711b94, 8, false}, {"xPendedTicks", 0x00711ba0, 8, false}, {"uxSchedulerSuspended", 0x00711bb8, 8, false}, {"xNextTaskUnblockTime", 0x00711bb0, 8, false}, {"xSchedulerRunning", 0x00711b9c, 8, false}, {"uxTaskNumber", 0x00711bac, 8, false}, {"pxCurrentTCB", 0x00711a90, 176, true}, {"pxDelayedTaskList", 0x00711b48, 40, true}, {"xIdleTaskHandle", 0x00711bb4, 176, true}, {"xActiveTimerList1", 0x00711c08, 40, false}, {"xTimerTaskHandle", 0x00b21c3c, 176, true}, {"xTimerQueue", 0x00711c38, 168, true}, {"pxOverflowTimerList", 0x00711c34, 40, true}, {"pxCurrentTimerList", 0x00711c30, 40, true}, {"xQueue", 0x00711230, 168, true}, {"xTimer", 0x007118e0, 88, true}};

using namespace std;

void getAddress(PROCESS_INFORMATION &pi, uint8_t *h, LPVOID address){
    if(!ReadProcessMemory(pi.hProcess, address, h, (size_t)4,
                          nullptr)) {
        cerr << "Houston we have a problem... " << GetLastError() << endl;
        exit(-1);
    }
}

long getRandomAddressInRange(long a1, long a2){
    random_device generator;
    uniform_int_distribution<long> address_distribution = uniform_int_distribution<long>(a1, a2);
    return (long) address_distribution(generator);
}

DWORD inject_queue(PROCESS_INFORMATION &pi, DWORD address, Target& t){
    uint8_t h[4];
    if(t.getName() == "xQueue")
        getAddress(pi, h, reinterpret_cast<LPVOID>(objects[19].getAddress()));
    else
        getAddress(pi, h, reinterpret_cast<LPVOID>(objects[16].getAddress()));
    long queue = (long) (h[0] + h[1]*256 + h[2]*256*256);

    if (address >= queue && address < queue + 8) {
        cout << "Injecting " << t.getName() << "->pcHead\n";
        t.setSubName(t.getName() + "->pcHead");
        getAddress(pi, h, reinterpret_cast<LPVOID>(queue));
        long newAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        return getRandomAddressInRange(newAddress, newAddress + 50);
    }
    else if(address >= queue + 8 && address < queue + 16) {
        cout << "Injecting " << t.getName() << "pcWriteTo\n";
        t.setSubName(t.getName() + "->pcWriteTo");
        getAddress(pi, h, reinterpret_cast<LPVOID>(queue + 8));
        long newAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        return getRandomAddressInRange(newAddress, newAddress + 50);
    }
    else if (address >= queue + 120 && address < queue + 128) {
        cout << "Injecting " << t.getName() << "->uxMessagesWaiting\n";
        t.setSubName(t.getName() + "->uxMessagesWaiting");
    }
    else if(address >= queue + 128 && address < queue + 136) {
        cout << "Injecting " << t.getName() << "->uxLength\n";
        t.setSubName(t.getName() + "->uxLength");
    }
    else if(address >= queue + 136 && address < queue + 144) {
        cout << "Injecting " << t.getName() << "->uxItemSize\n";
        t.setSubName(t.getName() + "->uxItemSize");
    }
    return address;
}

DWORD inject_TCB(PROCESS_INFORMATION &pi, DWORD address, Target& t){
    uint8_t h[4];
    if(t.getName() == "pxCurrentTCB")
        getAddress(pi, h, reinterpret_cast<LPVOID>(objects[18].getAddress()));
    else
        getAddress(pi, h, reinterpret_cast<LPVOID>(objects[14].getAddress()));

    long tcb = (long) (h[0] + h[1]*256 + h[2]*256*256);
    if (address >= tcb && address < tcb + 8) {
        cout << "Injecting " << t.getName() << "->pxTopOfStack\n";
        t.setSubName(t.getName() + "->pxTopOfStack");
        return address;
    }
    else if(address >= tcb + 88 && address < tcb + 96) {
        cout << "Injecting " << t.getName() << "uxPriority\n";
        t.setSubName(t.getName() + "->uxPriority");
    }
    else if (address >= tcb + 96 && address < tcb + 104) {
        t.setSubName(t.getName() + "->pxStack");
        cout << "Injecting " << t.getName() << "->pxStack\n";
        getAddress(pi, h, reinterpret_cast<LPVOID>(tcb + 96));
        long newAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        return getRandomAddressInRange(newAddress, newAddress + 70);
    }
    else if(address >= tcb + 104 && address < tcb + 112) {
        cout << "Injecting " << t.getName() << "->pcTaskName\n";
        t.setSubName(t.getName() + "->pcTaskName");
    }
    else if(address >= tcb + 116 && address < tcb + 124) {
        cout << "Injecting " << t.getName() << "->uxTCBNumber\n";
        t.setSubName(t.getName() + "->uxTCBNumber");
    }
    else if(address >= tcb + 104 && address < tcb + 112) {
        cout << "Injecting " << t.getName() << "->uxTaskNumber\n";
        t.setSubName(t.getName() + "->uxTaskNumber");
    }
    else if(address >= tcb + 112 && address < tcb + 120) {
        cout << "Injecting " << t.getName() << "->uxBasePriority\n";
        t.setSubName(t.getName() + "->uxBasePriority");
    }
    else if(address >= tcb + 120 && address < tcb + 128) {
        cout << "Injecting " << t.getName() << "->callback\n";
        t.setSubName(t.getName() + ">callback");
    }
    else if(address >= tcb + 128 && address < tcb + 136) {
        cout << "Injecting " << t.getName() << "->ulRunTimeCounter\n";
        t.setSubName(t.getName() + "->ulRunTimeCounter");
    }
    return address;
}

DWORD inject_timer(PROCESS_INFORMATION &pi, DWORD address, Target& t){
    uint8_t h[4];
    getAddress(pi, h, reinterpret_cast<LPVOID>(objects[20].getAddress()));
    long timer = (long) (h[0] + h[1]*256 + h[2]*256*256);
    if (address >= timer && address < timer + 8) {
        cout << "Injecting xTimer name\n";
        t.setSubName("xTimer->name");
        getAddress(pi, h, reinterpret_cast<LPVOID>(timer));
        return (long) (h[0] + h[1]*256 + h[2]*256*256);
    }
    else if(address >= timer + 8 && address < timer + 48) {
        cout << "Injecting xTimer->xTimerListItem\n";
        t.setSubName("xTimer->xTimerListItem");
        if(address < timer + 16) {
            t.setSubName("xTimer->xTimerListItem->xItemValue");
            cout << "Injecting xItemValue\n";
        }
        else if (address < timer +  24) {
            t.setSubName("xTimer->xTimerListItem->pxNext");
            cout << "Injecting pxNext\n";
        }
        else if(address > timer + 40) {
            t.setSubName("xTimer->xTimerListItem->pvContainer");
            cout << "Injecting pvContainer\n";
            getAddress(pi, h, reinterpret_cast<LPVOID>(timer + 40));
            long newAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
            return getRandomAddressInRange(newAddress, newAddress + 40);
        }
        return address;
    }
    else if (address >= timer + 48 && address < timer + 56) {
        t.setSubName("xTimer->xTimerPeriodInTicks");
        cout << "Injecting xTimer->xTimerPeriodInTicks\n";
    }
    else if (address >= timer + 64 && address < timer + 72) {
        t.setSubName("xTimer->callback");
        cout << "Injecting xTimer callback function\n";
    }
    return address;
}

long injector(PROCESS_INFORMATION &pi, Target &t, long *chosenAddr, int timer_range) {
    random_device generator;
    uniform_int_distribution<int> time_dist(100, timer_range);
    this_thread::sleep_for(chrono::milliseconds(time_dist(generator)));
    cout << "Child starting injector" << endl;

    uniform_int_distribution<int> bit_distribution(0, 7);
    uint8_t byte, mask = bit_distribution(generator);
    uniform_int_distribution<long> address_distribution;

    if(!t.isPointer())
        address_distribution = uniform_int_distribution<long>(t.getAddress(), t.getAddress() + t.getSize());
    else {
        uint8_t h[4];
        if(!ReadProcessMemory(pi.hProcess, reinterpret_cast<LPVOID>(t.getAddress()), &h, (size_t)4,
                              nullptr)) {
            cerr << "Houston we have a problem... " << GetLastError() << endl;
            return -1;
        }
        long heapAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        address_distribution = uniform_int_distribution<long>(heapAddress, heapAddress + t.getSize());
    }

    DWORD addr = address_distribution(generator);
    SIZE_T length_read = 0;
    DWORD err = 0;
    if(!ReadProcessMemory(pi.hProcess, (LPVOID)(addr), &byte, (SIZE_T)1,
                      &length_read)) {
        cerr << "Houston we have a problem... " << GetLastError() << endl;
        return -1;
    }

    if(t.getName() == "xTimer")
        addr = inject_timer(pi, addr, t);
    else if(t.getName() == "pxCurrentTCB" || t.getName() == "xTimerTaskHandle")
        addr = inject_TCB(pi, addr, t);
    else if(t.getName() == "xQueue" || t.getName() == "xTimerQueue")
        addr = inject_queue(pi, addr, t);
    if(!ReadProcessMemory(pi.hProcess, reinterpret_cast<LPVOID>(t.getAddress()), &byte, (size_t)1,
                          nullptr)) {
        cerr << "Houston we have a problem... " << GetLastError() << endl;
        return -1;
    }

    //Flipbit
    byte ^= 1 << mask;
    if(!WriteProcessMemory(pi.hProcess, reinterpret_cast<LPVOID>(addr), (LPVOID)&byte, (SIZE_T)1,
                       &length_read)){
        err = GetLastError();
        cout << err;
    }

    cout << "Modified " <<
         // put red color for the bit flipped
         bitset<8>(byte).to_string().insert(8 - mask, COLOR_RESET).insert(7 - mask, COLOR_RED)
         << " at 0x" << hex << addr << endl;
    *chosenAddr = (long) addr;
    cout << hex << *chosenAddr << endl;
    return 0;
}

long getFileLen(const char* file) {
    HANDLE h = CreateFile(file,                // name of the write
                       GENERIC_READ,          // open for reading
                       1,                      // do not share
                          nullptr,                   // default security
                       OPEN_EXISTING,             //
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                          nullptr);                  // no attr. template
    if(h == INVALID_HANDLE_VALUE)
    {
        printf("hFile is NULL\n");
        printf("Could not open golden\n");
// return error
        return 4;
    }
    long len = (long) GetFileSize( h, nullptr);
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

int checkFiles(Target &t, int pid_rtos, chrono::duration<long, std::ratio<1, 1000>> elapsed) {
    ifstream golden_output("../files/Golden_execution.txt");
    string fileName = "../files/Falso_Dante_" + to_string(pid_rtos) + ".txt";
    ifstream rtos_output(fileName);
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
    s2 = getFileLen(fileName.c_str());

    if (s2 == 0) { // Crash
        cout << endl << "Files differ in size" << endl << "golden = " << s1 << "; falso = " << s2 << endl;
        logger.addInjection(t, elapsed, "Crash");
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
            logger.addInjection(t, elapsed, "Masked");
        } else
            logger.addInjection(t, elapsed, "SDC");
    }
    rtos_output.close();
    remove(fileName.c_str());
    golden_output.close();
    return error;
}

void injectRTOS(PROCESS_INFORMATION& pi, int iter, int chosen, int timer_range, chrono::duration<long, ratio<1, 1000>> gtime){
    DWORD pid_rtos;
    string name = objects[chosen].getName();

    cout << endl << "Itering injections, iteration : " << iter << endl;
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    STARTUPINFO si = {sizeof(si)};

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcess(nullptr,   // No module name (use command line)
                       (char *)"./RTOSDemo.exe",        // Command line
                       nullptr,           // Process handle not inheritable
                       nullptr,           // Thread handle not inheritable
                       FALSE,          // Set handle inheritance to FALSE
                       0,              // No creation flags
                       nullptr,           // Use parent's environment block
                       nullptr,           // Use parent's starting directory
                       &si,            // Pointer to STARTUPINFO structure
                       &pi)           // Pointer to PROCESS_INFORMATION structure
            ) {
        printf("CreateProcess failed (%lu).\n", GetLastError());
        exit(-1);
    }

    pid_rtos = GetProcessId(pi.hProcess);
    //opzione1 ->

    long inj_addr;
    auto *t = new Target(objects[chosen]);
    thread injection(injector, pi, objects[chosen], &inj_addr, timer_range);
    injection.join();

    chrono::duration<long, std::ratio<1, 1000>> elapsed = chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - begin);

    //hang handling
    DWORD hang;
    hang = WaitForSingleObject(pi.hProcess, gtime.count()*2);
    if (hang == WAIT_TIMEOUT) { //HANG
        cout << endl << "Timeot expired, killing process " << endl;
        TerminateProcess(pi.hProcess, 0);

        logger.addInjection(reinterpret_cast<Target &>(*t), elapsed, "Hang");
    }


    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    chrono::duration<long, std::ratio<1, 1000>> rtime = chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    cout << endl << "RTOS iter time : " << rtime.count() << endl;

    int err = 0;
    if(hang!=WAIT_TIMEOUT)
        err = checkFiles(*t, (int)pid_rtos, elapsed);

    long timeDifference = (rtime - gtime).count();

    if(abs(timeDifference) > 1100 && hang != WAIT_TIMEOUT && err!=2) //delay detected when there was not a crash or a hang
        logger.addInjection(reinterpret_cast<Target &>(*t), elapsed, "Delay");

    cout << endl << "Time difference = " << to_string(timeDifference) << "[ms]" << endl;


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
    if (!CreateProcess(nullptr,   // No module name (use command line)
                       (char *)"./RTOSDemo.exe",        // Command line
                       nullptr,           // Process handle not inheritable
                       nullptr,           // Thread handle not inheritable
                       FALSE,          // Set handle inheritance to FALSE
                       0,              // No creation flags
                       nullptr,           // Use parent's environment block
                       nullptr,           // Use parent's starting directory
                       &si,            // Pointer to STARTUPINFO structure
                       &pi)           // Pointer to PROCESS_INFORMATION structure
            ) {
        printf("CreateProcess failed (%lu).\n", GetLastError());
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

void fillAddresses(){
    ifstream file("rtos.output");
    if (!file.is_open()){
        cout << "Cannot open rtos.output (addresses file)\n";
        exit(-1);
    }
    for(Target& obj : objects){
        char n[100];
        long addr;
        file.getline(n, 100);
        sscanf(n, "%lx", &addr);
        obj.setAddress(addr);
    }
    file.close();
    remove("rtos.output");
}

int main() {
    int chosen, numInjection, timer_range;
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
    fillAddresses();
#if PARALLELIZATION
    vector<thread> p(numInjection);
    for (int iter = 0; iter < numInjection; iter++) {
        p[iter] = thread(injectRTOS, pi, iter, chosen, timer_range, gtime);
    }
    for (int i = 0; i < numInjection; i++) {
        p[i].join();
    }
#else
    for (int iter = 0; iter < numInjection; iter++)
        injectRTOS(pi, iter, chosen, timer_range, gtime);
#endif

#if WRITE_ON_FILE
    logger.logOnfile();
#else
    logger.printInj();
#endif

    return 0;
}