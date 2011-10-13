/*
 * Useful constants
 */
#define TRUE 1
#define FALSE 0
#define NULL 0
#define CRLF "\r\n"
#define ON  1
#define OFF 0

/*
 * Scheduler constants
 */
#define NUM_PRIORITY 32
#define MAX_PRIORITY (NUM_PRIORITY - 1)
#define MIN_PRIORITY 0

/*
 * Sever names
 */

#define NAME_TIMESERVER "00"

/*
 * Server priorities
 */
#define PRIORITY_IDLESERVER MIN_PRIORITY
#define PRIORITY_NAMESERVER MAX_PRIORITY
#define PRIORITY_TIMESERVER MAX_PRIORITY
#define PRIORITY_TIMENOTIFIER MAX_PRIORITY

/*
 * System call constants
 */
#define SYSCALL_CREATE 0
#define SYSCALL_MYTID 1
#define SYSCALL_MYPARENTTID 2
#define SYSCALL_PASS 3
#define SYSCALL_EXIT 4
#define SYSCALL_MALLOC 5
#define SYSCALL_SEND 6
#define SYSCALL_RECEIVE 7
#define SYSCALL_REPLY 8
#define SYSCALL_NAMESERVERTID 9
#define SYSCALL_AWAITEVENT 10
#define SYSCALL_EXITKERNEL 11

/*
 * Memory constants.
 */
// The start of the task memory region
// also look at the variables in orex.ld
#define USER_MEM_START	0x300000
// The end of the task memory region
#define USER_MEM_END	0x1900000
// size of user memory in bytes (64 KB)
#define STACK_SIZE 65536

// &_TextStart is the start of the .text
extern int _TextStart;
// &_TextStart is the end of the .text
extern int _TextEnd;

