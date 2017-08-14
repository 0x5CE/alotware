;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;          Application Programming Interface		  ;
;---------------------------------------------------------;

	use32

;________________________________________________
;Alotware main system call interface
;
;IN: 	System call number on Stack (see "apps/alotware.inc"),
;	Depands upon system call's Input
;OUT: 	Depands upon system call's Output
;	EBP = 0x12345678 on call not available
	
APIint100:
	push ebp
	mov ebp, esp
	
	push 0x10		;Kernel's segment
	pop ds

	mov [.es], es
	
	push 0x18
	pop es
	cld
	
	mov dword[.eax], eax
	
	add esi, dword[progAddress]
	sub esi, 0x500
	add edi, dword[progAddress]
	sub edi, 0x500

	pop eax			;Clear stack
	mov dword[.ebp], eax
	
	pop eax
	mov dword[.eip], eax

	pop eax
	mov dword[.cs], eax

	pop eax			;Flag register

	pop eax
	mov dword[.param], eax	;System call

	mov eax, dword[.eax]

	mov ebp, dword[ds:.param]
	
	cmp ebp, dword[.totalCalls]
	ja .callNotAvailable
	
	sti
	call dword[.table+ebp*4]
.end:
	sti
	pushfd
	push dword[.cs]
	push dword[.eip]
	
	sub esi, dword[progAddress]
	add esi, 0x500
	sub edi, dword[progAddress]
	add edi, 0x500

	mov es, [.es]
	
	push 0x38
	pop ds

	iret
	
.callNotAvailable:
	mov ebp, 0x12345678
	jmp .end
	
.eflag:	dd 0
.param:	dd 0
.eax:	dd 0
.cs:	dd 0
.es:	dw 0
.eip:	dd 0
.ebp: 	dd 0

.totalCalls: dd 51	

;-----------System calls---------------;
;				       ;
;--------------------------------------;
.table:
	dd null
	dd printString
	dd printIntDec
	dd printIntBin
	dd printIntHex
	dd clearScreen
	dd clearLine
	dd setGraphicsMode
	dd setTextMode
	dd scrollDown
	dd setCursor
	dd waitKeyboard
	dd stringLength
	dd stringCompare
	dd stringUpperCase
	dd stringLowerCase
	dd stringTrim
	dd stringFindChar
	dd stringToInt
	dd filesList
	dd fileLoad
	dd fileExists
	dd .loadProgram
	dd terminate
	dd memoryAllocate
	dd memoryDeallocate
	dd .installISR
	dd waitMouse
	dd graphicsPutPixel
	dd .graphicsDrawBlock
	dd putChar
	dd getString
	dd fileNew
	dd fileSave
	dd stringWordsCompare
	dd fileDelete
	dd getCursor
	dd stringRemoveChar
	dd stringInsertChar
	dd text.setColor
	dd text.getColor
	dd putCharO
	dd getScreenInfo
	dd useVideoBuffer2
	dd useVideoBuffer1
	dd screenRefresh
	dd getKeysStatus
	dd rand
	dd srand
	dd mouseGet
	dd mouseSet
	dd reboot	

.installISR:
	cli
	call installISR
	ret
	
.loadProgram:
	push dword[.eip]
	push dword[.cs]
	call loadProgram
	pop dword[.cs]
	pop dword[.eip]
	ret
	
.graphicsDrawBlock:

	sub esi, dword[progAddress]
	add esi, 0x500
	sub edi, dword[progAddress]
	add edi, 0x500

	call graphicsDrawBlock

	add esi, dword[progAddress]
	sub esi, 0x500
	add edi, dword[progAddress]
	sub edi, 0x500
	ret
	
null:	mov ebp, 0x12345678
	ret
