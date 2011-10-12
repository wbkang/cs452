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
#define TD_STATE_WAITING4RECEIVE 6
#define TD_STATE_WAITING4REPLY 7
#define TD_STATE_WAITING4EVENT 8

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

void td_init();

inline int td_index(int tid);

inline int td_impossible(int tid);

inline int td_list_empty(task_descriptor *td);

inline void td_list_close(task_descriptor *td);

inline void td_list_remove(task_descriptor *td);

inline void td_list_push(task_descriptor *head, task_descriptor *td);

task_descriptor *td_new();

void td_free(task_descriptor *td);

task_descriptor *td_find(uint id);

task_descriptor *td_pop(task_descriptor *head);

void reginfo(register_set *reg);
