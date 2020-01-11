.global sys_flush_and_clean_dcache
.global sysSetupCP15


sys_flush_and_clean_dcache:
tci_loop:
	MRC p15, #0, r15, c7, c14, #3
	BNE tci_loop
	BX      lr


sysSetupCP15:
	MOV     r1, r0
	MCR     p15, #0, r1, c2, c0, #0
	MOV     r1, #0x40000000 
	MCR     p15, #0, r1, c3, c0, #0 
	MRC     p15, #0, r1, c1, c0, #0
	ORR     r1, r1, #0x1000
	ORR     r1, r1, #0x5
	MCR     p15, #0, r1, c1, c0, #0
	BX      lr

