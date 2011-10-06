#include <constants.h>

#define NAMESERVER_PRIORITY (MAX_PRIORITY - 1)
#define NAMESERVER_MAX_NAME_LEN 4
#define NAMESERVER_REQUEST_REGISTERAS 0
#define NAMESERVER_REQUEST_WHOIS 1

typedef struct _tag_nameserver_request {
	int n;
	char str[NAMESERVER_MAX_NAME_LEN];
} nameserver_request;

int get_nameserver();

void nameserver();
