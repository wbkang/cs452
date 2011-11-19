#pragma once

// appease stack trace gods
void publisher();

// api
int publisher_new(char name[], int priority, int item_size);
int publisher_pub(int tid, void* item, int item_size);
int publisher_sub(int tid_publisher, int tid);
