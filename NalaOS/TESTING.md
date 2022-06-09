# TESTING for NalaOS

## Testing sample code from yalnix_framework/sample/test

### torture.c
We tested NalaOS using torture.c under the command ```./yalnix -x progs/torture -W``` with an additional modification of including the Reclaim Syscall. All functionality worked as expected.

### zeros.c, bigstack.c, forktest.c
All functionality worked as expected.

## Testing with custom test files
We included a range of custom test programs to test NalaOS.
- brk.c: tests brk by mallocing large amount of memory.
- exec1.c and exec2.c: Makes the exec syscall.
- fork.c: makes the fork syscall. 
- idle.c: NalaOS idle program.
- init.c: Initalization program for NalaOS.
- ipc_basic.c: Creates a pipe and writes and reads from seperate pipes.
- pid_test.c: Tests the PID syscall.
- pipe_basic.c: Creates a simple pipe. Tests reclaim. Write and read functionalituy.
- simul_ttywrite.c: Tests ttywrite by writing to different consols by two different processes
- spam_ttywrite.c: Spams ttywrite with a large buffer.
- stressful_pipes.c: Writes to pipes with a large buffer.
- wait_exit.c: Tests Wait syscall by forking.
- to_exec.c: Partner test file for wait_exit.c.
- trap_math.c: Tests trap math handler.
- trap_mem.c: Tests trap memory.
- ttyread_test.c: Tests ttyread by reading for console.
- ttywrite.c: Tests by writing to console.
- really_bad_calls.c: Makes many invalid syscalls e.g. NULL parameters to make sure we fail gracefully.

Refer to checkpoint writeups for more details on testing.
