#include <stdio.h>
#include <stdlib.h>
#include "dshlib.h"

/*
 * main:
 *   Calls exec_local_cmd_loop() and then prints exactly "cmdloopreturned0"
 *   (if rc == 0) to match the expected output.
 */
int main(void) {
    int rc = exec_local_cmd_loop();
    printf("cmdloopreturned%d", rc);
    return rc;
}
