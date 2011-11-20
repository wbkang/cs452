#pragma once

void buffertask();

int buffertask_new(char name[], int priority, int item_size);
int buffertask_put(int tid, void* item, int item_size);
int buffertask_get(int tid, void* item, int item_size);
