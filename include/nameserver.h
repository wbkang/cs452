#include <constants.h>
#include <string.h>

/*
 * The priority in which the name server runs.
 */
#define NAMESERVER_PRIORITY (MAX_PRIORITY - 1)

/*
 * The registered names has to be exactly of the length 2.
 * The valid characters for the names are the ASCII printable characters.
 * http://en.wikipedia.org/wiki/ASCII#ASCII_printable_characters
 * This gives us 95^2=9025 possibilities.
 */
#define NAMESERVER_NAME_LEN 2

#define ASCII_PRINTABLE_START 0x20
#define ASCII_PRINTABLE_END 0x7e
#define NUM_ASCII_PRINTABLE (ASCII_PRINTABLE_END - ASCII_PRINTABLE_START + 1)
#define IS_ASCII_PRINTABLE(x) (ASCII_PRINTABLE_START <= (x) && (x) <= ASCII_PRINTABLE_END)
#define NAMESERVER_VALID_NAME(x) ((x) && strlen(x) == NAMESERVER_NAME_LEN && IS_ASCII_PRINTABLE((x)[0]) && IS_ASCII_PRINTABLE((x)[1]))

#define NAMESERVER_REQ(reqno, name) (((int)(name)[0]) << 24 | ((int)(name)[1]) << 16) | (reqno)
#define NAMESERVER_GET_REQNO(req) ((req) & 0xffff)
#define NAMESERVER_GET_NAME(req) ((((req >> 24) & 0xff) - ASCII_PRINTABLE_START) * NUM_ASCII_PRINTABLE) + (((req >> 16) & 0xff) - ASCII_PRINTABLE_START);

// name server request numbers
#define NAMESERVER_REQUEST_REGISTERAS 0
#define NAMESERVER_REQUEST_WHOIS 1

void nameserver();
