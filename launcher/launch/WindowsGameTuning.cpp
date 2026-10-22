// SPDX-License-Identifier: GPL-3.0-only
#include "WindowsGameTuning.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <avrt.h>
#include <mmsystem.h>
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "winmm.lib")
#endif

namespace WindowsGameTuning {
void apply(qint64 processId)
{
#ifdef Q_OS_WIN
    if (processId <= 0) return;
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_SET_INFORMATION, FALSE,
                                 static_cast<DWORD>(processId));
    if (!process) return;

    // ABOVE_NORMAL is deliberately used instead of HIGH: it improves responsiveness without starving audio/input.
    SetPriorityClass(process, ABOVE_NORMAL_PRIORITY_CLASS);
    SetProcessPriorityBoost(process, FALSE);

    // 1 ms is supported on current Windows versions and is released automatically when the process exits.
    timeBeginPeriod(1);
    CloseHandle(process);
#else
    Q_UNUSED(processId);
#endif
}
}
