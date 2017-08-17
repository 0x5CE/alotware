;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;        Video and text output related functions	  ;
;---------------------------------------------------------;

;________________________________________________
;Set text mode
;IN/OUT: nothing
;
setTextMode:
	push eax
	
	mov ah, 0		;Funtion to set video mdoe
	mov al, 3		;Text video mode
	call int10h		;Call real-mode BIOS interrupt
	
	mov ax, 0x1003	
	mov bx, 0
	call int10h		;Set text blinking off

	mov byte[isGraphicsMode], 0
	
	call clearScreen
	pop eax
	ret

;________________________________________________
;Put computer into graphics mode
;IN: 	nothing
;OUT: 	ESI pointer to video memory
;
setGraphicsMode:
	push eax
	push ebx
	push ecx
	push edi	

	mov ax, word[VBEMode]			;Default is 1024*768*24

	mov cx, ax				;CX: mode to get info
	mov ax, 0x4f01				;Funtion to get mode info
	mov di, VBE_MODE_BLOCK + 0x500
	call int10h

	mov esi, dword[VBE_MODE_BLOCK+40]	;Pointer to video memory base	
	mov dword[LFBAddress], esi

	or cx, 100000000000000b			;Set bit 14 to get linear framebuffer
	mov bx, cx
	mov ax, 0x4f02				;Funtion to set video mode
	call int10h				;Call real-mode BIOS interrupt
	
	mov ax, word[VBE_MODE_BLOCK+16]
	mov word[bytesPerScanline], ax
	
	mov al, byte[VBE_MODE_BLOCK+25]		;Get bits per pixel
	cmp al, 0
	jne .bitsPerPixelOK
	
	mov al, 24

.bitsPerPixelOK:
	mov byte[bitsPerPixel], al		;Save bits per pixel
	shr al, 3				;Divide by 8
	mov byte[bytesPerPixel], al

	mov ax, word[VBE_MODE_BLOCK+18]		;Get X resolution
	cmp ax, 0
	jne .xResOK
	mov ax, 1024
.xResOK:
	mov word[resolution.x], ax		;Save X resolution
	
	mov ax, word[VBE_MODE_BLOCK+20]		;Get Y resolution
	cmp ax, 0
	jne .yResOK
	mov ax, 768
.yResOK:
	mov word[resolution.y], ax		;Save Y resolution


	movzx eax, word[resolution.x]
	mov ebx, fonts.width
	xor edx, edx
	div ebx
	dec ax				;Counting form 0
	mov word[maxColumn], ax
	
	movzx eax, word[resolution.y]
	mov ebx, fonts.height
	xor edx, edx
	div ebx
	dec ax				;Counting form 0
	mov word[maxRow], ax
	
	mov byte[isGraphicsMode], 1
	
	mov eax, dword[bytesPerScanline]
	movzx ebx, word[resolution.y]
	mul ebx
	mov dword[videoSize], eax
	
	mov eax, dword[bytesPerScanline]
	mov ebx, fonts.height
	mul ebx
	mov dword[text.bytesPerRow], eax
	
	mov eax, [LFBAddress]
	mov [videoBuffer1], eax	;Save orignal address
	
	call clearScreen
	
	pop edi
	pop ecx
	pop ebx
	pop eax
	ret

;________________________________________________
;Get screen information
;IN: nothing
;OUT: eax Resolution X (bits 0..15), Y (bits 16..31)
;     ebx Columns (bit 0..7), Rows (8..15), Bits Per Pixel (16..23)
;     edx Frame buffer starting address
;     CF sets on text mode
;
getScreenInfo:
	cmp byte[isGraphicsMode], 0
	je .textMode

.graphicsMode:
	push ecx
	
	mov bl, byte[bitsPerPixel]
	shl ebx, 8
	
	mov bl, byte[maxRow]
	inc bl			;Counting from 1

	shl ebx, 8
	
	mov bl, byte[maxColumn]
	inc bl			;Counting from 1
	
	mov ax, word[resolution.y]
	shl eax, 16
	mov ax, word[resolution.x]	
	
	mov edx, dword[LFBAddress]

	pop ecx
	
	clc
	ret
	
.textMode:
	mov bl, textMode.MAX_COL+1
	mov bh, textMode.MAX_ROW+1
	and ebx, 0xffff
	
	mov eax, 0
	mov edx, textMode.videoMemory
	
	stc
	ret
;________________________________________________
;Clear the screen
;IN/OUT: nothing
;
clearScreen:
	cmp byte[isGraphicsMode], 1	;Check graphics mode
	je .graphics

.text:
	xor edx, edx
	call setCursor

	mov edi, textMode.videoMemory	
	mov ecx, (textMode.MAX_ROW+1) * (textMode.MAX_COL+1)
	mov ah, byte[textMode.currentColor]		;Color
	mov al, ' '			;Char to fill screen with
	rep stosw			;Clear video memory
	jmp .end

.graphics:
	cmp dword[text.background], 0xffffffff
	je .sseClear
	
	mov esi, dword[LFBAddress]
	
	mov eax, dword[videoSize]
	mov ebx, dword[bytesPerPixel]
	xor edx, edx
	div ebx
	mov ecx, eax
	
	mov ebx, dword[bytesPerPixel]
	mov edx, dword[text.background]
	
.clearLoop:
	mov dword[gs:esi], edx
	add esi, ebx
	loop .clearLoop	
	ret

.sseClear:
	mov edi, dword[LFBAddress]
	movdqa xmm0, [.clearBytes]
	
	mov ecx, dword[videoSize]
	shr ecx, 7
	
	push ds
	mov ax, 0x18
	mov ds, ax
.loop:
	movdqa [edi+00], xmm0
	movdqa [edi+16], xmm0
	movdqa [edi+32], xmm0
	movdqa [edi+48], xmm0
	movdqa [edi+64], xmm0
	movdqa [edi+80], xmm0
	movdqa [edi+96], xmm0
	movdqa [edi+112], xmm0
	
	add edi, 128
	loop .loop
	pop ds
	
	mov dx, 0
	call setCursor
.end:
	ret
align 16
.clearBytes: times 4 dd text.DEFAULT_BACKGROUND

;________________________________________________
;Clear specified line on screen
;IN: 	al line to clear
;OUT: 	nothing
;
clearLine:
	cmp byte[isGraphicsMode], 1
	je .graphics
	
	push eax
	push ecx
	push edx
	push edi
	push es
	
	push 0x18
	pop es
	
	mov dl, 0
	mov dh, al
	call setCursor

	movzx eax, al			;Calculate position
	mov ecx, 160	
	xor edx, edx
	mul cx

	mov edi, textMode.videoMemory
	add edi, eax

	shr ecx, 2			;Divide ecx by 4

	mov ah, [textMode.currentColor]	;Color
	mov al, ' '
	shl eax, 16
	mov ah, [textMode.currentColor]	;Color
	mov al, ' '
	rep stosd

	pop es
	pop edi
	pop edx
	pop ecx
	pop eax
	ret
	
.graphics:
	push eax
	push ebx
	push ecx
	push edx
	push esi
	
	xor dl, dl
	mov dh, al
	call setCursor
	 
	mov esi, dword[LFBAddress]
	
	and eax, 0xff
	mov ebx, fonts.height
	mul ebx
	
	mov ebx, dword[bytesPerScanline]
	mul ebx
	add esi, eax
	
	movzx eax, word[bytesPerScanline]
	mov ebx, fonts.height
	mul ebx
	mov ebx, dword[bytesPerPixel]
	xor edx, edx
	div ebx
	mov ecx, eax

	mov ebx, dword[bytesPerPixel]
	mov edx, dword[text.background]
.clearLoop:
	mov dword[gs:esi], edx
	add esi, ebx
	loop .clearLoop
	
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
;________________________________________________
;Scroll the screen down
;IN/OUT: nothing
;
scrollDown:
	push eax
	push ecx
	push edx
	push esi
	push edi
	
	push ds
	push es
	
	cmp byte[isGraphicsMode], 1
	je .graphics
.text:
;Move all screen content up one line
;
	mov ax, 0x18
	mov es, ax
	mov ds, ax
	
	mov esi, textMode.videoMemory
	mov edi, textMode.videoMemory-160		;One line up
	mov ecx, 2000
	rep movsw	;Repeat ECX times (mov byte[es:edi], byte[ds:esi])
	
	mov ax, 0x10
	mov ds, ax
	
	mov eax, textMode.MAX_ROW	;Clear last line
	call clearLine
	
	jmp .end
	
.graphics:
	mov esi, dword[LFBAddress]
	
	mov edi, esi
	sub edi, dword[text.bytesPerRow]
	
	mov ecx, [videoSize]
	shr ecx, 7		;Divide by 128
	
	mov ax, 0x18
	mov es, ax
	mov ds, ax
	
.copy:

	prefetchnta [esi+0]
	prefetchnta [esi+32]
	prefetchnta [esi+64]
	prefetchnta [esi+96]

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
	
	loop .copy
	
	mov ax, 0x10
	mov ds, ax
	
	movzx eax, word[maxRow]
	call clearLine
.end:
	pop es
	pop ds
	
	pop edi
	pop esi
	pop edx
	pop ecx
	pop eax
	ret
;________________________________________________
;Prints an Integer in decimal
;IN: 	eax Integer
;OUT: 	nothing
;
printIntDec:
	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi

;Check for negative
;
	cmp eax, 0
	jge .positive
	
.negative:
	push eax
	mov al, '-'			;Print minus
	call putChar
	pop eax
	
	neg eax
		
.positive:

;Convert Integer to string. To print it to screen
;
	mov ebx, 10			;decimals are base-10	
	xor ecx, ecx			;mov ecx, 0
	
.ConvertLoop:
	xor edx, edx			;moc edx, 0
	div ebx					 
	add dl, 0x30			;Convert it to Ascii
	push edx
	inc ecx
	or eax, eax			;cmp eax, 0
	jne .ConvertLoop
 
	mov edx, esi
.PrintLoop:
	pop eax
	call putChar
	loop .PrintLoop
.end:
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret

;________________________________________________
;Prints an Integer in Binary
;IN: 	eax Integer
;OUT: 	nothing
;
printIntBin:
	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi
	
;Check for negative
;
	cmp eax, 0
	jge .positive
	
.negative:
	push eax
	mov al, '-'			;Print minus
	call putChar
	pop eax
	
	neg eax
		
.positive:

;Convert Integer to string. To print it to screen
;
	mov ebx, 2			;Binary numbers are base-2	
	xor ecx, ecx			;mov ecx, 0
	
.ConvertLoop:
	xor edx, edx			;moc edx, 0
	div ebx					 
	add dl, 0x30			;Convert it to Ascii
	push edx
	inc ecx
	or eax, eax			;cmp eax, 0
	jne .ConvertLoop
 
	mov edx, esi
.PrintLoop:
	pop eax
	call putChar
	loop .PrintLoop
.end:
	mov al, 'b'
	call putChar
	
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret


;________________________________________________
;Prints an Integer in Hexadecimal
;IN: 	eax Integer
;OUT: 	nothing
;
printIntHex:
	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi
	
;Check for negative
;
	cmp eax, 0
	jge .positive
	
.negative:
	push eax
	mov al, '-'			;Print minus
	call putChar
	pop eax
	
	neg eax
		
.positive:
	
	push eax
	mov al, '0'
	call putChar
	mov al, 'x'
	call putChar
	pop eax
	
;Convert Integer to string. To print it to screen
;
	mov ebx, 16			;Hexadecimals are base-16	
	xor ecx, ecx			;mov ecx, 0
.ConvertLoop:
	xor edx, edx			;moc edx, 0
	div ebx		
	add dl, 0x30	
	cmp dl, 0x39
	ja .add
	jmp short .next

.add:		 
	add dl, 7			;Convert it to ASCII

.next:
	push edx
	inc ecx
	or eax, eax			;cmp eax, 0
	jne .ConvertLoop
 
	mov edx, esi
.PrintLoop:
	pop eax
	call putChar
	loop .PrintLoop
.end:

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret

;________________________________________________
;Same as putChar but does not move cursor
;IN: al Character
;OUT: nothing
;
putCharO:
	cmp byte[isGraphicsMode], 1
	je putChar.graphics
	
	mov dl, byte[textMode.cursor.X]
	mov dh, byte[textMode.cursor.Y]

	cmp al, 10		;New-line character
	je .newLine
	cmp al, 9
	je .tab
	cmp al, ' '		;First printable character
	jb .notPrintable
	cmp al, '~'		;Last printable character
	ja .notPrintable
	
	jmp .next
	
.tab:
	mov al, ' '		;(stub)
	jmp .next
	
.newLine:
	inc dh
	mov dl, 0
	mov al, 0xff
	jmp .next

.notPrintable:
	mov al, 0xff

.next:
	
;Fix X and Y
;
	cmp dh, textMode.MAX_ROW
	jna .yOK
	call scrollDown
	mov dh, textMode.MAX_ROW
	
.yOK:
	cmp dl, textMode.MAX_COL
	jna .xOK
	mov dl, 0
	inc dh
	
.xOK:
	
	push edx	
	push eax
;Calculate character position in Screen
;
	mov eax, 0
	mov al, dl
	shl ax, 1			;Multiply X by 2
	mov edi, eax			;Add it to index
	mov al, (textMode.MAX_COL+1)*2	;Counting from 1
	mul dh				;Multiply Y by MAX_COL*2
	add edi, eax			;Add it to index
	pop eax
	
;Put Character
;
	pop edx
	cmp al, 0xff
	je .notPrintableChar
	
	inc dl

	mov ah, byte[textMode.currentColor]
	
;If character already exists
;
	cmp word[gs:textMode.videoMemory + edi], ax
	je .end
	
	mov word[gs:textMode.videoMemory + edi], ax

.notPrintableChar:

.end:	
;Update cursor
;
	call setCursor
	
	ret
	
putChar.graphics:
	
	call getCursor
	
	cmp al, 9
	je .tab
	cmp al, 10
	je .return
	cmp al, '~'
	ja .notPrintable
	cmp al, ' '
	jl .notPrintable
	jmp .fixXandY

.tab:
	mov al, ' '		;(stub)
	jmp .fixXandY
	
.notPrintable:
	mov al, ' '
	jmp .fixXandY
	
.return:
	movzx eax, word[setCursor.graphics.previousX]
	movzx ebx, word[setCursor.graphics.previousY]
	
	push edx
	mov ecx, fonts.height
	mov edx, [setCursor.graphics.previousCursorColor]
.clearPreviousCursor:
	call graphicsPutPixel
	inc ebx
	loop .clearPreviousCursor
	pop edx

	mov dl, 0
	inc dh
	mov al, 0	;Mark as not printable
	
.fixXandY:
	cmp dl, byte[maxColumn]
	jna .yOK
	
	mov dl, 0
	inc dh
.yOK:	
	cmp dh, byte[maxRow]
	jna .xOK
	
	call scrollDown
	mov dh, byte[maxRow]
	mov dl, 0
.xOK:
	cmp al, 0
	je .next
	
.printable:
	push edx
	call graphics.putBitmapChar
	pop edx
	inc dl
	jmp .next
.next:
	mov byte[textMode.cursor.X], dl
	mov byte[textMode.cursor.Y], dh
	ret
	
;________________________________________________
;Write a character at cursor position
;IN: al Character
;OUT: nothing
;
putChar:
	pushad
	call putCharO
	call setCursor
	popad
	ret
	
;________________________________________________
;Get cursor position
;IN: nothing
;OUT: dl X, dh Y
;
getCursor:
	mov dl, byte[textMode.cursor.X]
	mov dh, byte[textMode.cursor.Y]
	ret
;________________________________________________
;Move the cursor to specified position
;IN: DL X, DH Y
;OUT: nothing
;
setCursor:
	cmp byte[isGraphicsMode], 1
	je setCursor.graphics
	
	push eax
	push ebx
	push edx

	mov byte[textMode.cursor.X], dl
	mov byte[textMode.cursor.Y], dh
	
;Fix X and Y
	cmp dh, textMode.MAX_ROW
	jna .yOK
	mov dh, textMode.MAX_ROW
.yOK:
	cmp dl, textMode.MAX_COL
	jna .xOK
	mov dl, textMode.MAX_COL
.xOK:
	
;Now we have to multiply Y by total columns and add X to it
	movzx eax, dh
	mov bl, textMode.MAX_COL+1	;Counting from 1
	mul bl				;Multiply Y by columns

	movzx ebx, dl
	add eax, ebx			;Add X to it

	mov ebx, eax
	
	mov al, 0x0f
	mov dx, 0x3d4
	out dx, al

;Send lower byte to VGA port
	mov al, bl			;BL is lower byte
	mov dx, 0x3d5			;VGA port
	out dx, al

	mov al, 0x0e
	mov dx, 0x3d4
	out dx, ax

;Send higher byte to VGA port
	mov al, bh			;BH is higher byte
	mov dx, 0x3d5			;VGA port
	out dx, al
	
	pop edx
	pop ebx
	pop eax
	ret
	
setCursor.graphics:
	push eax
	push ebx
	push ecx
	push edx
	
	mov byte[textMode.cursor.X], dl
	mov byte[textMode.cursor.Y], dh	
	
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

	movzx eax, word[.previousX]
	movzx ebx, word[.previousY]
	
	mov ecx, fonts.height
	mov edx, [.previousCursorColor]
.clearPreviousCursor:
	call graphicsPutPixel
	inc ebx
	loop .clearPreviousCursor
	
	movzx eax, word[.x]
	movzx ebx, word[.y]
	
	mov word[.previousX], ax
	mov word[.previousY], bx

	mov edx, dword[text.background]
	mov dword[.previousCursorColor], edx
	
	mov ecx, fonts.height
	mov edx, dword[text.foreground]
.drawCursor:
	call graphicsPutPixel
	inc ebx
	loop .drawCursor
	
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
	
.previousX: dw 0
.previousY: dw 0
.previousCursorColor: dd text.DEFAULT_BACKGROUND
.x: dw 0
.y: dw 0

;________________________________________________
;Print a 0 terminated string at cursor position
;IN: 	ESI string
;OUT: 	nothing
;
printString:
	push esi
	push eax
	push ecx

;Check for null string
;
	cmp byte[esi], 0
	je .end
	
;Find length of string
;	
	call stringLength
	mov ecx, eax

;Write all characters
;
.printStringLoop:
	lodsb			;mov al, byte[esi] & inc esi
	call putChar
	loop .printStringLoop	
	
.end:
	pop ecx
	pop eax
	pop esi
	ret

;________________________________________________
;Change text background and foreground color
;IN: 	eax Foreground (hex RGB)
; 	ebx Background (hex RGB)
;OUT: 	nothing
;
;Note: Text mode is black and white only.
;
text.setColor:
	cmp byte[isGraphicsMode], 1
	je .graphics
	
.textMode:
	mov byte[textMode.currentColor], textMode.DEFAULT_COLOR
	ret
.graphics:
	mov dword[text.foreground], eax
	mov dword[text.background], ebx
	ret
;________________________________________________
;Get text background and foreground color
;IN: 	nothing
;OUT: 	eax Foreground (hex RGB)
; 	ebx Background (hex RGB)
;
;Note: Text mode is black and white only.
;
text.getColor:
	cmp byte[isGraphicsMode], 1
	je .graphics
	
.textMode:
	mov al, textMode.DEFAULT_COLOR
	ret
.graphics:
	mov eax, dword[text.foreground]
	mov ebx, dword[text.background]
	ret
textMode:
	.currentColor:	db .DEFAULT_COLOR
	.DEFAULT_COLOR	= 0xf0
	
	.cursor.X:	db 0
	.cursor.Y:	db 0
	
	.MAX_ROW	= 24			;Counting from zero
	.MAX_COL	= 79
	
	.videoMemory	= 0xb8000
