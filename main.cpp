#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <random>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

using namespace std;

int injector(pid_t pid, long startAddr, long endAddr) {
    fstream memFile("/proc/" + to_string(pid) + "/mem", ios::binary | ios::in | ios::out);
    if(!memFile.is_open()){
        cout << "Can't open file /proc/" + to_string(pid) + "/mem" << endl;
        return -1;
    }
    random_device generator;
    uniform_int_distribution<int> bit_distribution(0,7);
    uint8_t byte, mask = bit_distribution(generator);
    uniform_int_distribution<long> address_distribution(startAddr, endAddr);

    long addr = address_distribution(generator);
    memFile.seekg(addr);
    byte = memFile.peek();

    //Flipbit
    byte ^= 1 << mask;
    memFile.put((char)byte);

    cout << "Modified " <<
         // put red color for the bit flipped
         bitset<8>(byte).to_string().insert(8 - mask, COLOR_RESET).insert(7 - mask, COLOR_RED)
         << " at 0x" << hex << addr << endl;
    memFile.close();
    return 0;
}

void rtos(){
    cout << "Child running rtos" << endl;
    execl("/home/marco/Scrivania/Progetto_PDS/RTOS_fault_injector/FreeRTOS-cmake/FreeRTOS/Demo/build/freeRTOS",
          "/home/marco/Scrivania/Progetto_PDS/RTOS_fault_injector/FreeRTOS-cmake/FreeRTOS/Demo/build/freeRTOS");
}


int main(int argc, char** argv){
    pid_t pid_golden, pid_injector, pid_rtos;
    int status, status2;
    pid_golden = fork();
    if(pid_golden == 0) {
        rtos(); //golden
        return 0;
    }else {
        int status;
        waitpid(pid_golden, &status, 0);
    }
    pid_golden = fork();
    if(pid_golden == 0) {
        execl("/bin/mv", "/bin/mv", "../Falso_Dante.txt", "../Golden_execution.txt", (char *)0);
        return 0;
    }else {

        waitpid(pid_golden, &status, 0);
    }
    pid_rtos = fork();
    if(pid_rtos == 0){
        rtos();
        return 0;
    }
    long startAddr = 0x431000, endAddr = 0x432000;

    //TODO: Select a random range of address to inject


    pid_injector = fork();
    if(pid_injector == 0){
        usleep((rand() % 8000000));
        injector(pid_rtos, startAddr, endAddr);
        return 0;
    }
    waitpid(pid_rtos, &status, 0);
    waitpid(pid_injector, &status2, 0);
    //TODO: Check the output with the golden
}
