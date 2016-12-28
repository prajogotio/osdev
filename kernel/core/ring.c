#include "ring.h"


void RingEnterUserMode() {
  __asm__(
    "cli\n\t"               
    // Preparing for IRET. Data selector for user mode is 0x20, and the
    // last 3 bits are RPL for ring 3, hence 0x23. All segments must be
    // set to this value.
    "mov $0x23, %ax\n\t"
    "mov %ax, %ds\n\t"
    "mov %ax, %es\n\t"
    "mov %ax, %fs\n\t"
    "mov %ax, %gs\n\t"      
    
    // Requirement for IRET
    "push $0x23\n\t"        // SS
    "push esp\n\t"          // ESP
    "pushf\n\t"             // EFLAGS
    "push 0x1b\n\t"         // CS (user mode code selector = 3 * 8 = 0x18,
                            // lower 3 bits = 011 --> 0x1b)
                            // Hence we are requesting to enter to protection
                            // level ring 3
    "lea (a), %eax\n\t"     // Getting EIP to be used by IRET
    "push eax\n\t"          // This means, when IRET is called, 0x1b:a will be
    "iret\n\t"              // called.
  "a:\n\t"
    "add esp, 4\n\t"        // Fix stack
  );
  for(;;);
}