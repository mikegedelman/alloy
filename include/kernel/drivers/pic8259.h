#ifndef _KERNEL_PIC_8259_H
#define _KERNEL_PIC_8259_H

void pic1_eoi();
void pic2_eoi();
void pic_remap(int, int);
void pic_irq_clear_mask(uint8_t IRQline);

#endif