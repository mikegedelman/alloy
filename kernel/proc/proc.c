#include <kernel/all.h>

// // 1. Create a new page table for the process, switch to it and execute the process inside that
// typedef struct __attribute__((aligned(4096))) {
//     uint32_t entries[1024];
// } PageDirectory;

enum ProcessState {
    NONE = 0,
    READY,
    RUNNING
};

typedef struct {
    pid_t pid;
    uint32_t base_mem_addr_phys;

    ProcessCPUState cpu_state;
    int state;
} Process;

static Process processes[256];
static pid_t max_pid;
static pid_t cur_pid;


void setup_page_table(Process *proc) {
    AllocResult phys_alloc_result = alloc_contiguous_4mb();
    if (phys_alloc_result.status == ALLOC_NONE_AVAILABLE) {
        panic("Unable to alloc space for process.\n");
    }
    printf("physical alloc result for process: %x\n", phys_alloc_result.addr);
    // uint32_t flags = PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE;
    // virtualmem_map(phys_alloc_result.addr, 0x08000000, flags);
    proc->base_mem_addr_phys = phys_alloc_result.addr;
    // virtualmem_map_kernel(proc->page_directory.entries);
    // set_cr3((void *) (&proc->page_directory.entries - BASE_VIRTUAL_ADDRESS));
}

pid_t schedule_process(uint8_t *buf) { // , size_t buf_len) {
    if (max_pid >= 256) {
        panic("Unable to spawn process - ran out of pids");
    }
    // Use 0 as an indicator that nothing is running.
    if (cur_pid == 0) {
        cur_pid++;
    }
    Process *proc = &processes[max_pid];
    proc->pid = max_pid;
    max_pid++;

    setup_page_table(proc);
    virtualmem_map(proc->base_mem_addr_phys, 0x8000000, PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE | PAGEDIR_USER);
    proc->cpu_state.eip = load_elf(buf);
    proc->state = READY;

    return proc->pid;
}

extern void restore_process(uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t ebx, uint32_t esp, uint32_t ebp, uint32_t esi, uint32_t edi, uint32_t eip);

void next_process(ProcessCPUState *cpu_state) {
    pid_t next_pid = 1;
    if (cur_pid > 0) {
        Process *old_proc = &processes[cur_pid];
        old_proc->cpu_state = *cpu_state;
        next_pid = (cur_pid + 1) % max_pid;
    }

    Process *new_proc = &processes[next_pid];
    if (new_proc->state != READY && new_proc->state != RUNNING) {
        return;
    }
    printf("Moving from PID %u to %u\n", cur_pid, next_pid);

    // uint32_t flags = PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE | PAGEDIR_USER;
    printf("Mapping 0x8000000 to %x\n", new_proc->base_mem_addr_phys);
    virtualmem_map(new_proc->base_mem_addr_phys, 0x8000000, PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE | PAGEDIR_USER);

    // new_proc->state = RUNNING;
    // cur_pid = new_proc->pid;
    // new_proc->cpu_state.eax = 1;
    // new_proc->cpu_state.ebx = 2;
    // new_proc->cpu_state.ecx = 3;
    // new_proc->cpu_state.edx = 4;
    new_proc->cpu_state.esp = (0x8400000 - 4096);
    // new_proc->cpu_state.ebp = 6;
    // new_proc->cpu_state.esi = 7;
    // new_proc->cpu_state.edi = 8;
    // new_proc->cpu_state.eip = 9;
    restore_process(
        new_proc->cpu_state.eax,
        new_proc->cpu_state.ebx,
        new_proc->cpu_state.ecx,
        new_proc->cpu_state.edx,
        new_proc->cpu_state.esp,
        new_proc->cpu_state.ebp,
        new_proc->cpu_state.esi,
        new_proc->cpu_state.edi,
        new_proc->cpu_state.eip
    );
}
