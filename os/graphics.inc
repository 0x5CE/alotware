;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;	        Graphics Related functions		  ;
;---------------------------------------------------------;

;________________________________________________
;Calculate pixel offset in framebuffer
;IN:  eax X
;     ebx Y
;OUT: esi Pixel address
;
pixelOffset:
	push eax		;X

	mov esi, dword[LFBAddress]	;Pointer to video memory
	
	movzx eax, word[bytesPerScanline]
	mul ebx			;Y * bytes per line

	add esi, eax
	
	pop eax			;X
	
	movzx ebx, byte[bytesPerPixel]
	mul ebx			;X * Bytes per pixel

	add esi, eax		;ESI is pointer to video memory
	ret
;________________________________________________
;Put bitmap character in graphics mode
;IN:  dl Column
;     dh Row
;     al Character
;OUT: nothing
;
graphics.putBitmapChar:
	
	push edx
	
	and eax, 0xff
	sub eax, 32
	mov ebx, fonts.height
	mul ebx
	
	mov edi, fonts
	add edi, eax
	
	pop edx
	push edx
	
	mov eax, fonts.width
	movzx ebx, dl
	mul ebx
	mov word[.x], ax

	pop edx
	
	mov eax, fonts.height
	movzx ebx, dh
	mul ebx
	mov word[.y], ax
	
	mov eax, fonts.width
	mov ebx, dword[bytesPerPixel]
	mul ebx
	mov dword[.nextLine], eax
	
	movzx eax, word[.x]
	dec eax
	movzx ebx, word[.y]
	call pixelOffset
	
	mov ecx, fonts.height
	
.putColumn:
	mov al, byte[edi]
	inc edi
		
	push ecx
	mov ecx, fonts.width
.putRow:
	bt ax, 7
	jc .putForeground
	
.putBackground:
	mov edx, dword[text.background]	
	jmp .putRow.next
	
.putForeground:
	mov edx, dword[text.foreground]

.putRow.next:
	add esi, dword[bytesPerPixel]	
	
	mov word[gs:esi], dx
	shr edx, 8
	mov byte[gs:esi+2], dh
	
	shl al, 1
	loop .putRow
	
	pop ecx

	add esi, dword[bytesPerScanline]
	sub esi, dword[.nextLine]
	
	loop .putColumn
	
.end:
	ret
	
.x:	dw 0
.y:	dw 0
.nextLine: dd 0

;________________________________________________
;Use back buffer (double buffering)
;IN/OUT: nothing
;
useVideoBuffer2:
	mov eax, [LFBAddress]
	mov [videoBuffer1], eax	;Save orignal address
	
	mov eax, [videoBuffer2]
	mov [LFBAddress], eax
	
	ret
	
;________________________________________________
;Use real video frame buffer
;IN/OUT: nothing
;
useVideoBuffer1:
	mov eax, [videoBuffer1]
	mov [LFBAddress], eax	;Restore orignal address
	ret

;________________________________________________
;Copy backend buffer to video memory
;IN/OUT: nothing
;	
screenRefresh:
	cmp byte[isGraphicsMode], 1
	jne .nothingToDo
	
	mov eax, dword[videoSize]
	mov ecx, eax
	shr ecx, 7		;Divide by 128
	
	mov edi, dword[videoBuffer1]
	mov esi, dword[videoBuffer2]
	
	push es
	push ds
	
	mov ax, 0x18
	mov es, ax
	mov ds, ax

.refreshLoop:

	prefetch [esi+128]
	prefetch [esi+160]
	prefetch [esi+192]
	prefetch [esi+224]

	movdqa xmm0, [esi+0]
	movdqa xmm1, [esi+16]
	movdqa xmm2, [esi+32]
	movdqa xmm3, [esi+48]
	movdqa xmm4, [esi+64]
	movdqa xmm5, [esi+80]
	movdqa xmm6, [esi+96]
	movdqa xmm7, [esi+112]
	
	movdqa [edi+0], xmm0 
	movdqa [edi+16], xmm1
	movdqa [edi+32], xmm2
	movdqa [edi+48], xmm3
	movdqa [edi+64], xmm4
	movdqa [edi+80], xmm5
	movdqa [edi+96], xmm6
	movdqa [edi+112], xmm7
		
	add edi, 128
	add esi, 128
	
	loop .refreshLoop
	
	pop ds
	pop es

.nothingToDo:
	ret

;________________________________________________
;Put a pixel on screen
;IN:  eax X
;     ebx Y
;     edx Hex color
;OUT: nothing
;
graphicsPutPixel:
	push eax
	push edx
	push ebx
	push esi
	
	push edx
	call pixelOffset	;Get pixel offset
	pop edx
	
	mov word[gs:esi], dx
	shr edx, 8
	mov byte[gs:esi+2], dh

.end:	
	pop esi
	pop ebx
	pop edx
	pop eax
	ret

;______________________________________________________________________________________________________
;Draw a block of specified color
;IN:  eax X
;     ebx Y
;     esi Width
;     edi height
;     edx Hex fill color
;OUT: nothing
;
graphicsDrawBlock:
	push eax
	push ebx
	push ecx

	cmp byte[isGraphicsMode], 1
	jne .end

	mov ecx, edi		;Height
.y:
	push ecx
	mov ecx, esi		;Width
.x:
	call graphicsPutPixel	;Just a stub, I should not do it this way
	inc eax
	loop .x
	pop ecx

	sub eax, esi

	inc ebx
	loop .y

.end:
	pop ecx
	pop ebx
	pop eax
	ret


bitsPerPixel: db 0
bytesPerPixel:	dd 0

bytesPerScanline: dd 0

LFBAddress:	dd 0
videoSize:	dd 0

resolution:
	.x:	dw 1024
	.y:	dw 768
	
maxColumn:	dw 0
maxRow:		dw 0

VBEMode:	dw .DEFAULT 
.DEFAULT	= 0x118		;I should not assume video mode number

text:
.background: dd .DEFAULT_BACKGROUND
.foreground: dd .DEFAULT_FOREGROUND

.bytesPerRow: dd 0

.DEFAULT_BACKGROUND = 0xffffffff
.DEFAULT_FOREGROUND = 0
