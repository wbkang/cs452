#include <test.h>
#include <util.h>
#include <hardware.h>
#include <memory.h>
#include <rawio.h>


static void test_pages() {
	int size = 1024;
	struct _tag_pages _p;
	pages p = &_p;
	memptr node_table[size];
	pages_init(p, size, (uint) node_table, 0x300000);
	memptr test[size];
	int i;
	for (i = size - 1; i >= 0; --i) {
		test[i] = pages_get(p);
		ASSERT(test[i], "This should NOT be null. we have more pages.");
		bwprintf(COM2, "page %d: %x\n", i + 1, (uint) test[i]);
	}

	ASSERT(p->head == NULL, "we have used up all the pages. should return 0.");

	bwprintf(COM2, "head after remove %d: %x\n", size, (uint) p->head);
	for (i = size - 1; i >= 0; --i) {
		pages_put(p, test[size - 1 - i]);
		bwprintf(COM2, "head %d: %x\n", i + 1, (uint) p->head);
	}

	ASSERT(p->head == (memptr) p->first_node, "we returned the last page last.");
}

void test_run()
{
	bwprintf(COM2, ">>>>>>>>>>>>>>>>>>>>TEST START\n");
	test_pages();
	bwprintf(COM2, ">>>>>>>>>>>>>>>>>>>>TEST END\n");
}
