#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>
#include <tchar.h>

static void CloseIfConsoleWindow(DWORD process_id)
{
    if (!AttachConsole(process_id))
        return;

    printf("Attached!\n");
        
    const HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        
    if (hStdOut != NULL)
    {
        CONSOLE_SCREEN_BUFFER_INFO console_buffer_info = { 0 };
        if (GetConsoleScreenBufferInfo(hStdOut, &console_buffer_info))
        {
            console_buffer_info.dwCursorPosition.X;

            const COORD coordBufSize = { 80, 2 };
            CHAR_INFO chiBuffer[80 * 2] = { 0 }; // [2][80];
            const COORD coordBufCoord = { 0, 0 };
            SMALL_RECT srcReadRect;
            srcReadRect.Top = console_buffer_info.dwCursorPosition.Y;
            srcReadRect.Bottom = console_buffer_info.dwCursorPosition.Y;
            srcReadRect.Left = 0;
            srcReadRect.Right = 80;
            if (ReadConsoleOutput(
                hStdOut,        // screen buffer to read from 
                chiBuffer,      // buffer to copy into 
                coordBufSize,   // col-row size of chiBuffer 
                coordBufCoord,  // top left dest. cell in chiBuffer 
                &srcReadRect)) // screen buffer source rectangle 
            {
                printf("Buffer:\n%S\n", chiBuffer);
            }
    
        }
    }
    
     FreeConsole();   
}

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam)
{
    if (!IsWindow(hwnd))
        return TRUE;
    
    const size_t maxTitleLength = 255;
    wchar_t title[maxTitleLength + 1] = { 0 };
    GetWindowText(hwnd, title, maxTitleLength);
    DWORD process_id = 0;
    if (!GetWindowThreadProcessId(hwnd, &process_id))
        return TRUE;

    printf("Title: %S\n", title);
    if (!wcsstr(title, L"cmd.exe"))
        return TRUE;
    
    CloseIfConsoleWindow(process_id);
    
    return TRUE;
}

static void FindAndCloseConsoleWindows()
{
    if (!EnumWindows(EnumWindowsProc, NULL))
    {
        printf("EnumWindows failed %d\n", GetLastError);
    }
}

static void PrintProcessNameAndID( DWORD processID )
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.
    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS |
                                   PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.
    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName, 
                               sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }

    // Release the handle to the process.
    CloseHandle( hProcess );

    // Print the process name and identifier.
    if (_tcsstr(szProcessName, _T("cmd.exe")))
    {
        _tprintf( TEXT("%s  (PID: %u)\n"), szProcessName, processID );
        CloseIfConsoleWindow(processID);
    }

}

int main()
{
    DWORD pids[1024], cbNeeded, cProcesses;
    EnumProcesses(pids, sizeof(pids), &cbNeeded);
    const size_t numProcessesFound = cbNeeded / sizeof(DWORD);
    for (size_t i = 0; i < numProcessesFound; ++i)
    {
        PrintProcessNameAndID(pids[i]);
    }
    
    return 0;
}