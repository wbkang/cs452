#pragma once

void kernel_init();
void handle_swi(register_set reg, int service_no);
