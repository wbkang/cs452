#include <constants.h>

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

#define IS_ASCII_PRINTABLE(x) (0x20 <= (x) && (x) <= 0x7e)
#define NAMESERVER_VALID_NAME(x) (IS_ASCII_PRINTABLE((x)[0]) && IS_ASCII_PRINTABLE((x)[1]))

#define MAKE_NAMESERVER_REQ(reqno, name) ((reqno) | ((int)(name)[0]) << 24 | ((int)(name)[1]) << 16)
#define GET_NAMESERVER_REQNUM(req) ((req) & 0xffff)

/*
 * These are request numbers for the name server.
 */
#define NAMESERVER_REQUEST_REGISTERAS 0
#define NAMESERVER_REQUEST_WHOIS 1

void nameserver();
