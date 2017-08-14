;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;        Functions related to character strings		  ;
;---------------------------------------------------------;

;________________________________________________
;Get the length of a string
;IN:	ESI string
;OUT:	EAX string length
stringLength:
	push ecx
	push esi
	push edi
	push es
	
	push ds			;Make sure es = ds
	pop es
	
	mov edi, esi

	or ecx, 0xffffffff
	xor al, al		;mov al, 0 (end of string)
	cld			;Clear direction
	repne scasb		;Search end of string at edi
	or eax, 0xffffffff
	sub eax, ecx
	dec eax			;Not including character 0
	
	pop es
	pop edi
	pop esi
	pop ecx
	ret

;________________________________________________
;Compare first words of two strings
;IN:	ESI first string
;	EDI second string
;OUT:	carry sets if strings are equal
stringWordsCompare:
	push eax
	push esi
	push edi

.compareLoop:
	mov al, byte[esi]
	
	cmp al, ' '
	je .equal

	cmp al, byte[edi]
	jne .notEqual

	cmp byte[edi], 0
	je .equal

	inc esi
	inc edi
	jmp .compareLoop
	
.notEqual:

	clc
	jmp .end
.equal:
	cmp byte[edi], 0
	jne .notEqual

	stc
.end:
	pop edi
	pop esi
	pop eax
	ret
	
;________________________________________________
;Compare two strings
;IN:	ESI first string
;	EDI second string
;OUT:	carry sets if strings are equal
stringCompare:
	push eax
	push esi
	push edi
	
.compareLoop:
	mov al, byte[edi]
	
	cmp al, 0		;End of string
	je .equal
	
	cmp al, byte[esi]
	jne .notEqual
	
	inc esi
	inc edi
	jmp .compareLoop
	
.notEqual:
	clc
	jmp .end
.equal:
	stc
.end:
	pop edi
	pop esi
	pop eax
	ret

;________________________________________________
;Convert a string to Upper-case
;IN:	ESI String
;OUT:	nothing
stringUpperCase:
	push eax
	push ecx	
	push esi

	mov al, byte[esi]	
	cmp al, 0
	je .end

	call stringLength
	mov ecx, eax
.convertLoop:
	mov al, byte[esi]

.check1:
	cmp al, 'a'		;Check if character is lower case
	jae .check2
	inc esi
	loop .convertLoop
	jmp .end

.check2:
	cmp al, 'z'		;Check if character is lower case
	jbe .ok
	inc esi
	loop .convertLoop
	jmp .end
.ok:
	sub al, ' '		;Convert if character is lower case
	mov byte[esi], al

	inc esi	
	loop .convertLoop

.end:
	pop esi
	pop ecx
	pop eax
	ret

;________________________________________________
;Convert a string to Lower-case
;IN:	ESI String
;OUT:	nothing
stringLowerCase:
	push eax
	push ecx	
	push esi

	mov al, byte[esi]	
	cmp al, 0
	je .end

	call stringLength
	mov ecx, eax
.convertLoop:
	mov al, byte[esi]
.check1:
	cmp al, 'A'		;Check if character is upper case
	jae .check2
	inc esi
	loop .convertLoop
	jmp .end
.check2:
	cmp al, 'Z'		;Check if character is upper case
	jbe .ok
	inc esi
	loop .convertLoop
	jmp .end
.ok:
	add al, ' '		;Convert if character is upper case
	mov byte[esi], al

	inc esi
	loop .convertLoop

.end:
	pop esi
	pop ecx
	pop eax
	ret
	

;________________________________________________
;Remove white spaces from start of string and end
;IN:	ESI String
;OUT:	nothing
stringTrim:
	push eax
	push ebx
	push ecx
	push esi
	push edi
	push es
	
	push ds				;Make sure es = ds
	pop es
	
;First we have to remove white spaces from left and then from right

	cmp byte[esi], 0		;If string is empty, exit
	je .end

	call stringLength		;Get length of string SI in EAX
	mov ecx, eax			;Put it into ECX to use in loop

	push esi			;Save string position for latter use
	push ecx			;Save string length for latter use
	xor ebx, ebx			;EBX is counter of white spaces

	cld				;We are going from left to right so, clear direction flag
.trimFromLeft:
	lodsb
	cmp al, ' '
	je .trimLeft
	jmp short .noWhiteSpaceL
.trimLeft:
	inc ebx
	
	mov byte[esi-1], 0		;Fill spaces with 0

	loop .trimFromLeft

.noWhiteSpaceL:

	pop ecx				;Restore string length
	pop esi				;Restore string position
	push esi		
	push ecx

	mov edi, esi	
	add esi, ebx			;Add total white spaces
	rep movsb			;Move string to new position

	
	pop ecx
	sub ecx, ebx
	pop esi
	add esi, ecx
	dec esi

	std				;Set direction to decrement to go from right to left
.trimFromRight:
	lodsb
	cmp al, ' '
	je .trimRight
	jmp short .noWhiteSpaceR
.trimRight:

	mov byte[esi+1], 0		;Fill spaces with 0

	loop .trimFromRight
	jmp .end

.noWhiteSpaceR:

.end:
	cld
	pop es
	pop edi
	pop esi
	pop ecx
	pop ebx
	pop eax
	ret

;________________________________________________
;Convert ASCII decimal integer to integer
;IN:	esi String
;
;OUT:	cf Sets on incorrect number
;	eax Integer
;
stringToInt:
	push ebx
	push ecx
	push edx
	push esi
	
	mov dword[.number], 0
	
	mov al, '-'
	call stringFindChar
	cmp eax, 1
	ja .negative
	
.positive:	
	mov byte[.negativeFlag], 0
	jmp .next
	
.negative:
	inc esi
	mov byte[.negativeFlag], 1
.next:

	call stringLength		;Find string length	
	mov ecx, eax			;Use length as counter of loop
	add esi, eax			;and also to reach end of string
	dec esi
	
	mov ebx, 0
	mov eax, 1
.convertLoop:
	mov bl, byte[esi]
	dec esi
	
	sub bl, 0x30
	
	cmp bl, 9
	ja .wrongNumber

	mov edx, 10
	mul edx
	push eax
	mul ebx

	add dword[.number], eax
	pop eax
	
	loop .convertLoop

	mov ebx, 10
	mov eax, dword[.number]
	mov edx, 0
	div ebx				;Divide by 10
	mov dword[.number], 0		

.successful:
	cmp byte[.negativeFlag], 0
	je .end1
	neg eax
.end1:
	clc
	jmp short .end

.wrongNumber:
	mov eax, 0
	stc
.end:
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret
.number: dd 0
.negativeFlag: db 0

;________________________________________________
;Find a particular character in a string
;IN:	ESI	String
;	AL	Char to search
;OUT:	CF	sets on char not found
;	EAX	number of char occurrence
;
stringFindChar:
	push ebx
	push ecx
	push edx
	push esi	
	
	mov bl, al
	xor ecx, ecx

.charFindLoop:
	lodsb
	
	or al, al	;cmp al, 0 (last character)
	jz .next
	
	cmp al, bl	;Character found
	jne .charFindLoop
	
	inc ecx		;Counter
	jmp .charFindLoop
	
.next:
	mov eax, ecx
	
	or eax, eax	;cmp edx, 0
	jz .notFound
	
	clc
	jmp .end

.notFound:
	stc
.end:
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret
	
;________________________________________________
;Remove a character from specific position of string.
;IN:  esi string
;     eax character position
;OUT: nothing
;
stringRemoveChar:
	push esi
	push edx
	
	mov edx, eax
	
	call stringLength
	cmp edx, eax		;Eax is string Length
	ja .end
	
	inc eax			;Including last null character
	add esi, edx
	
	push es
	
	push ds			;Make sure ds = es
	pop es
	
	mov edi, esi
	inc esi			;Next character
	
	mov ecx, eax
	cld			;Clear direction
	rep movsb		;mov ecx characters form esi to edi
	
	pop es
	
	pop edx
	pop esi
.end:
	ret
	
;________________________________________________
;Insert a character into specific position of string.
;IN:  esi string
;     edx character position
;     al character to insert
;OUT: nothing
;
;NOTE: Make sure sufficinet space in string's buffer
;
stringInsertChar:
	push eax
	push ebx
	push ecx
	push edi
	
	mov ebx, eax		;Save character
	push esi

;Make space for char to insert
;	
	call stringLength
	push eax		;eax is string length
	
	add esi, eax
	inc esi			;Including null char
	
	push es
	
	push ds			;Make sure es = ds
	pop es
	
	std			;Reverse direction in rep movsb
	add esi, edx
	dec esi
	
	mov edi, esi
	dec esi
	
	mov ecx, eax
	
	rep movsb		;mov ecx characters from esi to edi	
	
	pop es
	
	pop eax
	pop esi
	
;Insert character now
;
	mov byte[esi+edx], bl	;BL is character
	mov byte[esi+eax+1],0	;Make sure end of string
	cld

	pop edi
	pop ecx
	pop ebx
	pop eax
	ret
