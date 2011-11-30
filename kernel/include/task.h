#pragma once

#include <util.h>
#include <hardware.h>
#include <memory.h>

#define REG_FP 11
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

#define TASK_LIST_SIZE ((USER_MEM_END - USER_MEM_START) / STACK_SIZE)

typedef struct _tag_register_set {
	int spsr;
	int r[16]; // r0-r12, sp, lr, pc
} register_set;

typedef struct _tag_task_descriptor {
	int id;
	int parent_id;
	uint state;
	uint priority;
	register_set registers;
	memptr heap;
	struct _tag_task_descriptor *_prev; // sibling queue
	struct _tag_task_descriptor *_next;
	struct _tag_task_descriptor *_tail_child; // children queue
	struct _tag_task_descriptor *_head_child;
	int last_receiver;
} task_descriptor;

void td_init();
uint get_td_list_size();
int td_index(int tid);
int td_impossible(int tid);
void td_clear_siblings(task_descriptor *td);
void td_clear_children(task_descriptor *td);
int td_has_children(task_descriptor *td);
void td_push_child(task_descriptor *td, task_descriptor *child);
task_descriptor *td_shift_child(task_descriptor *td);
task_descriptor *td_new();
void td_free(task_descriptor *td);
task_descriptor *td_find(uint id);
void reginfo(register_set *reg);
void td_print_crash_dump();
