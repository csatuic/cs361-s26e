/* global.h (or at the top of spy.c) */
typedef struct {
    char     *module_name; /* dynamically allocated, caller must free */
    char     *name;      /* dynamically allocated, caller must free */
    uintptr_t address;
    size_t    size;      /* 0 if unknown */
} GlobalVar;

void print_disassembly(pid_t pid, uintptr_t rip);
void print_function_name(pid_t pid, uintptr_t addr);
const char* get_function_name(pid_t pid, uintptr_t addr, char* buf, size_t buflen);
ssize_t read_tracee_memory(pid_t pid, uintptr_t addr, void *buf, size_t len);
int get_globals(pid_t pid, GlobalVar **out_list, int *out_count);
void free_globals(GlobalVar *list, int count);