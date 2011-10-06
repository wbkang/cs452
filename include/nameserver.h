#define NAMESERVER_PRIORITY 16
#define NAMESERVER_MAX_NAME_LEN 32
#define NAMESERVER_REQUEST_REGISTERAS 0
#define NAMESERVER_REQUEST_WHOIS 1

typedef struct _tag_nameserver_request {
	int n;
	char str[NAMESERVER_MAX_NAME_LEN];
} nameserver_request;

void nameserver();
