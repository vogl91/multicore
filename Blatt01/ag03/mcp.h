#ifndef MCP_H
#define MCP_H

#ifdef __cplusplus
extern "C" {
#endif

extern int num_elements;
extern int num_threads;

void mcp_init(int argc, char **argv);

int get_num_threads();
int get_num_elements();

void time_start();
void time_stop();
double time_get();
void time_print();

#ifdef __cplusplus
}
#endif

#endif

