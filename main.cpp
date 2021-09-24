#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

using namespace std;
int main(int argc, char** argv) {
    if(argc != 2){
        cout << "Inserisci il pid" << endl;
        return -1;
    }
    string pid = string(argv[1]);
    ifstream mapfile;
    mapfile.open("/proc/" + pid + "/maps");
    if(!mapfile.is_open()){
        cout << "Can't open file /proc/{}/maps" << endl;
        return -2;
    }
    string line, perms;
    long startAddr = -1, endAddr;
    while (getline(mapfile, line) && mapfile.good()){
        if(line.find("heap") != string::npos) {
            size_t separetor = line.find('-');
            startAddr = stol(line.substr(0, separetor), nullptr, 16);
            endAddr = stol(line.substr(separetor+1, separetor), nullptr, 16);
            perms = line.substr(line.find(' '), 4);
            break;
        }
    }
    mapfile.close();
    if(startAddr == -1) {
        cout << "Heap not found" << endl;
        return -3;
    }
    if(perms.find('w') == string::npos && perms.find('r') == string::npos){
        cout << "You dont have permission to write/read" << endl;
        return -4;
    }
//    fstream memFile("test", ios::binary | ios::in | ios::out);
    fstream memFile("/proc/" + pid + "/mem", ios::binary | ios::in | ios::out);
    if(!memFile.is_open()){
        cout << "Can't open file /proc/" + pid + "/mem" << endl;
        return -5;
    }
    memFile.seekg(startAddr);
    uint8_t bit, mask = 1;
    bit = memFile.peek();
//    memFile.get(bit);

    cout << "original bit:" << endl;
    cout << bitset<8>(bit) << endl;
    memFile.put(bit ^ mask);
    memFile.seekg(startAddr);
    cout << "modified bit:" << endl;
    bit = memFile.peek();
    cout << bitset<8>(bit) << endl;
    memFile.close();

    return 0;
}
