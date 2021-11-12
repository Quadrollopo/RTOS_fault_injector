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
    cout << "Child starting injector" << endl;
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
    cout << endl << "Child running rtos" << endl;
    execve("../build/freeRTOS", NULL, NULL);
    //execl("../build/freeRTOS",
      //    "../build/freeRTOS");
}

int checkFiles(){
    ifstream golden_output("../Golden_execution.txt");
    ifstream rtos_output("../Falso_Dante.txt");
    bool found = false;
    if(!golden_output.is_open()){
        cout << "Can't open the golden execution output" << endl;
        return -1;
    }
    if(!rtos_output.is_open()){
        cout << "Can't open the rtos execution output" << endl;
        return -1;
    }
    for (string g_line, f_line; getline(golden_output, g_line), getline(rtos_output, f_line); ){
        if (g_line != f_line){
            found = true;
            cout << "The output should be" << endl << g_line << endl
                 << "instead I found" << endl << f_line << endl;
        }
    }
    if(!found)
        cout << endl << "No differences has been found" << endl;
    rtos_output.close();
    golden_output.close();
    return 0;
}

int main(int argc, char** argv){
    pid_t pid_golden, pid_injector, pid_rtos;
    int status, status2;
    int chosen;
    if(argc < 2)
        chosen = 2;
    else
        chosen = (int) strtol(argv[1], NULL, 10);

    pid_golden = fork();
    if(pid_golden == 0) {
		//DO NOT REMOVE, FOR SOME REASON THE PROGRAM WONT START IF YOU REMOVE THIS COUT
		this_thread::sleep_for(chrono::seconds(1));

		rtos(); //golden
        return 0;
    }
	waitpid(pid_golden, &status, 0);
    system("mv ../Falso_Dante.txt ../Golden_execution.txt");

    int iter = 0;
    while(iter<8) {
        cout << endl << "Itering injections, iteration : " << iter << endl;
        pid_rtos = fork();
        if(pid_rtos == 0){
            cout << "freeRTOS iter : " << iter << endl;
            //this_thread::sleep_for(chrono::seconds(1));
            rtos();
            return 0;
        }

            long startAddr = 0x431000, endAddr = 0x432000;
            long addresses[4] = {0x433ba0, 0x433ba8, 0x4316e8, 0x4316f0};

            //TODO: Select a random range of address to inject

        pid_injector = fork();
        if(pid_injector == 0){
            this_thread::sleep_for(chrono::milliseconds (rand()%9000));
            //injector(pid_rtos, startAddr, endAddr);
            injector(pid_rtos, addresses[chosen], addresses[chosen] + 8);
            return 0;
        }
        waitpid(pid_rtos, &status, 0);
        waitpid(pid_injector, &status2, 0);
        /*
        string cmd = "diff ../Golden_execution.txt ../Falso_Dante.txt >> ../diffs/diff" + to_string(iter) + ".txt";
        cout << endl << "Now printing differences between generated files" << endl;
        system(cmd.c_str());
        cout << "print done" << endl; */
        checkFiles();
        iter++;
    }

return 0;
}
