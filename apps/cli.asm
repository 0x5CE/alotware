;---------------------------------------------------------;
;                    Command-line Shell		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;						          ;
;---------------------------------------------------------;

include "alotware.inc"
use32

start:	
	os getScreenInfo
	jc .textMode
	
	mov byte[isGraphicsMode], 1
	jmp .screenOK
	
.textMode:
	mov byte[isGraphicsMode], 0
	
.screenOK:
	mov byte[maxCol], bl
	
.getCommandLoop:	
	newLine
	
	os getCursor
	
	push edx
	
	mov eax, 0xffffffff
	mov ebx, 0xff828282
	os text.setColor
	
	mov al, 0
	os clearLine
	
	mov esi, osMsg
	os printString
	
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	os text.setColor		
	
	pop edx
	
	os setCursor
	
	mov eax, 0xff00f000		;Green
	mov ebx, 0xffffffff		;White
	os text.setColor
	
	mov esi, prompt
	os printString
	
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	os text.setColor
	
	mov al, byte[maxCol]		;Maximum characters to get
	sub al, 20
	
	os getString
	
	os stringTrim			;remove extra White-Spaces
	os stringLowerCase		;Convert string to lower case
	
	cmp byte[esi], 0		;No command entered
	je .getCommandLoop
	
;Compare commands
;
	;CLS command
	mov edi, commands.cls		
	os stringWordsCompare		;Compare string EDI with ESI
	jc .doCLS

	;EXIT command
	mov edi, commands.exit		
	os stringWordsCompare		;Compare string EDI with ESI
	jc .endCLI

	;ALOTWARE command
	mov edi, commands.osName		
	os stringWordsCompare		;Compare string EDI with ESI
	jc .osCommand

	;GUI command
	mov edi, commands.gui		
	os stringWordsCompare		;Compare string EDI with ESI
	jc .guiCommand

	;LIST command
	mov edi, commands.list		
	os stringWordsCompare		;Compare string EDI with ESI
	jc .listCommand

	;JMP $ command
	mov edi, commands.jmpDollor		
	os stringWordsCompare		;Compare string EDI with ESI
	jc .jmpDollorCommand

	;HELP command
	mov edi, commands.help		
	os stringWordsCompare		;Compare string EDI with ESI
	jc .helpCommand
	
	;DISPLAY command
	mov edi, commands.display
	os stringWordsCompare
	jc .displayCommand


	;DELETE command
	mov edi, commands.delete
	os stringWordsCompare
	jc .deleteCommand

	;REBOOT command
	mov edi, commands.reboot
	os stringWordsCompare
	jc .rebootCommand

;Try to load the program
;	
	call getArguments		;Separate argument and command
	
	push esi
	push edi
	
	os stringLength
	add esi, eax
	sub esi, 4
	mov edi, progExtension
	os stringWordsCompare		;Check for .BIN extension
	jc .loadProgram
	
	pop edi
	pop esi
	
.noExtension:
		
;Try to add extesion
;
	os stringLength
	mov ebx, eax

	mov al, byte[progExtension+0]
	mov byte[esi+ebx+0], al
	
	mov al, byte[progExtension+1]
	mov byte[esi+ebx+1], al
	
	mov al, byte[progExtension+2]
	mov byte[esi+ebx+2], al
	
	mov al, byte[progExtension+3]
	mov byte[esi+ebx+3], al
	
	mov byte[esi+ebx+4], 0		;End of string
	
	push esi
	push edi
	jmp .loadProgram
	
.notFound:

	os getCursor
	
	mov dl, byte[maxCol]		;Maximum characters to get
	sub dl, 17
	
	os setCursor
	
	mov eax, 0xffff0000
	mov ebx, 0xffffffff
	os text.setColor
	
	mov esi, wrongCommand
	os printString
	
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	os text.setColor
	
	jmp .getCommandLoop	

.deleteCommand:

	add esi, 6			;Length of command name
	os stringTrim
	
	os fileDelete
	
	jmp .getCommandLoop

.displayCommand:

	newLine
	
	add esi, 7			;Length of command name
	os stringTrim
	
	mov edi, fileBuffer
	os fileLoad
	jc .fileNotFound
	
	mov esi, fileBuffer
	os printString
	jmp .getCommandLoop	
	
.fileNotFound:
	mov esi, commands.display.fileNotFound
	os printString
	jmp .getCommandLoop
		
.jmpDollorCommand:

	sti
	jmp $
	
	jmp .getCommandLoop

.loadProgram:
	
	pop edi

	mov esi, edi
	os stringTrim
	
	pop esi
	
	mov eax, edi
	
	stc
	os loadProgram
	jc .notFound
	
	jmp .getCommandLoop

.helpCommand:

	newLine
	
	mov esi, commands.help.helpMessage
	os printString
	
	mov dh, al
	
	jmp .getCommandLoop

.rebootCommand:
	
	os reboot
	jmp .getCommandLoop	

.listCommand:

	os filesList			;Get files list in esi
	push eax			;Total files
	
	newLine
	
	os printString
	
	newLine

	mov esi, commands.list.total
	os printString
	
	pop eax				;Total files
	os printIntDec
	
	jmp .getCommandLoop

.guiCommand:

	cmp byte[isGraphicsMode], 1
	je .alreadyGraphicsMode
;Set graphics mode
;
	os setGraphicsMode

.alreadyGraphicsMode:

;Draw a pointer
;
	mov edx, 0x9f0f9f	;Color
	mov eax, 0
	mov ebx, 0
	mov esi, 10		;Width
	mov edi, 10		;Height
	os graphicsDrawBlock
	
.mouseLoop:
	os waitMouse
	push edx

;Clear previous pointer
;
	push eax
	push ebx
	mov ax, word[.mouse.lastX]
	mov bx, word[.mouse.lastY]
	
	mov edx, 0xffffff	;Color

	mov esi, 10		;Width
	mov edi, 10		;Height

	os graphicsDrawBlock
	
	mov edx, 0x9f0f9f	;Color

	pop ebx
	pop eax

	mov word[.mouse.lastX], ax
	mov word[.mouse.lastY], bx

;Draw new pointer
;
	mov esi, 10		;Width
	mov edi, 10		;Height
	os graphicsDrawBlock

	pop edx
	bt dx, 0		;If left button pressed, exit
	jc .mouseLoop.end

	mov al, 0
	os clearLine
	
	os mouseGet
	push ebx
	os printIntDec

	mov al, ','
	os putChar
	mov al, ' '
	os putChar
	
	pop eax
	os printIntDec
	
	
	jmp .mouseLoop

.mouse.lastX: dw 0
.mouse.lastY: dw 0

.mouseLoop.end:	

	mov word[.mouse.lastX], 0
	mov word[.mouse.lastY], 0

	cmp byte[isGraphicsMode], 0
	je .mouseLoop.end.textMode
	jmp .getCommandLoop
	
.mouseLoop.end.textMode:
	os setTextMode
	jmp .getCommandLoop
	
.osCommand:
	jmp .getCommandLoop

.doCLS:	
	os clearScreen
	
	jmp .getCommandLoop

.endCLI:

	os terminate
	
	jmp .getCommandLoop
	
	os waitKeyboard
	os terminate

	
;----------Data-----------;

commands:
.display:	db 'display', 0
.display.fileNotFound:
		db 'File not found!', 0
.delete:	db 'delete', 0
.exit:		db 'exit',0
.gui:		db 'gui',0
.list:		db 'list',0
.list.total:	db 'Total files : ', 0
.cls:		db 'cls',0
.osName:	db 'alotware',0
.jmpDollor:	db 'jmp $', 0
.help:		db 'help', 0
.reboot:	db 'reboot', 0
.help.helpMessage:
		db 'Available commands:', 10, 10
		db '    LIST    --   Display list of files', 10
		db '    HELP    --   Display this help message', 10
		db '    CLS     --   Clear screen', 10
		db '    GUI     --   GUI demo', 10
		db '    EXIT    --   Exit CLI', 10
		db '    DISPLAY --   Display the content of a file', 10
		db '    DELETE  --   Delete a file from the disk', 10
		db '	REBOOT	--   Reboot the computer', 10
		db '    JMP $   --   Infinate Loop', 10, 10
		db "Type application's name to execute.", 10, 0

wrongCommand:	db '  Wrong command!',0
progExtension:  db '.bin', 0	;Default extension of Alotware application
prompt: 	db '>> ', 0
osMsg:		db 'Alotware. Written by: Muazzam Ali Kazmi.', 0

;------------Functions------------;

;________________________________________________
;Separate command name and arguments
;IN:  esi command address
;OUT: esi command name
;     edi command arguments
;     CF sets on no extension
;
getArguments:
	push esi
.loop:
	lodsb			;mov al, byte[esi] & inc esi
	
	cmp al, 0
	je .notFound
	
	cmp al, ' '
	je .spaceFound
	
	jmp .loop
	
.notFound:
	pop esi
	mov edi, 0
	stc
	jmp .end

.spaceFound:
	mov byte[esi-1], 0
	mov ebx, esi
	
	os stringLength
	mov ecx, eax
	inc ecx			;Including last null char
	
	push es
	
	push ds
	pop es
	
	mov esi, ebx
	mov edi, fileBuffer
	rep movsb		;Copy ecx char string from esi to edi
	
	pop es
	
	mov edi, fileBuffer
	
	pop esi
	clc
.end:
	ret
maxCol:	db 0
isGraphicsMode: db 0
fileBuffer:
