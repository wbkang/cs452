#pragma once

#include <util.h>

#define PAGE_SIZE 4096
#define PAGE_BSIZE 12

typedef struct _tag_pages {
		memptr head;
		uint node_table;
		uint first_page;
}*pages;

void pages_init(pages p, uint maxlen, uint addr, uint offset);

memptr pages_next(memptr node);

memptr pages_n2p(pages p, memptr node);

memptr pages_p2n(pages p, memptr page);

memptr pages_get(pages p);

void pages_put(pages p, memptr page);
