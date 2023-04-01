	.global asmfunc
	.global decode_display
	; FB to spew
	.global current_framebuffer

; int asmfunc(int a) {
; 	return a + a;
; }
asmfunc:
	add R12, R12
	ret


decode_display:
	; R4: loop maximum
	; R5: loop counter
	mov #0, R5
	; R6: temp
	; R6: temp
	; R12: constant 0x3

dd_loop:
		mov current_framebuffer(R5), R6 ; load 8 pixels to register (3? fram? cycles) (4 w/ one wait state?)
		mov #4, R8   ; store loop counter for loop_inner (2 cycles)
		mov R6, R7   ; backup R7 (1 cycles)
dd_loop_inner:
			and R12, R6  ; only lower 2 bits of R12 survive (1 cycle)
			;mov.w TABLE(R12),R13 ; lookup (3 cycles if linked in SRAM)
			mov.b R13, R14 ; backup lower byte in R14 (1 cycle)
			sxt R14  ; expand R14 into two bytes (1 cycle)
			;mov.w R14, UCB0TXBUF ; move the R and G values into TXBUF (3 cycles)
			; 16 cycles since dd_loop!
			;mov.b R14, UCB0TXBUF ; send out the B channel (3 cycles)

			; Onto Pixel 2 of the pair
			swpb R13  ; Swap the byte for Pixel 2 down lower
			sxt R13 ; extend out to full byte
			;mov.w R13, UCB0TXBUF
			rra R7 ; shift away the two bits we just sent
			rra R7
			;mov.b R13, UCB0TXBUF ; send B channel of pixel 2
			dec R8 ; 1 cycle
			jnz dd_loop_inner ; 2 cycles
		inc R5 ; 1 cycle
		;tst R4, R5 ; 1 cycle
		jnz dd_loop ; 2 cycles
	ret
