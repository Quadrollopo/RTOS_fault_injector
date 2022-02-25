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
#include "config.h"

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"


using namespace std;
Logger logger;
vector<Target> objects = {{"pxReadyTasksLists", 0x00711aa8, 280, false}, {"xDelayedTaskList1", 0x00711a94, 40, false}, {"xPendingReadyList", 0x00711b50, 40, false}, {"xSuspendedTaskList", 0x00711b7c, 40, false}, {"uxTopReadyPriority", 0x00711b98, 8, false}, {"xTickCount", 0x00711b94, 8, false}, {"xPendedTicks", 0x00711ba0, 8, false}, {"uxSchedulerSuspended", 0x00711bb8, 8, false}, {"xNextTaskUnblockTime", 0x00711bb0, 8, false}, {"xSchedulerRunning", 0x00711b9c, 8, false}, {"uxTaskNumber", 0x00711bac, 8, false}, {"pxCurrentTCB", 0x00711a90, 176, true}, {"pxDelayedTaskList", 0x00711b48, 40, true}, {"xIdleTaskHandle", 0x00711bb4, 176, true}, {"xActiveTimerList1", 0x00711c08, 40, false}, {"xTimerTaskHandle", 0x00b21c3c, 176, true}, {"xTimerQueue", 0x00711c38, 168, true}, {"pxOverflowTimerList", 0x00711c34, 40, true}, {"pxCurrentTimerList", 0x00711c30, 40, true}, {"xQueue", 0x00711230, 168, true}, {"xTimer", 0x007118e0, 88, true}};
static volatile int cnt = 0;

using namespace std;

void getAddress(PROCESS_INFORMATION &pi, uint8_t *h, LPVOID address){
    //read the content of "address" and returns it in the h parameter
    if(!ReadProcessMemory(pi.hProcess, address, h, (size_t)4,
                          nullptr)) {
        cerr << "Error Reading memory 1: " << GetLastError() << endl;
        exit(-1);
    }
}

long getRandomAddressInRange(long a1, long a2){
    random_device generator;
    uniform_int_distribution<long> address_distribution = uniform_int_distribution<long>(a1, a2);
    return (long) address_distribution(generator);
}

DWORD inject_list_item(PROCESS_INFORMATION &pi, DWORD address, Target& t, DWORD baseItem){
    //it performs a specific injection for a list_item object, specifying which parameter of the struct has been injected
    if (address >= baseItem && address < baseItem + 8) {
        cout << "Injecting " << t.getSubName() << "->xItemValue\n";
        t.setSubName(t.getSubName() + "->xItemValue");
    }
    else if(address >= baseItem + 8 && address < baseItem + 16) {
        cout << "Injecting " << t.getSubName() << "->pxNext\n";
        t.setSubName(t.getSubName() + "->pxNext");
    }
    else if(address >= baseItem + 16 && address < baseItem + 24) {
        cout << "Injecting " << t.getSubName() << "->pxPrevious\n";
        t.setSubName(t.getSubName() + "->pxPrevious");
    }
    else if(address >= baseItem + 24 && address < baseItem + 32) {
        cout << "Injecting " << t.getSubName() << "->pvOwner\n";
        t.setSubName(t.getSubName() + "->pvOwner");
    }
    else if(address >= baseItem + 32 && address < baseItem + 40) {
        cout << "Injecting " << t.getSubName() << "->pvContainer\n";
        t.setSubName(t.getSubName() + "->pvContainer");
    }
    return address;
}

DWORD inject_list(PROCESS_INFORMATION &pi, DWORD address, Target& t){
    //specific injection for a list object, it can move the file pointer to move within the struct.
    long list;
    if(t.isPointer()) {
        uint8_t h[4];
        getAddress(pi, h, reinterpret_cast<LPVOID>(t.getAddress()));
        list = (long) (h[0] + h[1] * 256 + h[2] * 256 * 256);
    }
    else
        list = t.getAddress();
    if (address >= list && address < list + 8) {
        cout << "Injecting " << t.getSubName() << "->uxNumberOfItems\n";
        t.setSubName(t.getSubName() + (t.getSubName().empty() ? "" : "->") + "uxNumberOfItems");
    }
    else if(address >= list + 8 && address < list + 16) {
        uint8_t h[4];
        cout << "Injecting " << t.getSubName() << "->pxNext\n";
        t.setSubName(t.getSubName() + (t.getSubName().empty() ? "" : "->") + "pxNext");
        getAddress(pi, h, reinterpret_cast<LPVOID>(list + 8));
        long baseItem = (long) (h[0] + h[1] * 256 + h[2] * 256 * 256);
        return inject_list_item(pi, getRandomAddressInRange(baseItem, baseItem + 40), t, baseItem);
    }
    else if (address >= list + 16 && address < list + 40){
        cout << "Injecting " << t.getSubName() << "->xListEnd\n";
        t.setSubName(t.getSubName() + (t.getSubName().empty() ? "" : "->") + "xListEnd");
        return inject_list_item(pi, address, t, list + 16);
    }
    return address;
}

DWORD wrapListInj(PROCESS_INFORMATION &pi, DWORD address, DWORD base, Target& t, bool isItem){
    //this is just a wrapper for list and list_item injections
    if(!isItem) {
        t.setAddress((long) base);
        t.setPointer(false);
        return inject_list(pi, address, t);
    }
    else
        return inject_list_item(pi, address, t, base);
}

DWORD inject_queue(PROCESS_INFORMATION &pi, DWORD address, Target& t){
    //specific injection for queues, it can move the file pointer to move within the struct to explore it.
    uint8_t h[4];
    getAddress(pi, h, reinterpret_cast<LPVOID>(t.getAddress()));
    long queue = (long) (h[0] + h[1]*256 + h[2]*256*256);

    if (address >= queue && address < queue + 8) {
        cout << "Injecting " << t.getName() << "->pcHead\n";
        t.setSubName("pcHead");
        getAddress(pi, h, reinterpret_cast<LPVOID>(queue));
        long newAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        return getRandomAddressInRange(newAddress, newAddress + 50);
    }
    else if(address >= queue + 8 && address < queue + 16) {
        cout << "Injecting " << t.getName() << "pcWriteTo\n";
        t.setSubName("pcWriteTo");
        getAddress(pi, h, reinterpret_cast<LPVOID>(queue + 8));
        long newAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        return getRandomAddressInRange(newAddress, newAddress + 50);
    }
    else if (address >= queue + 32 && address < queue + 72) {
        cout << "Injecting " << t.getName() << "->xTasksWaitingToSend\n";
        t.setSubName( "xTasksWaitingToSend");
        return wrapListInj(pi, address, queue + 32, t, false);
    }
    else if (address >= queue + 72 && address < queue + 112) {
        cout << "Injecting " << t.getName() << "->xTasksWaitingToReceive\n";
        t.setSubName("xTasksWaitingToReceive");
        return wrapListInj(pi, address, queue + 72, t, false);
    }

    else if (address >= queue + 112 && address < queue + 120) {
        cout << "Injecting " << t.getName() << "->uxMessagesWaiting\n";
        t.setSubName("uxMessagesWaiting");
    }
    else if(address >= queue + 120 && address < queue + 128) {
        cout << "Injecting " << t.getName() << "->uxLength\n";
        t.setSubName("uxLength");
    }
    else if(address >= queue + 128 && address < queue + 136) {
        cout << "Injecting " << t.getName() << "->uxItemSize\n";
        t.setSubName("uxItemSize");
    }
    return address;
}

DWORD inject_TCB(PROCESS_INFORMATION &pi, DWORD address, Target& t){
    //specific injection for TCB, it can move the file pointer to move within the struct.
    uint8_t h[4];
    getAddress(pi, h, reinterpret_cast<LPVOID>(t.getAddress()));

    long tcb = (long) (h[0] + h[1]*256 + h[2]*256*256);
    if (address >= tcb && address < tcb + 8) {
        cout << "Injecting " << t.getName() << "->pxTopOfStack\n";
        t.setSubName("pxTopOfStack");
        return address;
    }
    else if(address >= tcb + 8 && address < tcb + 48) {
        cout << "Injecting " << t.getName() << "->xStateListItem\n";
        t.setSubName("xStateListItem");
        return wrapListInj(pi, address, tcb + 8, t, true);
    }
    else if(address >= tcb + 48 && address < tcb + 88) {
        cout << "Injecting " << t.getName() << "->xEventListItem\n";
        t.setSubName("xEventListItem");
        wrapListInj(pi, address, tcb + 48, t, true);
    }
    else if(address >= tcb + 88 && address < tcb + 96) {
        cout << "Injecting " << t.getName() << "uxPriority\n";
        t.setSubName("uxPriority");
    }
    else if (address >= tcb + 96 && address < tcb + 104) {
        t.setSubName("pxStack");
        cout << "Injecting " << t.getName() << "->pxStack\n";
        getAddress(pi, h, reinterpret_cast<LPVOID>(tcb + 96));
        long newAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        return getRandomAddressInRange(newAddress, newAddress + 70);
    }
    else if(address >= tcb + 104 && address < tcb + 112) {
        cout << "Injecting " << t.getName() << "->pcTaskName\n";
        t.setSubName("pcTaskName");
    }
    else if(address >= tcb + 116 && address < tcb + 124) {
        cout << "Injecting " << t.getName() << "->uxTCBNumber\n";
        t.setSubName("uxTCBNumber");
    }
    else if(address >= tcb + 104 && address < tcb + 112) {
        cout << "Injecting " << t.getName() << "->uxTaskNumber\n";
        t.setSubName("uxTaskNumber");
    }
    else if(address >= tcb + 112 && address < tcb + 120) {
        cout << "Injecting " << t.getName() << "->uxBasePriority\n";
        t.setSubName("uxBasePriority");
    }
    else if(address >= tcb + 120 && address < tcb + 128) {
        cout << "Injecting " << t.getName() << "->callback\n";
        t.setSubName("callback");
    }
    else if(address >= tcb + 128 && address < tcb + 136) {
        cout << "Injecting " << t.getName() << "->ulRunTimeCounter\n";
        t.setSubName("ulRunTimeCounter");
    }
    return address;
}

DWORD inject_timer(PROCESS_INFORMATION &pi, DWORD address, Target& t){
    //specific injection for timers
    uint8_t h[4];
    getAddress(pi, h, reinterpret_cast<LPVOID>(t.getAddress()));
    long timer = (long) (h[0] + h[1]*256 + h[2]*256*256);
    if (address >= timer && address < timer + 8) {
        cout << "Injecting xTimer name\n";
        t.setSubName("name");
        getAddress(pi, h, reinterpret_cast<LPVOID>(timer));
        return (long) (h[0] + h[1]*256 + h[2]*256*256);
    }
    else if(address >= timer + 8 && address < timer + 48) {
        cout << "Injecting xTimer->xTimerListItem\n";
        t.setSubName("xTimerListItem");
        return wrapListInj(pi, address, timer + 8, t, true);;
    }
    else if (address >= timer + 48 && address < timer + 56) {
        t.setSubName("xTimerPeriodInTicks");
        cout << "Injecting xTimer->xTimerPeriodInTicks\n";
    }
    else if (address >= timer + 64 && address < timer + 72) {
        t.setSubName("callback");
        cout << "Injecting xTimer callback function\n";
    }
    return address;
}

long injector(PROCESS_INFORMATION &pi, Target &t, long *chosenAddr, int timer_range) {
    //the real injector, given a target t it will choose a random address inside this object and inject it
    //returns the random address in the chosenAddr parameter.
    random_device generator;
    uniform_int_distribution<int> time_dist(100, timer_range);
    int millisecond_time = time_dist(generator);
    this_thread::sleep_for(chrono::milliseconds(millisecond_time));
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
            cerr << "Error reading memory 2: " << GetLastError() << endl;
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
        cerr << "Error reading memory 3: " << GetLastError() << endl;
        return -1;
    }

    if(t.getName() == "xTimer")
        addr = inject_timer(pi, addr, t);
    else if(t.getName() == "pxCurrentTCB" || t.getName() == "xTimerTaskHandle")
        addr = inject_TCB(pi, addr, t);
    else if(t.getName() == "xQueue" || t.getName() == "xTimerQueue")
        addr = inject_queue(pi, addr, t);
    else if(t.getName() == "xActiveTimerList1" || t.getName() == "xDelayedTaskList1" || t.getName() == "xPendingReadyList" || t.getName() == "xSuspendedTaskList" || t.getName() == "pxOverflowTimerList" || t.getName() == "pxCurrentTimerList" || t.getName() == "pxDelayedTaskList")
        addr = inject_list(pi, addr, t);
    if(!ReadProcessMemory(pi.hProcess, reinterpret_cast<LPVOID>(t.getAddress()), &byte, (size_t)1,
                          nullptr)) {
        cerr << "Error reading memory 4: " << GetLastError() << endl;
        return -1;
    }

    //Bitflip
    byte ^= 1 << mask;
    if(!WriteProcessMemory(pi.hProcess, reinterpret_cast<LPVOID>(addr), (LPVOID)&byte, (SIZE_T)1,
                       &length_read)){
        err = GetLastError();
        cout << "Error writing memory" << err << endl;
    }

    cout << "Modified " <<
         // put red color for the bit flipped
         bitset<8>(byte).to_string().insert(8 - mask, COLOR_RESET).insert(7 - mask, COLOR_RED)
         << " at 0x" << hex << addr << endl;
    *chosenAddr = (long) addr;
    cout << hex << *chosenAddr << endl;
    return millisecond_time;
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
        cout<<GetLastError()<<endl;
        printf("hFile is NULL\n");
        cout << "Could not open" << file << endl;
// return error
        return 0;
    }
    long len = (long) GetFileSize( h, nullptr);
    return len;
}

void menu(int &c, int &range, int &numInjection) {
    //just prints a menu to get the injection's parameters
    cout << "Type what do you want to inject:" << endl;
    for(int i = 0; i < objects.size(); i++)
        cout << i << " - " << objects[i].getName() << endl;
    do {
        cin >> c;
    }while(c >= objects.size() || c < 0);

    cout << "Insert a range in millisecond to randomly inject:" << endl;
    cin >> range;
    cout << "Number of injection:" << endl;
    cin >> numInjection;
}

int checkFiles(Target &t, int pid_rtos, chrono::duration<long, std::ratio<1, 1000>> elapsed) {
    //it compares the Golden execution output with the one generated by an instance of RTOS affected by the injection
    //it will register the type of error found
    // if output file is empty --> crash
    // if they differ in some way --> SDC
    // else --> Masked
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
    //this is the function that actually launches an instance of rtos and the injector that will perform the bitflip
    //after the injection is done it will analyze its results and check which kind of error has been generated.
    //it uses checkFiles() to look for Crash, SDC and Masked
    //in order to detect hangs it uses the select() function
    //delay are detected by measuring the execution time of the RTOS instance.
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

    long inj_addr;
    Target t(objects[chosen]);
    thread injection(injector, ref(pi), ref(t), &inj_addr, timer_range);
    if(injection.joinable())
        injection.join();

    chrono::duration<long, std::ratio<1, 1000>> elapsed = chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - begin);

    //hang handling
    DWORD hang;
    hang = WaitForSingleObject(pi.hProcess, gtime.count()*2);
    if (hang == WAIT_TIMEOUT) { //HANG
        cout << endl << "Timeot expired, killing process " << endl;
        TerminateProcess(pi.hProcess, 0);

        logger.addInjection(t, elapsed, "Hang");
    }


    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    chrono::duration<long, std::ratio<1, 1000>> rtime = chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    cout << endl << "RTOS iter time : " << rtime.count() << endl;

    int err = 0;
    if(hang!=WAIT_TIMEOUT)
        err = checkFiles(t, (int)pid_rtos, elapsed);

    long timeDifference = (rtime - gtime).count();
    bool delay = abs(timeDifference) > 1000;

    if(delay && hang != WAIT_TIMEOUT && err!=2) //delay detected when there was not a crash or a hang
        logger.addInjection(t, elapsed, "Delay");

    cout << endl << "Time difference = " << to_string(timeDifference) << "[ms]" << endl;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void execGolden(PROCESS_INFORMATION& pi, chrono::duration<long, ratio<1, 1000>> &gtime){
    //launch an instance of RTOS, without performing any injection, then renames the output file as "Golden_execution".
    DWORD pid_golden;

    STARTUPINFO si = {sizeof(si)};

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    chrono::steady_clock::time_point bgold = chrono::steady_clock::now();
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
    //it takes the gdb.output file (containing all the addresses of the injectable objects) and updates all the addresses in the object list
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
    int rounds = (int) (numInjection / (HW_CONCURRENCY_FACTOR * thread::hardware_concurrency()));
    int extra = (int) (numInjection % (HW_CONCURRENCY_FACTOR * thread::hardware_concurrency()));
    vector<thread> p(HW_CONCURRENCY_FACTOR * thread::hardware_concurrency());
    for (int r = 0; r < rounds; r++){
        for (int iter = 0; iter < HW_CONCURRENCY_FACTOR * thread::hardware_concurrency(); iter++) {
            p[iter] = thread(injectRTOS, ref(pi), iter, chosen, timer_range, gtime);
        }
        for (int i = 0; i < HW_CONCURRENCY_FACTOR * thread::hardware_concurrency(); i++) {
            if(p[i].joinable())
                p[i].join();
        }
    }
    //remaining iterations
    for (int r = 0; r < extra; r++){
        p[r] = thread(injectRTOS, ref(pi), r, chosen, timer_range, gtime);
    }
    for (int r = 0; r < extra; r++){
        if(p[r].joinable())
            p[r].join();
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