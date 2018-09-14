#ifndef PROCESSOR_STARTUP_H_28042018
#define PROCESSOR_STARTUP_H_28042018

#include "types.h"

// starts all AP processors
void startup_all_AP();

// setup common stack for userspace threads (used when switching from ring 3 to ring 0) and return its top
virtual_addr_t setup_processor_common_stack(uint8_t cpu_id);


#endif