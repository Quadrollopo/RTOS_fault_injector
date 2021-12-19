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
#include "Logger.hpp"
#include <tuple>

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

using namespace std;
Logger logger;
vector<tuple<long, string>> rtosObj = {{0x431840, "pxCurrentTCB"}, {0x431860, "pxReadyTasksLists"}, {0x431980, "xDelayedTaskList1"}, {0x4319c0, "xDelayedTaskList2"}, {0x4319e8, "pxDelayedTaskList"}, {0x431a00, "xPendingReadyList"}, {0x431a80, "xSuspendedTaskList"}, {0x431a40, "xTasksWaitingTermination"}, {0x431aa8, "uxCurrentNumberOfTasks"}, {0x431ab8, "uxTopReadyPriority"}, {0x431ab0, "xTickCount"}, {0x431ac8, "xPendedTicks"}, {0x431ad0, "xYieldPending"}, {0x431af8, "uxSchedulerSuspended"}, {0x431af0, "xIdleTaskHandle"}, {0x431ae8, "xNextTaskUnblockTime"}, {0x431ac0, "xSchedulerRunning"}, {0x431ae0, "uxTaskNumber"}, {0x431b20, "xActiveTimerList1"}, {0x431b60, "xActiveTimerList2"}, {0x431b88, "pxCurrentTimerList"}, {0x431b90, "pxOverflowTimerList"}, {0x431b98, "xTimerQueue"}, {0x431ba0, "xTimerTaskHandle"}, {0x431f20, "xResumeSignals"}, {0x431fa0, "xAllSignals"}, {0x4320a0, "hMainThread"}, {0x4320b0, "xSchedulerEnd"}, {0x432720, "xMutex"}, {0x432728, "xErrorOccurred"}, {0x432730, "xControllingIsSuspended"}, {0x432738, "xBlockingIsSuspended"}, {0x432740, "uxControllingCycles"}, {0x432748, "uxBlockingCycles"}, {0x432750, "uxPollingCycles"}, {0x432758, "xControllingTaskHandle"}};

static volatile int cnt = 0;

int injector(pid_t pid, long startAddr, long endAddr, long *chosenAddr, int timer_range) {
    this_thread::sleep_for(chrono::milliseconds(rand() % timer_range));
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
    *chosenAddr = addr;
    cout << hex << *chosenAddr << endl;
    return 0;
}

void rtos() {
	cout << endl << "Child running rtos" << endl;
	execve("../build/freeRTOS", nullptr, nullptr);
}

void sigCHLDHandler(int){
    cnt++;
}

void sigINTHandler(int){
    logger.printInj();
    exit(0);
}

long getFileLen(ifstream &file) {
	file.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize length = file.gcount();
	file.clear();   //  Since ignore will have set eof.
	file.seekg(0, std::ios_base::beg);
	return (long) length;
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
	s1 = getFileLen(golden_output);
	s2 = getFileLen(rtos_output);

	if (s2 == 0) { // Crash
		cout << endl << "Files differ in size" << endl << "golden = " << s1 << "; falso = " << s2 << endl;
		logger.addInjection(name, addr, elapsed, "Crash");
		error = 2;
	} else {
		for (string g_line, f_line; getline(golden_output, g_line), getline(rtos_output, f_line);) {
			if (g_line != f_line) {
				found = true;
				error = 1;
				cout << "The output should be" << endl << g_line << endl
					 << "instead I found" << endl << f_line << endl;
			}
		}
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

void execGolden(chrono::duration<long, ratio<1, 1000>> &gtime) {
	chrono::steady_clock::time_point beginTime = chrono::steady_clock::now();
	pid_t pid_golden = fork();
	if (pid_golden == 0) {
		//DO NOT REMOVE, FOR SOME REASON THE PROGRAM WONT START IF YOU REMOVE THIS
		this_thread::sleep_for(chrono::seconds(1));

		rtos(); //golden
		exit(0);
	}
	int status;
    waitpid(pid_golden, &status, 0);
    //Golden time
    gtime = chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now() - beginTime);
    cout << endl << "Golden time : " << gtime.count() << endl;
	ofstream time_golden("../files/Time_golden.txt");
	if (time_golden)
		time_golden << gtime.count();
	else
		cout << "Can't create Time_golden.txt";
	time_golden.close();
	cnt = 0;
	const string cmd = "mv ../files/Falso_Dante_" + to_string(pid_golden) + ".txt ../files/Golden_execution.txt";
	system((const char *) cmd.c_str());
}

void injectRTos(int numInjection, int chosen, int timer_range, chrono::duration<long, ratio<1, 1000>> gtime) {
	int status;
	long addr1, addr2;

    addr1 = get<0>(rtosObj[chosen]);
    addr2 = addr1 + 8;
    string name = get<1>(rtosObj[chosen]);
	for (int iter = 0; iter < numInjection; iter++) {
		cout << endl << "Itering injections, iteration : " << iter << endl;
		chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		pid_t pid_rtos = fork();
		if (pid_rtos == 0) {
			cout << "freeRTOS iter : " << iter << endl;
			this_thread::sleep_for(chrono::milliseconds(300));
			rtos();
			exit(0);
		}

		//opzione1 ->


		long inj_addr;
		thread injection(injector, pid_rtos, addr1, addr2, &inj_addr, timer_range);
		injection.join();

		chrono::duration<long, std::ratio<1, 1000>> elapsed = chrono::duration_cast<std::chrono::milliseconds>(
				chrono::steady_clock::now() - begin);

		//hang handling
		struct timeval timeout = {40, 0};
		int hang;
		hang = select(0, nullptr, nullptr, nullptr, &timeout);
		if (hang == 0) { //HANG
			cout << endl << "Timeout expired, killing process " << endl;
			kill(pid_rtos, SIGKILL);

			logger.addInjection(name, inj_addr, elapsed, "Hang");
		} else if (cnt > 0) {
			cnt = 0;
		}
		waitpid(pid_rtos, &status, 0);
		chrono::steady_clock::time_point end = chrono::steady_clock::now();

		chrono::duration<long, std::ratio<1, 1000>> rtime = chrono::duration_cast<std::chrono::milliseconds>(
				end - begin);
		cout << endl << "RTOS iter time : " << rtime.count() << endl;

		int err = 0;
		if (hang != 0)
			err = checkFiles(name, pid_rtos, inj_addr, elapsed);

		long timeDifference = chrono::duration_cast<chrono::milliseconds>(rtime - gtime).count();

		if (abs(timeDifference) > 800 && hang != 0 && err != 2) //delay detected when there was not a crash or a hang
			logger.addInjection(name, inj_addr, elapsed, "Delay");

		cout << endl << "Time difference = " << to_string(timeDifference) << "[ms]" << endl;

	}

}

void menu(int &c, int &range, int &numInjection) {
	cout << "Type what do you want to inject:" << endl;
    for(int i = 0; i < rtosObj.size(); i++)
        cout << i << " - " << get<1>(rtosObj[i]) << endl;
	do {
		cin >> c;
	}while(c >= rtosObj.size() || c < 0);

	// Sta frase non ha molto senso, magari qualcosa in inglese non sarebbe male
	cout << "Insert a range in millisecond to randomly inject:" << endl;
	cin >> range;
	cout << "Number of injection:" << endl;
	cin >> numInjection;
}


int main(int argc, char **argv) {
    signal(SIGCHLD, sigCHLDHandler);
    signal(SIGINT, sigINTHandler);
	int chosen, numInjection, timer_range;
	chrono::duration<long, ratio<1, 1000>> gtime{};
	menu(chosen, timer_range, numInjection);

	//If already exist a golden execution, dont start another one
	ifstream gold("../files/Golden_execution.txt");
	if(!gold) {
		execGolden(gtime);
	}else{
		cout << "Found another golden execution output, skipping execution..." << endl;
		ifstream time_golden("../files/Time_golden.txt");
		if (time_golden) {
			long time;
			time_golden >> time;
			gtime = chrono::milliseconds(time);
			time_golden.close();
		} else {
			cout << "Can't open Time_golden.txt: re-exec the golden execution";
			execGolden(gtime);
		}
	}
	gold.close();


	injectRTos(numInjection, chosen, timer_range, gtime);

    logger.printInj();

	return 0;
}
