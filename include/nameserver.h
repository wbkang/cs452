#pragma once
#include <constants.h>
#include <string.h>

/*
 * The registered names have to be exactly of the length 2.
 * The valid characters for the names are the ASCII printable characters.
 * http://en.wikipedia.org/wiki/ASCII#ASCII_printable_characters
 * This gives us 95^2=9025 possibilities.
 */

#define ASCII_PRINTABLE_START 0x20
#define ASCII_PRINTABLE_END 0x7e
#define NUM_ASCII_PRINTABLE (ASCII_PRINTABLE_END - ASCII_PRINTABLE_START + 1)

#define NAMESERVER_REGISTERAS 0
#define NAMESERVER_WHOIS 1

#define NAMESERVER_ERROR_BADDATA -3
#define NAMESERVER_ERROR_NOTREGISTERED -4
#define NAMESERVER_ERROR_BADREQNO -5
#define NAMESERVER_ERROR_BADNAME -6

void nameserver();

inline int nameserver_registeras(char *name);

inline int nameserver_whois(char *name);
