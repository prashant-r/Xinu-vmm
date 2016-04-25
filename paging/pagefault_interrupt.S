/* pagefault_interrupt.S */

	.text
error_code: .long 0
	.globl  pagefault_interrupt
	.globl  error_code

pagefault_interrupt:
    popl error_code /* pop the error code off the stack */
    pushfl
    cli                 /* disable all interrupts                   */
    pushal              /* save general purpose registers       */
    call pagefault_handler
    popal               /* restore general purpose registers    */
    popfl
    iret
