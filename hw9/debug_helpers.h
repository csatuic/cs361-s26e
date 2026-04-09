void print_disassembly(pid_t pid, uintptr_t rip);
void print_function_name(pid_t pid, uintptr_t addr);
ssize_t read_tracee_memory(pid_t pid, uintptr_t addr, void *buf, size_t len);
