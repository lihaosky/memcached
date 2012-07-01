#ifndef MONOISIS_H
#define MONOISIS_H

#ifndef FALSE 
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* Initialize ISIS */
void isis_init(int nnum, int ssize, int mrank, bool is_verbose);

/* Start ISIS, create group etc. */
void isis_start(void);

/* Send command to other nodes */
int isis_send(char *cmd);
#endif