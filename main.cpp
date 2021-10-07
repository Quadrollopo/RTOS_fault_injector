#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <random>
#include <thread>
#include <chrono>

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

using namespace std;

int main(int argc, char** argv) {
    string pid;
    if(argc == 1){
        ifstream pidFile("rtos_pid.txt");
        getline(pidFile, pid);
        pidFile.close();
    }
    else {
        pid = string(argv[1]);
    }
    ifstream mapfile;
    mapfile.open("/proc/" + pid + "/maps");
    if(!mapfile.is_open()){
        cout << "Can't open file /proc/{}/maps" << endl;
        return 2;
    }
    string line, perms;
    long startAddr = -1, endAddr;
    while (getline(mapfile, line) && mapfile.good()){
        if(line.find("heap") != string::npos) {
            size_t separator = line.find('-');
            startAddr = stol(line.substr(0, separator), nullptr, 16);
            endAddr = stol(line.substr(separator + 1, separator), nullptr, 16);
            perms = line.substr(line.find(' '), 4);
            break;
        }
    }
    mapfile.close();
    cout << (endAddr - startAddr) / 1024 << "KB di heap allocato" << endl;
    if(startAddr == -1) {
        cout << "Heap not found" << endl;
        return 3;
    }
    if(perms.find('w') == string::npos && perms.find('r') == string::npos){
        cout << "You dont have permission to write/read" << endl;
        return 4;
    }
//    fstream memFile("test", ios::binary | ios::in | ios::out);
    fstream memFile("/proc/" + pid + "/mem", ios::binary | ios::in | ios::out);
    if(!memFile.is_open()){
        cout << "Can't open file /proc/" + pid + "/mem" << endl;
        return -5;
    }
    random_device generator;
    uniform_int_distribution<long> addressDistribution(startAddr, endAddr);
    uniform_int_distribution<int> bit_distribution(0,7);
    bool flag = false;
    for (int i = 0; i < 60; ++i) {
        uint8_t byte, mask = bit_distribution(generator);
        long addr;
        do {
            addr = addressDistribution(generator);
            memFile.seekg(addr);
            byte = memFile.peek();
        } while(byte == 0 || byte == 165);
        if(byte == 255)
            if(flag)
                break;
            else
                flag = true;
        else
            flag = false;
        //Flipbit
        byte ^= 1 << mask;

        memFile.seekg(addr);
        memFile.put((char)byte);

        cout << "Modified " <<
        // put red color for the bit flipped
        bitset<8>(byte).to_string().insert(mask + 1, COLOR_RESET).insert(mask, COLOR_RED)
        << " at 0x" << hex << addr << endl;
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    memFile.close();
    return 0;
}
