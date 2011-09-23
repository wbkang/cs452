#pragma once

#include <util.h>

#define PAGE_SIZE 4096
#define PAGE_BSIZE 12

typedef struct _tag_pages {
		memptr head;
		uint first_node;
		uint first_page;
}*pages;

void pages_init(pages p, uint size, uint first_node, uint first_page);
memptr pages_nextnode(memptr node);
uint pages_nodeindex(pages p, memptr node);
uint pages_pageindex(pages p, memptr page);
memptr pages_node2page(pages p, memptr node);
memptr pages_page2node(pages p, memptr page);
memptr pages_get(pages p);
void pages_put(pages p, memptr page);
void pages_test();
