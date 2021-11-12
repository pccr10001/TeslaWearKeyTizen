#ifndef _KEY_MANAGER_H_
#define _KEY_MANAGER_H_

#include "stdint.h"
#include "stdbool.h"
#include <dlog.h>

 bool get_key(char* data);
 bool save_key(char *data, int len);

#endif
