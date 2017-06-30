/*
    Crash - Process Instrumentor
    Copyright (C) 2005 Pedram Amini <pamini@idefense.com,pedram.amini@gmail.com>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place, Suite 330, Boston, MA 02111-1307 USA

    Return Codes:
        -1   - An error occured during the process instrumentation.
         0   - Process exited normally.
         1   - Process generated exception.

*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "libdasm.h"

int main (int argc, char **argv)
{
    PROCESS_INFORMATION pi;
    INSTRUCTION         inst;
    STARTUPINFO         si;
    DEBUG_EVENT         dbg;
    CONTEXT             context;
    HANDLE              thread;
    HANDLE              process;
    DWORD               wait_time;
    DWORD               start_time;
    BOOL                ret;
	BOOL				exception;
//	BOOL				continueDebug;
    u_char              inst_buf[32];
    char                inst_string[256];
    char                command_line[32768];
    int                 i;

    //
    // variable initialization.
    //

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    memset(command_line, 0, sizeof(command_line));
    memset(inst_buf,     0, sizeof(inst_buf));

    start_time = GetTickCount();
	exception  = FALSE;

    //
    // command line processing.
    //

    // minimum arg check.
    if (argc < 4)
    {
        fprintf(stderr, "[!] Usage: crash <path to app> <milliseconds> <arg1> [arg2 arg3 ... argn]\n\n");
        return -1;
    }

    // convert wait time from string to integer.
    if ((wait_time = atoi(argv[2])) == 0)
    {
        fprintf(stderr, "[!] Milliseconds argument unrecognized: %s\n\n", argv[2]);
        return -1;
    }

    // create the command line string for the call to CreateProcess().
    strcpy(command_line, argv[1]);

    for (i = 3; i < argc; i++)
    {
        strcat(command_line, " ");
        strcat(command_line, argv[i]);
    }

    //
    // launch the target process.
    //

    ret = CreateProcess(NULL,       // target file name.
        command_line,               // command line options.
        NULL,                       // process attributes.
        NULL,                       // thread attributes.
        FALSE,                      // handles are not inherited.
        DEBUG_PROCESS,              // debug the target process and all spawned children.
        NULL,                       // use our current environment.
        NULL,                       // use our current working directory.
        &si,                        // pointer to STARTUPINFO structure.
        &pi);                       // pointer to PROCESS_INFORMATION structure.

    printf("[*] %s\n", GetCommandLine());  //Print the command line
	
	if (!ret)
    {
        fprintf(stderr, "[!] CreateProcess() failed: %d\n\n", GetLastError());
        return -1;
    }

    //
    // watch for an exception.
    //

    while (GetTickCount() - start_time < wait_time)
    {
        if (WaitForDebugEvent(&dbg, 100))
        {
            // we are only interested in debug events.
            if (dbg.dwDebugEventCode != EXCEPTION_DEBUG_EVENT)
            {
                ContinueDebugEvent(dbg.dwProcessId, dbg.dwThreadId, DBG_CONTINUE);
                continue;
            }

            // get a handle to the offending thread.
            if ((thread = OpenThread(THREAD_ALL_ACCESS, FALSE, dbg.dwThreadId)) == NULL)
            {
                fprintf(stderr, "[!] OpenThread() failed: %d\n\n", GetLastError());
                return -1;
            }

            // get the context of the offending thread.
            context.ContextFlags = CONTEXT_FULL;

            if (GetThreadContext(thread, &context) == 0)
            {
                fprintf(stderr, "[!] GetThreadContext() failed: %d\n\n", GetLastError());
                return -1;
            }

           // examine the exception code.
            switch (dbg.u.Exception.ExceptionRecord.ExceptionCode)
            {
                case EXCEPTION_ACCESS_VIOLATION:
					exception = TRUE;
					printf("[*] Access Violation\n");
					break;
				case EXCEPTION_INT_DIVIDE_BY_ZERO:
					exception = TRUE;
					printf("[*] Divide by Zero\n");
					break;
                case EXCEPTION_STACK_OVERFLOW:
                    exception = TRUE;
					printf("[*] Stack Overflow\n");
					break;
                default:
					//printf("[*] Unknown Exception (%08x):\n", dbg.u.Exception.ExceptionRecord.ExceptionCode);
                    ContinueDebugEvent(dbg.dwProcessId, dbg.dwThreadId, DBG_CONTINUE);
            }

			// if an exception occured, print more information.
			if (exception)
			{
				// open a handle to the target process.
				if ((process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dbg.dwProcessId)) == NULL)
				{
					fprintf(stderr, "[!] OpenProcess() failed: %d\n\n", GetLastError());
					return -1;
				}

				// grab some memory at EIP for disassembly.
				ReadProcessMemory(process, (void *)context.Eip, &inst_buf, 32, NULL);

				// decode the instruction into a string.
				get_instruction(&inst, inst_buf, MODE_32);
				get_instruction_string(&inst, FORMAT_INTEL, 0, inst_string, sizeof(inst_string));

				// print the exception to screen.
				printf("[*] Exception caught at %08x %s\n", context.Eip, inst_string);
				printf("[*] EAX:%08x EBX:%08x ECX:%08x EDX:%08x\n", context.Eax, context.Ebx, context.Ecx, context.Edx);
				printf("[*] ESI:%08x EDI:%08x ESP:%08x EBP:%08x\n\n", context.Esi, context.Edi, context.Esp, context.Ebp);
				
				return 1;
			}

        }
	}
    //
    // done.
    //

    printf("[*] Process terminated normally.\n\n");
    return 0;
}