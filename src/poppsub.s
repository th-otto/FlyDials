; Fliegende Dialoge V0.1
; Unterroutinen fÅr Popp.C

	.globl	_PoppXMouse,_PoppYMouse
	.globl	_PoppBt,_PoppButton
	.globl	_PoppMot

	.text

_PoppMot:
	move.w	d0,_PoppXMouse
	move.w	d1,_PoppYMouse
	rts

_PoppBt:
	move.w	d0,_PoppButton
	rts



	
	.end
	
	