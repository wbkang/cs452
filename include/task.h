#pragma once

#include <util.h>
#include <hardware.h>
#include <memory.h>

#define REG_SP 13
#define REG_LR 14
#define REG_PC 15

#define TD_STATE_FREE 0
#define TD_STATE_NEW 1
#define TD_STATE_READY 2
#define TD_STATE_RUNNING 3
#define TD_STATE_RETIRED 4
#define TD_STATE_WAITING4SEND 5
#define TD_STATE_WAITING4RECIEVE 6
#define TD_STATE_WAITING4REPLY 7

// this is calculated at compile time.
#define TASK_LIST_SIZE ((USER_MEM_END - USER_MEM_START) / STACK_SIZE)

typedef struct _tag_register_set {
		int spsr;
		int r[16]; // r0-r12, sp, lr, pc
} register_set;

typedef struct _tag_task_descriptor {
		int id;
		uint state;
		uint priority;
		int parent_id;
		register_set registers;
		memptr heap_base;
		memptr heap;
		struct _tag_task_descriptor *_prev;
		struct _tag_task_descriptor *_next;
} task_descriptor;

#define TD_INDEX(tid) ((tid) & 0xFFFF)

#define TD_IMPOSSIBLE(tid) (tid < 0 || (TD_INDEX(tid) >= TASK_LIST_SIZE))

#define TD_CLOSE(td) { \
	(td)->_prev = (td); \
	(td)->_next = (td); \
}

#define TD_REMOVE(td) { \
	(td)->_prev->_next = (td)->_next; \
	(td)->_next->_prev = (td)->_prev; \
	TD_CLOSE(td); \
}

#define TD_PUSH(head, td) { \
	(td)->_prev = (head); \
	(td)->_next = (head)->_next; \
	(td)->_next->_prev = (td); \
	(head)->_next = (td); \
}

#define TD_PEEK(td) ((td)->_prev)

#define TD_EMPTYLIST(td) (TD_PEEK(td) == td)

void td_init();

task_descriptor *td_new();

void td_free(task_descriptor *td);

task_descriptor *td_find(uint id);

task_descriptor *td_pop(task_descriptor *head);

void reginfo(register_set *reg);
