;---------------------------------------------------------;
;                   Simple Text Editor 		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;						          ;
;---------------------------------------------------------;

include "alotware.inc"
use32

start:
;__________ Initialize __________;

	os getScreenInfo
	
	mov byte[maxCol], bl
	mov byte[maxRow], bh
	
	cmp byte[edi], 0			;If no arguments
	je .newFile
	
	mov esi, edi				;Program arguments
	os stringLength
	
	cmp eax, 12				;Wrong file name
	ja .newFile
	
;Save file name
;
	push es
	
	push ds
	pop es					;Make sure es = ds
	
	mov edi, fileName
	mov ecx, eax				;Characters in file name
	inc ecx					;Including null char
	rep movsb
	
	pop es
	
;Read file
;	
	mov esi, fileName
	os fileExists
	jc .newFile				;File does not exists
	
	mov esi, fileName
	mov edi, fileBuffer			;Address to load
	os fileLoad
	
	mov esi, fileName
	os stringLength
	mov ecx, eax

;Add file name in title
;
	push es
	
	push ds
	pop es					;Make sure es = ds
	
	mov esi, fileName
	mov edi, title+14
	rep movsb
	
	pop es
	
	jmp .begin	
	
.newFile:
	mov byte[fileName], 0
	
;Add 'New file' in title
;
	push es
	
	push ds
	pop es					;Make sure es = ds
	
	mov ecx, 9
	
	mov esi, newFileTitle
	mov edi, title+14
	rep movsb
	
	pop es
	
.begin:
	mov al, 10				;New line character
	mov esi, fileBuffer
	os stringFindChar
	mov dword[totalLines], eax
	
	mov dword[currentLinePosition], 0
	
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line
	
	mov byte[cursorPositionInLine], dl	;Cursor at end of line
	mov byte[currentLineLength], dl	;Save current line's length
	
	mov dword[currentPagePosition], 0
	
	jmp .getKey

	
;__________ Render __________;

.getKey:
	cmp byte[needRedraw], 0
	je .otherLinesPrinted			;No need to print other lines
	
;Print other lines
;
	os useVideoBuffer2			;Enable double buffering
	os clearScreen
	
	mov eax, dword[totalLines]
	cmp dword[currentLine], eax
	je .otherLinesPrinted
	
.printOtherLines:
	
	mov esi, fileBuffer
	add esi, dword[currentPagePosition]
	
	newLine
	
	movzx ecx, byte[maxRow]
	sub ecx, 2
.printOtherLinesLoop:
	call printLine
	jc .printTitle
	newLine
	loop .printOtherLinesLoop

.printTitle:

;Print title on top and footer at bottom
;
	mov eax, 0xffffffff
	mov ebx, 0xff828282
	os text.setColor
	
	mov al, 0
	os clearLine
	
	mov esi, title
	os printString
	
	mov al, byte[maxRow]		;Last line
	dec al
	os clearLine
	
	mov esi, footer
	os printString
	
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	os text.setColor
	
;Refresh screen
.refreshBuffer:
	os screenRefresh
	os useVideoBuffer1		;Disable double buffering
	
.otherLinesPrinted:
	
	mov byte[needRedraw], 0

	mov dl, 0
	mov dh, byte[linePositionOnScreen]
	os setCursor
		
;Print current line
;	
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call printLine
	
	mov al, ' '
	os putChar
	
;Print current line and column number
;
	mov eax, 0xffffffff
	mov ebx, 0xff828282
	os text.setColor

	mov dl, byte[maxCol]
	sub dl, 20
	mov dh, byte[maxRow]
	dec dh
	os setCursor

	mov esi, lnMsg
	os printString
	
	mov eax, dword[currentLine]
	inc eax					;Counting from 1
	os printIntDec
	
	mov al, ','
	os putChar
	mov al, ' '
	os putChar
	
	mov esi, colMsg
	os printString
	
	movzx eax, byte[cursorPositionInLine]
	inc eax					;Counting from 1
	os printIntDec
	
	mov al, ' '
	os putChar
	
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	os text.setColor

.next1:

;Set cursor at current position in line
;
	mov dl, byte[cursorPositionInLine]
	mov dh, byte[linePositionOnScreen]
	os setCursor

;__________ Ready to get key __________;
	
.readyForKey:
	os waitKeyboard
	
	push eax
	os getKeysStatus
	bt eax, 0
	jc .controlKeys
	pop eax
	
	cmp al, 10
	je .returnKey
	
	cmp al, 9
	je .printableChar
	
	cmp ah, 71		;Scancode
	je .homeKey
	
	cmp ah, 79
	je .endKey
	
	cmp ah, 14		
	je .backSpace
		
	cmp ah, 83
	je .deleteKey
	
	cmp ah, 75
	je .left
	
	cmp ah, 77
	je .right
	
	cmp ah, 72
	je .up
	
	cmp ah, 80
	je .down
	
	cmp ah, 81
	je .pageDown
	
	cmp ah, 73
	je .pageUp
	
;If not printable character
;
	cmp al, ' '
	jl .getKey
	cmp al, '~'
	ja .getKey

;__________ Other Key __________;

.printableChar:
	
;No more than 79 characters per line are supported
;
	mov bl, byte[maxCol]
	dec bl
	cmp byte[currentLineLength], bl
	jae .getKey
	
	mov edx, 0
	movzx esi, byte[cursorPositionInLine]	;Position to insert char
	add esi, dword[currentLinePosition]
	add esi, fileBuffer
	
	os stringInsertChar			;Insert char into string
	
	inc byte[cursorPositionInLine]		;One character is added
	inc byte[currentLineLength]

;More keys
;
	jmp .getKey

;__________ Return or Enter Key __________;

.returnKey:
	mov byte[needRedraw], 1
	
	mov edx, 0
	
	movzx esi, byte[cursorPositionInLine]
	add esi, fileBuffer
	add esi, dword[currentLinePosition]
	
	mov al, 10
	os stringInsertChar
	
;Next line
;
	inc dword[currentLine]
	
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	mov dword[currentLinePosition], esi

;Calculate values for this line
;
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line
	
	mov byte[cursorPositionInLine], 0	;Cursor at end of line
	mov byte[currentLineLength], dl		;Save current line's length

	mov al, 10				;New line character
	mov esi, fileBuffer
	os stringFindChar
	mov dword[totalLines], eax
	
;Try to move cursor down
;
	mov bl, byte[maxRow]
	sub bl, 2
	cmp byte[linePositionOnScreen], bl
	jb .returnKey.cursorNextLine
	
;If at last line, Scroll Down
;
	mov bl, byte[maxRow]
	sub bl, 2
	mov byte[linePositionOnScreen], bl
	
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	movzx ebx, byte[maxRow]
	sub bl, 3
	sub eax, ebx
	call linePosition
	jc .getKey
	
	sub esi, fileBuffer
	mov dword[currentPagePosition], esi
		
	jmp .getKey
	
.returnKey.cursorNextLine:
	
	inc byte[linePositionOnScreen]
	
	jmp .getKey

;__________ Control Keys __________;
.controlKeys:
	pop eax
	
	cmp al, 's'
	je .controlSKey
	cmp al, 'S'
	je .controlSKey
	
	cmp al, 'q'
	je endProgram
	cmp al, 'Q'
	je endProgram
	
	jmp .getKey
	
;__________ Backspace Key __________;

.backSpace:
	
;If at first column, do nothing
;
	cmp byte[cursorPositionInLine], 0
	je .backSpace.firstColumn

;Remove character from left
;
	movzx eax, byte[cursorPositionInLine]
	add eax, dword[currentLinePosition]
	dec eax

	mov esi, fileBuffer
	os stringRemoveChar
	dec byte[cursorPositionInLine]	;One character is removed
	dec byte[currentLineLength]

	jmp .getKey

.backSpace.firstColumn:
	
	cmp byte[currentLine], 0
	je .getKey

;Calculate previus line's length
;
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	dec eax					;Previous line
	call linePosition
	jc .getKey

	sub esi, fileBuffer
	mov edx, 0
	add esi, fileBuffer
	call lineLength				;Find length
	push edx				;Save line's length
	
	add dl, byte[currentLineLength]
	
;Backspace not allowed (line's length is limited to 79 characters)
;
	mov bl, byte[maxCol]
	dec bl
	cmp dl, bl				;Couting from 0
	jae .getKey

;Remove new line character
;
	mov byte[needRedraw], 1
	
	movzx eax, byte[cursorPositionInLine]
	add eax, dword[currentLinePosition]
	dec eax

	mov esi, fileBuffer
	os stringRemoveChar
	
	dec byte[totalLines]			;One line is removed
	dec dword[currentLine]

;Previous line
;
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	push esi
	
;Calculate values for this line
;
	mov edx, 0
	pop esi
	push esi
	add esi, fileBuffer
	call lineLength				;Find length of current line
	
	mov byte[currentLineLength], dl		;Save line's length
	pop dword[currentLinePosition]
	
	pop edx
	mov byte[cursorPositionInLine], dl
	
	jmp .up.cursorMoved

;__________ Delete Key __________;

.deleteKey:

;If at last column, do nothing
;
	mov dl, byte[currentLineLength]
	cmp byte[cursorPositionInLine], dl
	jae .getKey

	movzx eax, byte[cursorPositionInLine]
	add eax, dword[currentLinePosition]
	mov esi, fileBuffer
	os stringRemoveChar
	dec byte[currentLineLength]	;One character is removed
	inc byte[cursorPositionInLine]

;__________ Left arrow Key __________;

.left:

;If at first column, do nothing
;
	cmp byte[cursorPositionInLine], 0
	jne .left.moveLeft
	
	cmp byte[currentLine], 0
	je .getKey
	
	mov bl, byte[maxCol]
	mov byte[cursorPositionInLine], bl
	jmp .up
	
;Move cursor left
;
.left.moveLeft:
	dec byte[cursorPositionInLine]
	
	jmp .getKey

;__________ Right arrow Key __________;

.right:

;If at last column, do nothing
;
	mov dl, byte[currentLineLength]
	cmp byte[cursorPositionInLine], dl
	jnae .right.moveRight

;Next line not allowed
;	
	mov eax, dword[currentLine]
	inc eax
	cmp dword[totalLines], eax
	je .getKey
	
;Next line
;	
	inc dword[currentLine]
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	mov dword[currentLinePosition], esi
	
;Calculate values for this line
;
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line
	
	mov byte[cursorPositionInLine], 0	;Cursor at end of line
	mov byte[currentLineLength], dl		;Save current line's length
	
	jmp .down.next
	
;Move cursor right
;
.right.moveRight:

	inc byte[cursorPositionInLine]
	jmp .getKey

;__________ Up arrow Key __________;

.up:

;Previous line not allowed
;	
	cmp dword[currentLine], 0
	je .getKey
	
;Previous line
;
	dec dword[currentLine]
	
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	mov dword[currentLinePosition], esi

;Calculate values for this line
;
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line
	
	mov byte[currentLineLength], dl		;Save current line's length
	
	cmp dl, byte[cursorPositionInLine]
	jb .up.moveCursorAtEnd
	
	jmp .up.cursorMoved			;Don't change cursor Column
	
.up.moveCursorAtEnd:
	mov byte[cursorPositionInLine], dl	;Cursor at end of line
	
.up.cursorMoved:

;Try to move cursor Up
;
	cmp byte[linePositionOnScreen], 1
	ja .up.cursorPreviousLine
	
;If cursor is at first line, Scroll Up
;
	mov byte[linePositionOnScreen], 1
	mov eax, dword[currentLinePosition]
	mov dword[currentPagePosition], eax
	
	mov byte[needRedraw], 1
	
	jmp .getKey

.up.cursorPreviousLine:
	
	dec byte[linePositionOnScreen]
	
	jmp .getKey
	
;__________ Down arrow Key __________;

.down:

;Next line not allowed
;	
	mov eax, dword[currentLine]
	inc eax
	cmp dword[totalLines], eax
	je .getKey
	
;Next line
;	
	inc dword[currentLine]
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	mov dword[currentLinePosition], esi
	
;Calculate values for this line
;
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line
	
	mov byte[currentLineLength], dl		;Save current line's length
	
	cmp dl, byte[cursorPositionInLine]
	jb .down.moveCursorAtEnd
	
	jmp .down.cursorMoved			;Don't change cursor Column
	
.down.moveCursorAtEnd:
	mov byte[cursorPositionInLine], dl	;Cursor at end of line
	
.down.cursorMoved:

.down.next:

;Try to move cursor down
;
	mov bl, byte[maxRow]
	sub bl, 2
	cmp byte[linePositionOnScreen], bl
	jb .down.cursorNextLine
	
;If at last line, Scroll Down
;
	mov bl, byte[maxRow]
	sub bl, 2
	mov byte[linePositionOnScreen], bl
	
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	movzx ebx, byte[maxRow]
	sub bl, 3
	sub eax, ebx
	call linePosition
	jc .getKey
	
	sub esi, fileBuffer
	mov dword[currentPagePosition], esi
	
	mov byte[needRedraw], 1
	
	jmp .getKey
	
.down.cursorNextLine:

	inc byte[linePositionOnScreen]
	
	jmp .getKey

;__________ Home Key __________;

.homeKey:

;Move cursor at first Column
;
	mov byte[cursorPositionInLine], 0
	jmp .getKey

;__________ End Key __________;

.endKey:

;Move cursor at last column
;
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength
	mov byte[cursorPositionInLine], dl
	jmp .getKey
	
;__________ Page Up (pg up) Key __________;

.pageUp:
	
	mov eax, dword[currentLine]
	movzx ebx, byte[maxRow]
	sub bl, 3
	sub eax, ebx
	cmp eax, 0
	jle .pageUp.gotoFirstLine

;No redraw if on last line
;	
	mov bl, byte[maxRow]
	sub bl, 2
	cmp byte[linePositionOnScreen], bl
	jae .pageUp.noNeedToRedraw
	
	mov byte[needRedraw], 1
	
.pageUp.noNeedToRedraw:
	
;Previous line
;
	movzx ebx, byte[maxRow]
	sub bl, 3
	sub dword[currentLine], ebx
	
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	mov dword[currentLinePosition], esi

;Calculate values for this line
;
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line
	
	mov byte[cursorPositionInLine], dl	;Cursor at end of line
	mov byte[currentLineLength], dl		;Save current line's length
		
.pageUp.end:
	mov byte[linePositionOnScreen], 1
	mov eax, dword[currentLinePosition]
	mov dword[currentPagePosition], eax
	
	jmp .getKey

.pageUp.gotoFirstLine:
	
;Page Up not allowed
;	
	cmp dword[currentLine], 0
	je .getKey
	
	mov byte[needRedraw], 1
	
	mov esi, fileBuffer
	mov eax, 0
	mov dword[currentLine], eax
	call linePosition
	sub esi, fileBuffer
	mov dword[currentLinePosition], esi
	
;Calculate values for this line
;	
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line
	
	mov byte[cursorPositionInLine], dl	;Cursor at end of line
	mov byte[currentLineLength], dl		;Save current line's length
	
	jmp .pageUp.end

;__________ Page Down (pg dn) Key __________;

.pageDown:
	
	mov eax, dword[currentLine]
	movzx ebx, byte[maxRow]
	sub bl, 3
	add eax, ebx
	cmp eax, dword[totalLines]
	jae .pageDown.gotoLastLine
	
	
;No redraw if on first line
;	
	cmp byte[linePositionOnScreen], 1
	jle .pageDown.noNeedToRedraw
	
	mov byte[needRedraw], 1
	
.pageDown.noNeedToRedraw:
	
;Next line
;
	movzx ebx, byte[maxRow]
	sub bl, 3
	add dword[currentLine], ebx
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	mov dword[currentLinePosition], esi

;Calculate values for this line
;
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line

	mov byte[cursorPositionInLine], dl	;Cursor at end of line
	mov byte[currentLineLength], dl		;Save current line's length

	mov bl, byte[maxRow]
	sub bl, 2
	mov byte[linePositionOnScreen], bl

	mov eax, dword[currentLine]
	movzx ebx, byte[maxRow]
	sub bl, 3
	sub eax, ebx
	mov esi, fileBuffer
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	
	mov dword[currentPagePosition], esi
	
	jmp .getKey

.pageDown.gotoLastLine:

;Page Down not allowed
;	
	mov eax, dword[currentLine]
	inc eax
	cmp eax, dword[totalLines]
	jae .getKey

	mov byte[needRedraw], 1

;Next line
;
	mov eax, dword[totalLines]		;last line is total lines - 1
	dec eax
	mov dword[currentLine], eax		;Make last line, current line
	
	mov esi, fileBuffer
	mov eax, dword[currentLine]
	
	call linePosition
	jc .getKey
	
	sub esi, fileBuffer
	mov dword[currentLinePosition], esi

;Calculate values for this line
;
	mov edx, 0
	mov esi, fileBuffer
	add esi, dword[currentLinePosition]
	call lineLength				;Find length of current line

	mov byte[cursorPositionInLine], dl	;Cursor at end of line
	mov byte[currentLineLength], dl		;Save current line's length
	
	movzx ebx, byte[maxRow]
	sub ebx, 3
	cmp dword[totalLines], ebx		;Check for small or large file
	jae .moreThanOnePages
	
;If small file
;
	mov ebx, dword[totalLines]
	dec ebx

;If large file
;

.moreThanOnePages:
	
	inc bl
	
	mov byte[linePositionOnScreen], bl
	
	mov eax, dword[currentLine]
	sub eax, ebx
	inc eax
	
	mov esi, fileBuffer
	call linePosition
	jc .getKey
	sub esi, fileBuffer
	
	mov dword[currentPagePosition], esi
	
	jmp .getKey
	
;__________ Control+S Key __________;
.controlSKey:
	call saveFile
	jmp .next1
	
;__________ Control+Q Key __________;
endProgram:
;	call saveFile
	os scrollDown
	os terminate
	jmp $
	
;__________ Data & Functions __________;

;.Data
;

totalLines:		dd 0	;Count of lines in file
currentLine:		dd 0	;Current line in file
currentLinePosition:	dd 0	;Position of current line in whole file
cursorPositionInLine:	dd 0	;Position of cursor in current line
currentLineLength:	dd 0	;Length of current line
linePositionOnScreen:	dd 1	;Position of line in dispaly or screen
currentPagePosition:	dd 0	;Position of current page in file (one screen)

needRedraw:		db 1	;If non-zero then need to redraw whole screen

fileName:		times 13 db 0

maxRow:			db 0	;Total rows per line
maxCol:			db 0	;Total columns

lnMsg: db 'ln:', 0
colMsg: db 'col:', 0

fileSavedMsg: db 'File saved.', 0
fileNameMsg: db 'File name: ', 0
newFileTitle: db 'New file', 0

footer:	db '[^Q] Exit  [^S] Save', 0
title:	db 'Text editor -                      ', 0

;.Functions
;

;________________________________________________
;Print one line from string
;IN:  esi buffer address
;OUT: esi next buffer
;     Carry sets on whole file ended
;
printLine:
	mov edx, 0		;Characters counter
.printLoop:

	lodsb
	
	cmp al, 10		;End of line
	je .end
	cmp al, 0		;End of string
	je .fileEnded
	
	movzx ebx, byte[maxCol]
	dec bl
	
	cmp edx, ebx
	jae .lineLengthMax
	
	pushad
	os putCharO		;Print character AL
	popad
	
	inc edx
	jmp .printLoop		;More characters
	
.lineLengthMax:
	jmp .printLoop
	
.fileEnded:
	stc
.end:	
	ret

;________________________________________________
;Find line length
;IN:  esi buffer address
;OUT: esi next buffer
;     edx += line length
;
lineLength:
	
	mov al, byte[esi]
	inc esi
	
	cmp al, 10		;End of line
	je .end
	cmp al, 0		;End of string
	je .end
	
	inc edx
	
	jmp lineLength		;More characters

.end:	
	ret
	
;________________________________________________
;Find address of line in string
;IN:  esi String
;     eax Line number (counting from 0)
;OUT: esi Position of line in string
;     Carry sets on line not found
;
linePosition:
	push ebx
	
	cmp eax, 0
	je .requiredLineFound	;Already at first line
	
	mov edx, 0		;Lines counter
	mov ebx, eax		;Save line
	dec ebx
	
.nextChar:	
	mov al, byte[esi]
	inc esi
	
	cmp al, 10		;New line character
	je .lineFound
	
	cmp al, 0		;End of string
	je .lineNotFound
	
	jmp .nextChar
	
.lineFound:
	cmp edx, ebx
	je .requiredLineFound
	
	inc edx			;Lines counter
	jmp .nextChar
	
.requiredLineFound:
	clc
	jmp .end
	
.lineNotFound:
	stc
.end:
	pop ebx
	ret

;________________________________________________
;Save file
;IN/OUT: nothing
;
saveFile:
	cmp byte[fileName], 0
	jne .notNewFile

;Get file name
;
	mov eax, 0
	mov ebx, 0xffafafaf
	os text.setColor
	
	mov al, byte[maxRow]
	sub al, 2
	os clearLine

	mov dl, 0
	mov dh, byte[maxRow]
	sub dh, 2
	os setCursor
	
	mov esi, fileNameMsg
	os printString
	
	mov eax, 12				;Maximum characaters
	os getString
	
	os stringLength
	cmp eax, 0
	je .end
	
;Save file name
;
	push es
	
	push ds
	pop es					;Make sure es = ds
	
	mov edi, fileName
	mov ecx, eax				;Characters in file name
	inc ecx					;Including null char
	rep movsb
	
;Add file name in title
;
	mov ecx, eax				;Characters in file name
	inc ecx					;Including null char
	
	mov esi, fileName
	mov edi, title+14
	rep movsb
	
	pop es
	
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	os text.setColor
	
.notNewFile:

;If file already exists, delete it
;
	mov esi, fileName
	os fileDelete
	
;Find file size
;
	mov esi, fileBuffer
	os stringLength
	
;Save now
;
	mov esi, fileName
	mov edi, fileBuffer
	os fileSave

;Display file save message
;
	mov eax, 0
	mov ebx, 0xffafafaf
	os text.setColor
	
	mov al, byte[maxRow]
	sub al, 2
	os clearLine

	mov dl, 0
	mov dh, byte[maxRow]
	sub dh, 2
	os setCursor

	mov esi, fileSavedMsg
	os printString

.end:
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	os text.setColor
	
	mov byte[needRedraw], 1
	ret

;__________ Buffer to load file __________;

fileBuffer: db 10
