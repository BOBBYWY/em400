
	lw	r0, ?C
	lw	r1, 0b1111111111111111
	ngc	r1

	lw	r2, 0b0000000000000001
	ngc	r2

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0

; XPCT r1 : 0b0000000000000001
; XPCT r2 : 0b1111111111111110
