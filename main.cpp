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
#include <limits>
#include <signal.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

using namespace std;

static volatile int cnt = 0;

/*
int injector(pid_t pid, long startAddr, long endAddr) {
	cout << "Child starting injector" << endl;
	ifstream mapfile;
	mapfile.open("/proc/" + to_string(pid) + "/maps");
	if(!mapfile.is_open()){
		cout << "Can't open file /proc/" + to_string(pid) + "/maps" << endl;
		return 2;
	}
	string line;
	int numAddrs = 0;
	long address[20][2];
	while (getline(mapfile, line) && mapfile.good() && line.find("heap") == string::npos);
	while (getline(mapfile, line) && mapfile.good() && line.find('/') == string::npos){
		string perms;
		perms = line.substr(line.find(' '), 4);
		if (perms.find('w') == string::npos || perms.find('r') == string::npos)
			continue;
		size_t separator = line.find('-');
		address[numAddrs][0] = stol(line.substr(0, separator), nullptr, 16);
		address[numAddrs][1] = stol(line.substr(separator + 1, separator), nullptr, 16);
		numAddrs++;
	}
	mapfile.close();
	if(numAddrs == 0){
		cout << "no address found" << endl;
		return 1;
	}

	fstream memFile("/proc/" + to_string(pid) + "/mem", ios::binary | ios::in | ios::out);
	if (!memFile.is_open()) {
		cout << "Can't open file /proc/" + to_string(pid) + "/mem" << endl;
		return -1;
	}
	random_device generator;
	uniform_int_distribution<int> bit_distribution(0, 7);
	uniform_int_distribution<int> range_addrs(0, numAddrs-1);
	for (int i = 0; i < 100; ++i) {
		uint8_t byte, mask = bit_distribution(generator);
		int index = range_addrs(generator);
		uniform_int_distribution<long> address_distribution(address[index][0], address[index][1]);
		long addr = address_distribution(generator);
		memFile.seekg(addr);
		byte = memFile.peek();
		for(int timer = 0; timer < 1000 && byte == 0; timer++){
			index = range_addrs(generator);
			uniform_int_distribution<long> address_distribution(address[index][0], address[index][1]);
			addr = address_distribution(generator);
			memFile.seekg(addr);
			byte = memFile.peek();
		}

		//Flipbit
		byte ^= 1 << mask;
		memFile.put((char) byte);

		cout << "Modified " <<
			 // put red color for the bit flipped
			 bitset<8>(byte).to_string().insert(8 - mask, COLOR_RESET).insert(7 - mask, COLOR_RED)
			 << " at 0x" << hex << addr << endl;
	}
	memFile.close();
	return 0;
}*/
int injector(pid_t pid, long startAddr, long endAddr) {
    cout << "Child starting injector" << endl;
    fstream memFile("/proc/" + to_string(pid) + "/mem", ios::binary | ios::in | ios::out);
    if (!memFile.is_open()) {
        cout << "Can't open file /proc/" + to_string(pid) + "/mem" << endl;
        return -1;
    }
    random_device generator;
    uniform_int_distribution<int> bit_distribution(0, 7);
    uint8_t byte, mask = bit_distribution(generator);
    uniform_int_distribution<long> address_distribution(startAddr, endAddr);

    long addr = address_distribution(generator);
    memFile.seekg(addr);
    byte = memFile.peek();

    //Flipbit
    byte ^= 1 << mask;
    memFile.put((char) byte);

    cout << "Modified " <<
         // put red color for the bit flipped
         bitset<8>(byte).to_string().insert(8 - mask, COLOR_RESET).insert(7 - mask, COLOR_RED)
         << " at 0x" << hex << addr << endl;
    memFile.close();
    return 0;
}

void rtos() {
	cout << endl << "Child running rtos" << endl;
	execve("../build/freeRTOS", nullptr, nullptr);
	//execl("../build/freeRTOS",
	//    "../build/freeRTOS");
}

void sigCHLDHandler(int){
    cnt++;
}

long getFileLen(ifstream &file) {
    file.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = file.gcount();
    file.clear();   //  Since ignore will have set eof.
    file.seekg(0, std::ios_base::beg);
    return (long) length;
}

int checkFiles(int pid_golden, int pid_rtos) {
	ifstream golden_output("../files/Golden_execution" + to_string(pid_golden) + ".txt");
	ifstream rtos_output( "../files/Falso_Dante_" + to_string(pid_rtos) + ".txt");
	bool found = false;
    long s1, s2;
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
    s1 = getFileLen(golden_output);
    s2 = getFileLen(rtos_output);

	for (string g_line, f_line; getline(golden_output, g_line), getline(rtos_output, f_line);) {
		if (g_line != f_line) {
			found = true;
			cout << "The output should be" << endl << g_line << endl
				 << "instead I found" << endl << f_line << endl;
		}
	}
	if (!found)
		cout << endl << "No differences have been found" << endl;
    if(s1!=s2)
        cout << endl << "Files differ in size" << endl << "golden = " << s1 << "; falso = " << s2 << endl;

	rtos_output.close();
	golden_output.close();
	return 0;
}

int main(int argc, char **argv) {
    signal(SIGCHLD, sigCHLDHandler);
	pid_t pid_golden, pid_injector, pid_rtos;
    int status, status2;
	int chosen;
	if (argc < 2)
		chosen = 1;
	else
		chosen = (int) strtol(argv[1], nullptr, 10);

	pid_golden = fork();
	if (pid_golden == 0) {
		//DO NOT REMOVE, FOR SOME REASON THE PROGRAM WONT START IF YOU REMOVE THIS
		this_thread::sleep_for(chrono::seconds(1));
		rtos(); //golden
		return 0;
	}
	waitpid(pid_golden, &status, 0);
    const string cmd = "mv ../files/Falso_Dante_" + to_string(pid_golden) + ".txt ../files/Golden_execution" + to_string(pid_golden) + ".txt";
	system((const char *) cmd.c_str());

	int iter = 0;
	while (iter < 8) {
		cout << endl << "Itering injections, iteration : " << iter << endl;
		pid_rtos = fork();
		if (pid_rtos == 0) {
			cout << "freeRTOS iter : " << iter << endl;
			this_thread::sleep_for(chrono::milliseconds (300));
			rtos();
			return 0;
		}
		long startAddr = 0x431000, endAddr = 0x432000;
		//long addresses[6] = {0x433ba0, 0x433ba8, 0x4316e8, 0x4316f0, 0x431700, 0x4311f8};
        string variables[7] = {"xNextTaskUnblockTime", "xIdleTaskHandle", "xTickCount", "xSchedulerRunning", "xYieldPending", "uxTaskNumber", "xIdleTaskHandle"};
        long addrOS[7] = {0x431a28, 0x431a30, 0x4319f0, 0x431a00, 0x431a10, 0x431a20, 0x431a30};
        long addr = 0x316e8;

        //TODO: Select a random range of address to inject

		pid_injector = fork();
		if (pid_injector == 0) {
            //ptrace(PTRACE_SEIZE, pid_rtos, NULL, NULL);
            this_thread::sleep_for(chrono::milliseconds(rand() % 7000));
			//injector(pid_rtos, startAddr - 0x400000, endAddr - 0x400000);
            switch(chosen){
                case 0:
                    injector(pid_rtos, 0x431320, 0x431320 + 176); //TCB1 --> crash or hang or nothing
                    break;
                case 1:
                    injector(pid_rtos, 0x431620, 0x431620 + 176); //TCB2 --> crash or hang or nothing
                    break;
                case 2:
                    injector(pid_rtos, 0x4313e0, 0x4313e0 + 576); //TCB3 --> crash or hang or nothing
                    break;
                case 3:
                    injector(pid_rtos, 0x431760, 0x431760 + 16);
                    break;
                default:
                    break;
            }
			//injector(pid_rtos, 0x431320, 0x431320 + 256);
            //injector(pid_rtos, addrOS[chosen], addrOS[chosen]+8);
            return 0;
		}
        waitpid(pid_injector, &status2, 0);
        //hang handling
        struct timeval timeout = {20,0};
        int rc;
        rc = select(0, NULL,NULL,NULL, &timeout );
        if (rc == 0) {
            cout << endl << "Timeot expired, killing process " << endl;
            kill(pid_rtos, SIGKILL);
        }
        else if (cnt == 2) {
            cnt = 0;
        }
        waitpid(pid_rtos, &status, 0);

		/*string cmd = "diff ../Golden_execution.txt ../Falso_Dante.txt >> ../diffs/diff" + to_string(iter) + ".txt";
		cout << endl << "Now printing differences between generated files" << endl;
		system(cmd.c_str());
		cout << "print done" << endl;*/
		checkFiles(pid_golden, pid_rtos);
		iter++;
	}

	return 0;
}
