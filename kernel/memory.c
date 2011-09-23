#include <memory.h>

void pages_init(pages p, uint size, uint node_table, uint first_page) {
	p->head = (memptr) node_table;
	p->node_table = node_table;
	p->first_page = first_page;
	uint addr = node_table + 4 * (size - 1); // bottom
	MEM(addr) = NULL;
	while (addr > node_table) {
		addr -= 4;
		MEM(addr) = addr + 4;
	}
}

memptr pages_next(memptr node) {
	return (memptr) (*node);
}

memptr pages_n2p(pages p, memptr node) {
	return (memptr) (p->first_page + (((uint) node) - p->node_table) * PAGE_SIZE);
}

memptr pages_p2n(pages p, memptr page) {
	return (memptr) (p->node_table + (((uint) page) - p->first_page) / PAGE_SIZE);
}

memptr pages_get(pages p) { // allocate a page
	memptr head = p->head;
	if (head == NULL) return NULL; // no more pages to give
	p->head = pages_next(head);
	return pages_n2p(p, head);
}

void pages_put(pages p, memptr page) {
	memptr node = pages_p2n(p, page);
	*node = (uint) p->head;
	p->head = node;
}
