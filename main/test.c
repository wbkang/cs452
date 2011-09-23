#include <test.h>
#include <util.h>
#include <hardware.h>
#include <memory.h>
#include <rawio.h>


static void test_pages()
{
	struct _tag_pages _p;
	pages p = &_p;
	memptr node_table[8];
	pages_init(p, 8, (uint) node_table, 0x300000);
	memptr test[8];
	int i;
	for (i = 7; i >= 0; --i) {
		test[i] = pages_get(p);
		bwprintf(COM2, "page %d: %x\n", i + 1, (uint) test[i]);
	}
	bwprintf(COM2, "head after remove 5: %x\n", (uint) p->head);

	ASSERT(p->head == 0, "there should be no more pages to allocate.");

	for (i = 7; i >= 0; --i) {
		pages_put(p, test[7 - i]);
		bwprintf(COM2, "head %d: %x\n", i + 1, (uint) p->head);
	}
}

void test_run()
{
	bwprintf(COM2, ">>>>>>>>>>>>>>>>>>>>TEST START\n");
	test_pages();
	bwprintf(COM2, ">>>>>>>>>>>>>>>>>>>>TEST END\n");
}
