	.globl vdi

	.text

vdi:
	MOVE.L 4(sp),D1
	MOVE.W #$73,D0
	TRAP   #2
    RTS
