#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/types.h>
#include <capstone/capstone.h>
#include <elfutils/libdwfl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

ssize_t read_tracee_memory(pid_t pid, uintptr_t addr, void *buf, size_t len) {
    size_t i = 0;
    while (i < len) {
        errno = 0;
        long word = ptrace(PTRACE_PEEKTEXT, pid, (void *)(addr + i), NULL);
        if (errno != 0) return -1;
        size_t to_copy = (len - i < sizeof(long)) ? (len - i) : sizeof(long);
        memcpy((char *)buf + i, &word, to_copy);
        i += to_copy;
    }
    return (ssize_t)len;
}
void print_function_name(pid_t pid, uintptr_t addr) {
    Dwfl *dwfl = NULL;
    Dwfl_Module *module = NULL;
    const char *func_name = NULL;
    const char *module_name = "unknown";

    static Dwfl_Callbacks callbacks = {
        .find_elf        = dwfl_linux_proc_find_elf,
        .find_debuginfo  = dwfl_standard_find_debuginfo
    };

    dwfl = dwfl_begin(&callbacks);
    if (!dwfl)
        goto out;

    /* Attach to the live process and report all loaded modules */
    if (dwfl_linux_proc_attach(dwfl, pid, true) != 0 ||
        dwfl_linux_proc_report(dwfl, pid) != 0) {
        goto out;
    }

    module = dwfl_addrmodule(dwfl, addr);
    if (module) {
        func_name = dwfl_module_addrname(module, addr);
        dwfl_module_info(module, NULL, NULL, NULL, NULL, NULL, &module_name, NULL);
    }

    if (func_name) {
        printf("Current function: %s (0x%lx) in %s\n",
               func_name, (unsigned long)addr, module_name);
    } else {
        printf("Current function: <unknown> (0x%lx) in %s\n",
               (unsigned long)addr, module_name);
    }

out:
    if (dwfl)
        dwfl_end(dwfl);
}

void print_disassembly(pid_t pid, uintptr_t rip) {
    csh handle;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        fprintf(stderr, "Capstone init failed\n");
        return;
    }
    cs_option(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL);

    const size_t window = 256;
    uintptr_t start_addr = (rip > window / 2) ? rip - window / 2 : 0;
    uint8_t code[256];
    memset(code, 0, sizeof(code));

    if (read_tracee_memory(pid, start_addr, code, window) < 0) {
        fprintf(stderr, "Failed to read tracee memory\n");
        cs_close(&handle);
        return;
    }

    cs_insn *insn;
    size_t count = cs_disasm(handle, code, window, start_addr, 0, &insn);
    if (count == 0) {
        fprintf(stderr, "Disassembly failed\n");
        cs_close(&handle);
        return;
    }

    printf("\nDisassembly around 0x%lx:\n", (unsigned long)rip);
    int printed = 0;
    for (size_t i = 0; i < count && printed < 25; ++i) {
        if (insn[i].address > rip + 96) break;
        if (insn[i].address + insn[i].size < rip - 80) continue;

        printf("0x%016lx: %-30s %s\n",
               (unsigned long)insn[i].address,
               insn[i].mnemonic, insn[i].op_str);
        printed++;
    }

    cs_free(insn, count);
    cs_close(&handle);
}