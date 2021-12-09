#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <random>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <limits>
#include "Logger.hpp"
#include <tuple>
#include <windows.h>
#include <tchar.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

using namespace std;
Logger logger;

static volatile int cnt = 0;

using namespace std;

int main(int argc, char **argv){
    int status, status2;
    int chosen;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    chrono::duration<long, ratio<1, 1000>> gtime{};
    if (argc < 2)
        chosen = 1;
    else
        chosen= (int) strtol(argv[1], nullptr, 10);
    ifstream gold("../files/Golden_execution.txt");
    if(!gold){
        chrono::steady_clock::time_point bgold = chrono::steady_clock::now();
        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );

        // Start the child process.
        if( !CreateProcess( NULL,   // No module name (use command line)
                            "./RTOSDemo.exe",        // Command line
                            NULL,           // Process handle not inheritable
                            NULL,           // Thread handle not inheritable
                            FALSE,          // Set handle inheritance to FALSE
                            0,              // No creation flags
                            NULL,           // Use parent's environment block
                            NULL,           // Use parent's starting directory
                            &si,            // Pointer to STARTUPINFO structure
                            &pi )           // Pointer to PROCESS_INFORMATION structure
                )
        {
            printf( "CreateProcess failed (%d).\n", GetLastError() );
            return -1;
        }

        // Wait until child process exits.
        WaitForSingleObject( pi.hProcess, INFINITE );

        // Close process and thread handles.
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
    }
    cout<< "Hello world!\n";
    return 0;
}