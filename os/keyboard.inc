;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;	       Keyboard related functions		  ;
;---------------------------------------------------------;
;________________________________________________
;Initialize keyboard:
;Set LEDs, repeat rate and delay
;IN/OUT: nothing
initKeyboard:
	push eax

	;We have to first send command and then send data 
	;Set LEDs

	mov al, 0xED		;0xED is command for set LEDs
	call waitPS2Write
	out 0x60, al		;Send Command
	
	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 000b		;000 is to set All LEDs off
	call waitPS2Write
	out 0x60, al		;Now send data
	
	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	;Set repeat rate and delay

	mov al, 0xf3		;0xF3 is command for set delay and repeat rate
	call waitPS2Write
	out 0x60, al		;Send Command
	
	call waitPS2Read
	in al, 0x60		;Get acknowledge byte
	
	mov al, 00000000b	;0 is always 0, 00 is for delay 250 ms, 00000 is repeat rate 30 hz
	call waitPS2Write
	out 0x60, al		;Now send data
	
	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

.end:
	pop eax
	ret

;________________________________________________
;Wait for PS2 Controller to write
;IN/OUT: nothing
waitPS2Write:
	push eax
.waitLoop:
	in al, 0x64		;0x64 is status register
	bt ax, 1		;Check 2nd bit to become 0
	jnc .OK
	jmp .waitLoop
.OK:
	pop eax
	ret

;________________________________________________
;Wait for PS2 Controller to read
;IN/OUT: nothing
waitPS2Read:
	push eax
.waitLoop:
	in al, 0x64		;0x64 is status register
	bt ax, 0		;Check 1st bit to become 1
	jc .OK
	jmp .waitLoop
.OK:
	pop eax
	ret

;________________________________________________
;Get string from keyboard
;IN: 	al Maximum length of string to get 
;OUT:	esi String
;
getString:
	push eax
	push ecx
	push edx
	push es
	
	mov byte[.string], 0
	mov byte[.currentChar], 0
	
	push ds
	pop es			;Make sure es = ds

	mov ecx, 0		;Characters counter	
	movzx ebx, al		;Maximum characters
	
	call getCursor

.getKey:
	call setCursor
	mov esi, .string
	call printString
	
	mov al, ' '
	call putChar
	
	push edx
	add dl, byte[.currentChar]
	call setCursor
	pop edx
	
	call waitKeyboard	;Get character

	cmp ah, 71		;Scancode
	je .homeKey
	
	cmp ah, 79
	je .endKey
		
	cmp ah, 83
	je .deleteKey
	
	cmp ah, 75
	je .left
	
	cmp ah, 77
	je .right
	
	cmp al, 10		;ASCII code
	je .end
	
	cmp al, 8
	je .backSpace
	
	cmp al, ' '
	jb .getKey		;Do not use this key
	cmp al, '~'
	ja .getKey		;Do not use this key

	cmp cl, bl
	je .getKey

	push edx
	movzx esi, byte[.currentChar]
	add esi, .string
	mov edx, 0
	call stringInsertChar
	pop edx
	
	inc byte[.currentChar]
	inc cl
	jmp .getKey
	
.backSpace:
	cmp byte[.currentChar], 0	;Not allowed
	je .getKey

	dec byte[.currentChar]
	
	push ecx	
	movzx esi, byte[.currentChar]
	add esi, .string
	mov eax, 0
	call stringRemoveChar
	pop ecx
	
	dec cl
	jmp .getKey

.deleteKey:
	cmp byte[.currentChar], cl	;Not allowed
	je .getKey
	
	push ecx	
	movzx esi, byte[.currentChar]
	add esi, .string
	mov eax, 0
	call stringRemoveChar
	pop ecx
	
	dec cl
	jmp .getKey	
	
.homeKey:
	mov byte[.currentChar], 0
	jmp .getKey

.endKey:
	mov byte[.currentChar], cl
	jmp .getKey
	
.left:
	cmp byte[.currentChar], 0	;Not allowed
	je .getKey
	
	dec byte[.currentChar]
	jmp .getKey
	
.right:
	cmp byte[.currentChar], cl	;Not allowed
	je .getKey
	
	inc byte[.currentChar]
	jmp .getKey
	
.end:
	and ecx, 0x0f
	mov esi, .string
	
	pop es
	pop edx
	pop ecx
	pop eax
	ret
	
.string: times 256 db 0		;Buffer to store characters
.currentChar:	db 0

;________________________________________________
;Get special keys status
;IN: 	nothing
;OUT:	eax Keys status
;
;Format:
;bit 0: Control key
;bit 1: Shift key
;bit 2-31: Reserved
;
getKeysStatus:
	mov eax, [keysStatus]
	ret
	
;________________________________________________
;Wait for keyboard keypress
;IN: 	nothing
;OUT:	al ASCII character
;	ah SCAN code
;
waitKeyboard:
	push ebx
	
	sti
.keyLoop:
	mov al, byte[.currentScancodeIndex]
	cmp byte[kbdHandler.scanCodes.index], al
	je .keyLoop

	mov ebx, kbdHandler.scanCodes
	add bl, byte[.currentScancodeIndex]
	
	mov ah, byte[ebx]
	mov al, ah
	
	cmp byte[.currentScancodeIndex], 31
	jl .incIndex
	mov byte[.currentScancodeIndex], -1
.incIndex:
	inc byte[.currentScancodeIndex]
	
	bt ax, 7			;Check if keyup
	jc .keyLoop

;Check shift status
;
	cmp byte[kbdHandler.shiftFlag], 1
	je .useShiftChars
	
	mov ebx, .keys			;Scan codes array
	xlatb
	
	jmp .end
	
.useShiftChars:
	mov ebx, .keysShift		;Scan codes array with shift key
	xlatb
.end:
	pop ebx
	ret
.currentScancodeIndex: db 0
;--------Variables----------;
.keys:
	db 27, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']'
	db 10, 29, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 42, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
	db 0xff, 0xff, 0xff, ' '
	
	db 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, '7', '8', '9', '-', '4', '5', '6', '+'
	db '1', '2', '3', '0', '.', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	db 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	db 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	
.keysShift:
	db 27,0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8, 9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}'
	db 10, 29, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 42, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'
	db 0xff, 0xff, 0xff, ' '
	
	db 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, '7', '8', '9', '-', '4', '5', '6', '+'
	db '1', '2', '3', '0', '.', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	db 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	db 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	

;_________________________________________________________________________________________________

