#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <random>
#include <thread>
#include <chrono>

using namespace std;

int main(int argc, char** argv) {
    if(argc != 2){
        cout << "Inserisci il pid" << endl;
        return 1;
    }
    string pid = string(argv[1]);
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
    for (int i = 0; i < 100; ++i) {
        long addr = addressDistribution(generator);
        memFile.seekg(addr);
        uint8_t byte, mask = bit_distribution(generator);
        byte = memFile.peek();
        memFile.put(byte ^ (1 << mask));
        memFile.seekg(addr);
        cout << "Modified " << bitset<8>(byte) << " at " << addr << endl;
        this_thread::sleep_for(chrono::milliseconds(200));
    }
    memFile.close();
    return 0;
}
