extern "C" {
    /// kernel_start and kernel_end symbols allow us to figure out where our kernel is
    /// stored in physical memory, because we also know where it was originally loaded at
    pub static kernel_start: u32;
    pub static kernel_end: u32;
}
