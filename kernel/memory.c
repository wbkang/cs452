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
