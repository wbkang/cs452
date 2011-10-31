#pragma once

int buffertask_new(int priority, int item_size);
int buffertask_put(int tid, void* item, int item_size);
int buffertask_get(int tid, void* item, int item_size);
