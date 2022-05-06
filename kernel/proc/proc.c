#include <kernel/all.h>

// // 1. Create a new page table for the process, switch to it and execute the process inside that
// typedef struct __attribute__((aligned(4096))) {
//     uint32_t entries[1024];
// } PageDirectory;

enum ProcessState {
    NONE = 0,
    RUNNING,
    BLOCKED,
    DONE
};

typedef struct {
    int gfd;
    size_t waiting_bytes;
} FileDescriptor;

typedef struct {
    pid_t pid;
    uint32_t base_mem_addr_phys;
    uint32_t wait_val;

    ProcessCPUState cpu_state;
    int state;
    int blocked_fd;
    FileDescriptor fds[4];

    uint32_t userspace_read_ptr;
    int read_bytes;
} Process;

static Process processes[256];
static pid_t max_pid;
static pid_t cur_pid;
static bool ready_for_scheduling = 0;


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

    Process *proc = &processes[max_pid];
    proc->pid = max_pid;
    max_pid++;

    setup_page_table(proc);
    printf("Mapping 0x8000000 to %x\n", proc->base_mem_addr_phys);
    virtualmem_map(proc->base_mem_addr_phys, 0x8000000, PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE | PAGEDIR_USER);
    proc->cpu_state.eip = load_elf(buf);
    proc->cpu_state.esp = (0x8400000 - 4096);
    proc->state = RUNNING;
    proc->fds[0].gfd = 0;
    proc->fds[1].gfd = 1;
    proc->fds[2].gfd = -1;
    proc->fds[3].gfd = -1;
    proc->blocked_fd = -1;

    return proc->pid;
}

void begin_scheduling() {
    ready_for_scheduling = true;
    max_pid++;
}

extern void restore_process(uint32_t eax,
    uint32_t ecx,
    uint32_t edx,
    uint32_t ebx,
    uint32_t esp,
    uint32_t ebp,
    uint32_t esi,
    uint32_t edi,
    uint32_t eip
);

void next_process(pid_t next_pid) {
    printf("Restoring pid: %d\n", next_pid);
    Process *new_proc = &processes[next_pid];
    if (new_proc->state != RUNNING) {
        puts("Proc is not running. exiting\n");
        return;
    }

    // uint32_t flags = PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE | PAGEDIR_USER;
    printf("Mapping 0x8000000 to %x\n", new_proc->base_mem_addr_phys);
    virtualmem_map(new_proc->base_mem_addr_phys, 0x8000000, PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE | PAGEDIR_USER);

    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    save_kernel_stack(esp);

    cur_pid = next_pid;

    // Must send EOI /and/ the interrupt flag must be set (EFLAGS).
    // Since we're returning from an interrupt handler in a non-normal
    // way, we need to manually set it so that we still get hardware
    // interrupts while the process is running.
    sti();
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

void read_and_resume(Process *proc) {

}

bool should_resume(Process *proc) {
    if (proc->state == NONE || proc->state == DONE) return false;
    if (proc->state == RUNNING) return true;
    if (proc->state == BLOCKED) {
        if (proc->blocked_fd < 0) return false;

        int gfd_idx = proc->fds[proc->blocked_fd].gfd;
        GlobalFileDescriptor *gfd = gft_get(gfd_idx);
        if (gfd == NULL) {
            printf("Warning: pid %d was blocked on an invalid gfd %d (proc fd: %d)", proc->pid, gfd_idx, proc->blocked_fd);
        }
        if (gfd->cache_dirty) {
            // read and resume
            virtualmem_map(proc->base_mem_addr_phys, 0x8000000, PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE | PAGEDIR_USER);
            memcpy((void*) proc->userspace_read_ptr, gfd->cache, proc->read_bytes);
            gfd->cache_dirty = false;
            cur_pid = proc->pid;
            proc->cpu_state.eax = proc->read_bytes;
            sti();
            restore_process(
                proc->cpu_state.eax,
                proc->cpu_state.ebx,
                proc->cpu_state.ecx,
                proc->cpu_state.edx,
                proc->cpu_state.esp,
                proc->cpu_state.ebp,
                proc->cpu_state.esi,
                proc->cpu_state.edi,
                proc->cpu_state.eip
            );
            // should be unreachable
        }
    }

    return false;
}

void timer_schedule(ProcessCPUState *cpu_state) {
    if (!ready_for_scheduling) {
        return;
    }

    if (cpu_state != NULL && cur_pid > 0) {
        processes[cur_pid].cpu_state = *cpu_state;
    }

    pid_t next_pid = 0;
    for (pid_t i = cur_pid + 1; i != cur_pid; i = (i + 1) % max_pid) {
        if (i == 0) continue;
        if (should_resume(&processes[i])) {
            next_pid = i;
            break;
        }
    }
    // Found a new process to go execute
    if (next_pid > 0) {
        next_process(next_pid);
        return;
    }
    // Try to swap back to cur_pid
    Process *cur_proc = &processes[cur_pid];
    if (cur_pid > 0 && cur_proc->state == RUNNING) {
        printf("Continuing pid: %d\n", next_pid);
        sti();
        restore_process(
            cur_proc->cpu_state.eax,
            cur_proc->cpu_state.ebx,
            cur_proc->cpu_state.ecx,
            cur_proc->cpu_state.edx,
            cur_proc->cpu_state.esp,
            cur_proc->cpu_state.ebp,
            cur_proc->cpu_state.esi,
            cur_proc->cpu_state.edi,
            cur_proc->cpu_state.eip
        );
    }

    puts("Nothing to do atm - just idle\n");
    cur_pid = 0;
    sti();
    hlt();
}


void block_process(ProcessCPUState *cpu_state, int fd, uint32_t read_ptr, int read_bytes) {
    Process *proc = &processes[cur_pid];
    proc->cpu_state = *cpu_state;
    proc->state = BLOCKED;
    proc->blocked_fd = fd;
    proc->userspace_read_ptr = read_ptr;
    proc->read_bytes = read_bytes;

    printf("Blocked process %d\n", cur_pid);
    timer_schedule(cpu_state);
}

void exit_process() {
    Process *proc = &processes[cur_pid];
    proc->state = DONE;
    printf("Finished process %d\n", cur_pid);
    timer_schedule(NULL);
}

int proc_write(int fd, char *buf, int len) {
    if (fd > 3) return -1;
    Process *proc = &processes[cur_pid];
    int gfd = proc->fds[fd].gfd;
    if (gfd >= 0) {
        return gft_write(gfd, buf, len);
    }
}
