;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;	Interrupt service routines and IRQ Handlers	  ;
;---------------------------------------------------------;

;---------------Interrupt Handlers-------------------

;Keyboard interrupt IRQ 1
;
kbdHandler:
	push eax
	push ebx
	push ds
	
	mov ax, 0x10			;Kernel's data segment
	mov ds, ax
		
	xor eax,eax

	in al, 0x60
  	
	cmp al, 1	
	je .terminate

;Check for control key
;
	cmp al, 29
	je .controlPressed
	
	cmp al, 29+128
	je .controlReleased
	
;Check for shift key
;
	cmp al, 54			;Right shift key
	je .shiftKey
	cmp al, 42			;Left shift key
	je .shiftKey
	
	cmp al, 54+128			;Right shift released
	je .shiftReleased
	cmp al, 42+128			;Left shift released
	je .shiftReleased
	
	jmp .otherKey

.controlPressed:
	or dword[keysStatus], 0x00000001
	jmp .doNotStore
	
.controlReleased:
	and dword[keysStatus], 0xfffffffe
	jmp .doNotStore
	
.shiftKey:
	or dword[keysStatus], 0x00000002
	
	mov byte[.shiftFlag], 1		;Shift is pressed
	jmp .doNotStore

.shiftReleased:
	and dword[keysStatus], 0xfffffffd
	
	mov byte[.shiftFlag], 0
	jmp .doNotStore
	
.otherKey:
	
	
	jmp .end

;Terminate current running application
;
.terminate:
	cmp byte[programCount], 0	
	je .end
	
	pop ds
	pop eax
	
	mov ax, 0x18
	mov es, ax
	
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	call text.setColor
	call screenRefresh
	call useVideoBuffer1
	call scrollDown
	
	mov al, 0x20
	out 0x20, al

	call terminate

.end:	
	mov ebx, .scanCodes
	add bl, byte[.scanCodes.index]
	
	mov byte[ebx], al

	cmp byte[.scanCodes.index], 31
	jl .incIndex
	mov byte[.scanCodes.index], -1
.incIndex:
	inc byte[.scanCodes.index]

.doNotStore:
	mov al, 0x20
	out 0x20, al
	
	pop ds
	pop ebx
	pop eax
	iret

.scanCodes: times 32	db 0
.scanCodes.index:	db 0
.shiftFlag: db 0

;bit 0: Control key
;bit 1: Shift key
;bit 2-31: Reserved
;
keysStatus: dd 0

;PIT timer handler IRQ 0
;
timerHandler:
	push eax
	
	;inc dword[.tickCount]
	
	mov al, 0x20		;End of interrupt
	out 0x20, al

	pop eax
	iret
;.tickCount:	dd	0

;PS/2 mouse handler IRQ 12
;
ps2MouseHandler:
	pusha

	cmp byte[.state], 0
	je .dataPacket
	
	cmp byte[.state], 1
	je .xPacket
	
	cmp byte[.state], 2
	je .yPacket
	
.dataPacket:
	in al, 0x60
	mov byte[.data], al

	mov byte[.state], 1
	jmp .end2

.xPacket:
	in al, 0x60
	mov byte[.deltaX], al

	mov byte[.state], 2
	jmp .end2

.yPacket:
	in al, 0x60
	mov byte[.deltaY], al

	mov byte[.state], 0

	mov byte[.changed], 1
.end:

	;----------------------
	
	movzx eax, byte[ps2MouseHandler.deltaX]	;deltaX is change in X
	movzx ebx, byte[ps2MouseHandler.deltaY]	;deltaY is change in Y
	mov dl, byte[ps2MouseHandler.data]
	
	bt dx, 4		;Check if mouse is moved in left direction
	jnc .rightMovement
	
	xor eax, 0xff		;255 - deltaX
	sub word[.mouseX], ax	;mouseX - deltaX
	
	jnc .xOK		;Check if mouseX is < 0
	mov word[.mouseX], 0	;Fix mouseX

	jmp .xOK
.rightMovement:
	add word[.mouseX], ax	;mouseX + deltaX
.xOK:
	bt dx, 5		;Check if mouse is moved in down direction
	jnc .upMovement
	
	xor ebx, 0xff		;255 - deltaY
	sub word[.mouseY], bx	;mouseY - deltaY

	jnc .yOK		;Check if mouseY is < 0
	mov word[.mouseY], 0	;Fix mouseY

	jmp .yOK

.upMovement:
	add word[.mouseY], bx	;mouseY + deltaY
.yOK:
	
	movzx eax, word[.mouseX]
	movzx ebx, word[.mouseY]

	;Make sure X and Y are not greater than screen resolution
	;
	cmp ax, word[resolution.x]
	jng .xIsNotHuge
	mov ax, word[resolution.x]
	mov word[.mouseX], ax
	
.xIsNotHuge:
	cmp bx, word[resolution.y]
	jng .yIsNotHuge
	mov bx, word[resolution.y]
	mov word[.mouseY], bx

.yIsNotHuge:
	
	push edx
	movzx edx, word[resolution.y]
	sub dx, word[.mouseY]
	mov ebx, edx
	pop edx
	
	mov dword[mouseX], eax
	mov dword[mouseY], ebx

.end2:	
	;------------------------
	mov al, 0x20		;End of interrupt
	out 0x20, al
	out 0xa0, al

	popa
	iret
.state:		db 0
.deltaX: 	db 0
.deltaY:	db 0
.data:		db 0
.changed:	db 0

align 32

.mouseState: 	dd 0
.mouseX:	dd 0
.mouseY:	dd 0

;Synaptics mouse handler IRQ 12
synapticsHandler:
	push eax
	push edx

	cmp byte[.state], 0
	je .packet0
	
	cmp byte[.state], 1
	je .packet1
	
	cmp byte[.state], 2
	je .packet2

	cmp byte[.state], 3
	je .packet3

	cmp byte[.state], 4
	je .packet4

	cmp byte[.state], 5
	je .packet5
	
.packet0:
	mov al, 0
	call clearLine

	in al, 0x60

	movzx eax, al
	mov dl, 0
	mov dh, 0	
	call printIntBin
	
	mov byte[.state], 1
	jmp .end

.packet1:
	mov al, 1
	call clearLine

	in al, 0x60
	mov bl, al

	and al, 1111b
	movzx eax, al
	shl eax, 8
	mov word[.X], ax
	
	mov al, bl
	and al, 11110000b
	movzx eax, al
	shl eax, 4
	mov word[.Y], ax

	mov al, bl
	movzx eax, al
	mov dl, 0
	mov dh, 1	
	call printIntBin

	mov byte[.state], 2
	jmp .end

.packet2:
	mov al, 2
	call clearLine

	in al, 0x60

	movzx eax, al
	mov dl, 0
	mov dh, 2	
	call printIntDec

	mov byte[.state], 3
	jmp .end

.packet3:
	mov al, 3
	call clearLine

	in al, 0x60
	mov bl, al
	
	movzx eax, al
	and eax, 00010000b
	shl eax, 8
	or word[.X], ax

	mov al, bl

	movzx eax, al
	and eax, 00100000b
	shl eax, 7
	or word[.Y], ax
	
	mov al, bl

	movzx eax, al
	mov dl, 0
	mov dh, 3
	call printIntBin

	mov byte[.state], 4
	jmp .end

.packet4:
	mov al, 4
	call clearLine

	in al, 0x60

	movzx ax, al
	or word[.X], ax

	movzx eax, word[.X]
;	sub eax, 1000
	mov dl, 0
	mov dh, 4	
	call printIntDec

	mov byte[.state], 5
	jmp .end

.packet5:
	mov al, 5
	call clearLine

	in al, 0x60

	movzx ax, al
	or word[.Y], ax

	movzx eax, word[.Y]
;	sub eax, 1000
	mov dl, 0
	mov dh, 5	
	call printIntDec

	mov byte[.state], 0
	jmp .end

.end:

	mov al, 0x20		;End of interrupt
	out 0x20, al
	out 0xa0, al

	pop edx
	pop eax
	iret
.state:		db 0
.X:		dw 0
.Y:		dw 0
.Z:		db 0

	
;Other Interrupts
unhandled:
	push eax
	mov al, 0x20
	out 0x20, al
	pop eax
	iret
;----------------------------------------------------

;________________________________________________________________
;Install interrupt handler or IRQ handler
;IN: eax Interrupt number
;    esi Interrupt service routine address
;OUT: nothing
;
installISR:
	push eax
	push ebp
	mov ebp, eax
	mov eax, esi
	mov word[IDT+ebp*8], ax
	shr eax, 16
	mov word[IDT+ebp*8+6], ax
	pop ebp
	pop eax
	ret
