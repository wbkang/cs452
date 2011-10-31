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
#define MIN_PRIORITY 1
#define MAX_PRIORITY (NUM_PRIORITY - 1)

/*
 * Sever names
 */

#define NAME_TIMESERVER "00"
#define NAME_IOSERVER_COM1 "01"
#define NAME_IOSERVER_COM2 "02"
#define NAME_TRAINCMDBUFFER "03"

/*
 * Server priorities
 */
#define PRIORITY_IDLESERVER (MIN_PRIORITY - 1)
#define PRIORITY_NAMESERVER MAX_PRIORITY
#define PRIORITY_TIMESERVER MAX_PRIORITY
#define PRIORITY_TIMENOTIFIER MAX_PRIORITY
#define PRIORITY_IOSERVER MAX_PRIORITY
#define PRIORITY_SENSORNOTIFIER (MAX_PRIORITY -1)
#define PRIORITY_IONOTIFIER (MAX_PRIORITY - 1)
#define PRIORITY_TRAINCMDBUFFER (MAX_PRIORITY - 2)
#define PRIORITY_TRAINCMDRUNNER (MAX_PRIORITY - 3)

/*
 * Memory constants.
 */
// size of user memory in bytes (64 KB) (MUST BE A POWER OF TWO)
#define STACK_SIZE 65536

// &_TextStart is the start of the .text
extern int _TextStart;
// &_TextStart is the end of the .text
extern int _TextEnd;

extern int _USER_MEM_START;
extern int _USER_MEM_END;
