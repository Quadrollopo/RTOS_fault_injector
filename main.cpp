#include <iostream>
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

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

#define PRINT_ON_FILE 1

using namespace std;
Logger logger;
vector<Target> objects = {{"xActiveTimerList1", 0x431a40, 40, false}, {"hMainThread", 0x431fc0, 8, false}, {"xResumeSignals", 0x431e40, 128, false}, {"pxReadyTasksLists", 0x431780, 280, false}, {"xDelayedTaskList1", 0x4318a0, 40, false}, {"xPendingReadyList", 0x431920, 40, false}, {"xSuspendedTaskList", 0x4319a0, 40, false}, {"uxTopReadyPriority", 0x4319d8, 8, false}, {"xTickCount", 0x4319d0, 8, false}, {"xPendedTicks", 0x4319e8, 8, false}, {"uxSchedulerSuspended", 0x431a18, 8, false}, {"xNextTaskUnblockTime", 0x431a08, 8, false}, {"xSchedulerRunning", 0x4319e0, 8, false}, {"uxTaskNumber", 0x431a00, 8, false}, {"xTimerTaskHandle", 0x431ac0, 176, true}, {"xTimerQueue", 0x431ab8, 168, true}, {"pxOverflowTimerList", 0x431ab0, 40, true}, {"pxCurrentTimerList", 0x431aa8, 40, true}, {"pxCurrentTCB", 0x431760, 176, true}, {"pxDelayedTaskList", 0x431908, 40, true}, {"xIdleTaskHandle", 0x431a10, 176, true}, {"xQueue", 0x4316d0, 168, true}, {"xTimer", 0x4316d8, 88, true}};
static volatile int cnt = 0;

int injector(pid_t pid, const Target& t, long *chosenAddr, int timer_range) {
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
    uniform_int_distribution<long> address_distribution;
    if(!t.isPointer())
        address_distribution = uniform_int_distribution<long>(t.getAddress(), t.getAddress() + t.getSize());
    else {
        memFile.seekg(t.getAddress());
        uint8_t h[4];
        memFile.read(reinterpret_cast<char *>(h), 4);
        long heapAddress = (long) (h[0] + h[1]*256 + h[2]*256*256);
        address_distribution = uniform_int_distribution<long>(heapAddress, heapAddress + t.getSize());
    }
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
#if PRINT_ON_FILE
        logger.logOnfile();
#else
        logger.printInj();
#endif
    exit(0);
}

long getFileLen(ifstream &file) {
	file.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize length = file.gcount();
	file.clear();   //  Since ignore will have set eof.
	file.seekg(0, std::ios_base::beg);
	return (long) length;
}

int checkFiles(const string& name, int pid_rtos, long addr, chrono::duration<long, std::ratio<1, 1000>> elapsed) {
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

void injectRTos(int chosen, int timer_range, chrono::duration<long, ratio<1, 1000>> gtime, int iter) {
	int status;
	long addr1, addr2;
	string name = objects[chosen].getName();

	cout << endl << "Itering injections, iteration : " << iter << endl;
	chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	pid_t pid_rtos = fork();
	if (pid_rtos == 0) {
		cout << "freeRTOS iter : " << iter << endl;
		this_thread::sleep_for(chrono::milliseconds(300));
		rtos();
		exit(0);
	}

	long inj_addr;
	injector(pid_rtos, objects[chosen], &inj_addr, timer_range);

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
    bool delay = abs(timeDifference) > 750 || abs(timeDifference) < 650;

	if (delay && hang != 0 && err != 2) //delay detected when there was not a crash or a hang
		logger.addInjection(name, inj_addr, elapsed, "Delay");

	cout << endl << "Time difference = " << to_string(timeDifference) << "[ms]" << endl;


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


int main() {
    signal(SIGCHLD, sigCHLDHandler);
    signal(SIGINT, sigINTHandler);
    srand(std::chrono::system_clock::now().time_since_epoch().count());
    system("echo 0 | sudo tee /proc/sys/kernel/randomize_va_space");
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
#if PARALLELIZATION
	vector<thread> p(numInjection);
	for (int iter = 0; iter < numInjection; iter++) {
		p[iter] = thread(injectRTos, chosen, timer_range, gtime, iter);
	}
	for (int i = 0; i < numInjection; i++) {
		p[i].join();
	}
#else
    for (int iter = 0; iter < numInjection; iter++)
        injectRTos(chosen, timer_range, gtime, iter);
#endif

#if PRINT_ON_FILE
    logger.logOnfile();
#else
    logger.printInj();
#endif
    system("echo 2 | sudo tee /proc/sys/kernel/randomize_va_space");
	return 0;
}
