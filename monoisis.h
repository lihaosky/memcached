#ifndef MONOISIS_H
#define MONOISIS_H

#ifndef false 
#define false 0
#endif

#ifndef true
#define true 1
#endif

/* Initialize ISIS */
void isis_init(int nnum, int ssize, int mrank);

/* Start ISIS, create group etc. */
void isis_start(void);

/* Send command to other nodes */
int isis_send(char *cmd);
#endif