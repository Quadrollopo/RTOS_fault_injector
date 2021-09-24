#include <iostream>
#include <fstream>
#include <string>

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
            startAddr = stol(line.substr(0, 8));
            endAddr = stol(line.substr(9, 8));
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
    fstream memFile("test", ios::binary | ios::in | ios::out);
    memFile.open("/proc/" + pid + "/mem");
    if(!memFile.is_open()){
        //cout << "Can't open file /proc/" + pid + "/mem" << endl;
        return -5;
    }
    memFile.seekg(startAddr, ios::beg);
    char bit;

    memFile.get(bit);
    cout << bit + ' ' + (bit & 1);
    memFile.close();

    return 0;
}
