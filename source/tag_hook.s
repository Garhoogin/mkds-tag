@player hit hook
ansub_0206D7C8:
		@r0: srcDriver
		@r1: dstDriver
		@r2: pushbackDir
		
		@push argument registers and registers OtherKartCollide pushes
		push {r0-r11, lr}
		bl DriverHandleCallback
		pop {r0-r11, lr}
		
		@call back to vanilla
		push {r4-r11, lr}
		b 0x0206D7CC

@minigame start hook
ansub_020A0EFC:
		bl IsTagMode
		cmp r0, #0
		blne ChooseNextPlayerIt
		pop {r4-r11, pc}

@minigame place markers hook
ansub_020BD0DC:
		@r0: cell number minus 0x85
		@r8: driver index
		
		push {r1, r2, r3}
		lsl r1, r8, #16
		lsr r1, r1, #16
		bl MarkerDrawCallback
		pop {r1, r2, r3}

		add r0, r0, #0x85
		b 0x020BD0E0

@minigame elimination hooks
ansub_020A1630:
		push {r1-r12}
		bl IsTagMode
		pop {r1-r12}
		cmp r0, #0
		bne 0x020A1638
		cmp r6, r4
		ble 0x020A15AC
		b 0x020A1638
ansub_020A1584:
		push {r0-r12}
		bl IsTagMode
		cmp r0, #0
		pop {r0-r12}
		beq .elimDefault

		ldr r0, =g_playerIt
		ldr r0, [r0]
		bl mgcnt_killDriverDirect
		push {r0-r3}
		bl ChooseNextPlayerIt
		pop {r0-r3}
		b 0x020A1610
	.elimDefault:
		cmp r4, r0
		beq 0x020A1638
		b 0x020A158C

@low shine flash hook
ansub_020A1048:
		push {r0, r14}
		bl IsTagMode
		mov r1, r0
		pop {r0, r14}
		cmp r1, #0
		ldreq r2, =0x0217B1FC
		beq 0x020A104C
		b IsDriverIt
