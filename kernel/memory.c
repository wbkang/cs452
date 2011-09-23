#include <memory.h>
#include <util.h>
#include <rawio.h>
#include <ts7200.h>

void pages_init(pages p, uint size, uint first_node, uint first_page) {
	p->head = (memptr) first_node;
	p->first_node = first_node;
	p->first_page = first_page;
	// initialize nodes
	uint addr = first_node + 4 * (size - 1); // bottom
	MEM(addr) = NULL;
	for (; addr > first_node; addr -= 4) {
		MEM(addr - 4) = addr;
	}
}

memptr pages_nextnode(memptr node) {
	return (memptr) (*node);
}

uint pages_nodeindex(pages p, memptr node) {
	return ((uint) node) - p->first_node;
}

uint pages_pageindex(pages p, memptr page) {
	return ((uint) page) - p->first_page;
}

memptr pages_node2page(pages p, memptr node) {
	return (memptr) (p->first_page + (pages_nodeindex(p, node) << PAGE_BSIZE));
}

memptr pages_page2node(pages p, memptr page) {
	return (memptr) (p->first_node + (pages_pageindex(p, page) >> PAGE_BSIZE));
}

memptr pages_get(pages p) {
	memptr head = p->head;
	if (head == NULL) return NULL;
	p->head = pages_nextnode(head);
	return pages_node2page(p, head);
}

void pages_put(pages p, memptr page) {
	memptr node = pages_page2node(p, page);
	*node = (uint) p->head;
	p->head = node;
}

void pages_test() {
	int size = 1024;
	struct _tag_pages _p;
	pages p = &_p;
	memptr node_table[size];
	pages_init(p, size, (uint) node_table, 0x300000);
	memptr test[size];
	int i;
	for (i = size - 1; i >= 0; --i) {
		test[i] = pages_get(p);
		bwprintf(COM2, "page %d: %x\n", i + 1, (uint) test[i]);
	}
	bwprintf(COM2, "head after remove %d: %x\n", size, (uint) p->head);
	for (i = size - 1; i >= 0; --i) {
		pages_put(p, test[size - 1 - i]);
		bwprintf(COM2, "head %d: %x\n", i + 1, (uint) p->head);
	}
}
