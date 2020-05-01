// TestACP.cpp --- Test for QueueUserAPC and threads
#include <windows.h>
#include <process.h>    // for _beginthreadex
#include <stdio.h>      // for printf

BOOL g_bTerminateAll = FALSE;

#define APC_HACK

#ifdef APC_HACK
    #define HANDLE_APC_HACK(fn, msg) case fn##_MSG: fn((msg).lParam); break;
    #define QUEUE_APC(fn, hThread, idThread, arg) \
        PostThreadMessage((idThread), fn##_MSG, 0, (LPARAM)(arg))

    #define DoUserAPC_MSG (WM_USER + 100)
    #define TerminateAPC_MSG (WM_USER + 101)
#else
    #define HANDLE_APC_HACK(fn, msg)
    #define QUEUE_APC(fn, hThread, idThread, arg) \
        QueueUserAPC(fn, hThread, arg);
#endif

VOID NTAPI DoUserAPC(ULONG_PTR Parameter)
{
    printf("DoUserAPC(%p)\n", Parameter);
}

VOID NTAPI TerminateAPC(ULONG_PTR Parameter)
{
    printf("TerminateAPC(%p)\n", Parameter);
    g_bTerminateAll = TRUE;
}

unsigned __stdcall thread_proc(void *)
{
#ifdef APC_HACK
    MSG msg;
#endif
    while (!g_bTerminateAll)
    {
        printf("thread_proc: sleep\n");
#ifdef APC_HACK
        WaitMessage();
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
                HANDLE_APC_HACK(DoUserAPC, msg);
                HANDLE_APC_HACK(TerminateAPC, msg);
            }
            DispatchMessage(&msg);
        }
#else
        SleepEx(INFINITE, TRUE);
#endif
        printf("thread_proc: awake\n");
    }
    return 0;
}

int main(void)
{
    unsigned tid;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, thread_proc, NULL, 0, &tid);

    printf("main: sleep\n");
    Sleep(3 * 1000);
    printf("main: awake\n");

    QUEUE_APC(DoUserAPC, hThread, tid, 0);

    printf("main: sleep\n");
    Sleep(3 * 1000);
    printf("main: awake\n");

    QUEUE_APC(DoUserAPC, hThread, tid, 1);

    printf("main: sleep\n");
    Sleep(3 * 1000);
    printf("main: awake\n");

    QUEUE_APC(TerminateAPC, hThread, tid, 2);

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    return 0;
}
