;---------------------------------------------------------;
;               (In progress) 2048-like game	 	  ;
;                                                         ;
;         It is similar to 2048 by Gabriele Cirulli       ;
;                 "but not based on that"                 ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;						          ;
;---------------------------------------------------------;

use32
include "alotware.inc"
start:
	os getScreenInfo
	jnc graphics		;If already in graphics mode
	
	os setGraphicsMode
	
graphics:
	os useVideoBuffer2
	mov eax, 0x00f9f6f2
	mov ebx, 0x00f9f6f2
	os text.setColor
	mov dx, 0
	os setCursor
	os clearScreen
	
	call generate2Random
	call generate2Random
	call drawBackground
getKey:		
	call printBoard
	os waitKeyboard
	
	cmp al, 'q'
	je quit
	cmp al, 'Q'
	je quit
	
	cmp ah, 75
	je leftKey
	
	cmp ah, 77
	je rightKey
	
	cmp ah, 72
	je upKey
	
	cmp ah, 80
	je downKey
	
	jmp getKey
	
leftKey:
	call copyBoards
	call rotateBoard	
	
	mov ecx, COLUMNS
.allColumns:
	push ecx
	dec ecx
	call mergeColumnElements
	pop ecx
	loop .allColumns
	
	call rotateBoard
	call rotateBoard
	call rotateBoard
	
	call checkBoard
	jmp getKey
	
rightKey:
	call copyBoards
	call rotateBoard
	call rotateBoard
	call rotateBoard
		
	mov ecx, COLUMNS
.allColumns:
	push ecx
	dec ecx
	call mergeColumnElements
	pop ecx
	loop .allColumns
	
	call rotateBoard
	
	call checkBoard
	jmp getKey
	
upKey:
	call copyBoards
	mov ecx, COLUMNS
.allColumns:
	push ecx
	dec ecx
	call mergeColumnElements
	pop ecx
	loop .allColumns
	
	call checkBoard
	jmp getKey

downKey:
	call copyBoards
	call rotateBoard
	call rotateBoard
	
	mov ecx, COLUMNS
.allColumns:
	push ecx
	dec ecx
	call mergeColumnElements
	pop ecx
	loop .allColumns
	
	call rotateBoard
	call rotateBoard
	
	call checkBoard
	jmp getKey

gameOver:
	mov eax, 0
	mov ebx, 0x00ffffff
	os text.setColor
	os clearScreen
	mov dl, 2
	mov dh, 2
	os setCursor
	mov esi, gameOverStr
	os printString
	os screenRefresh
	os waitKeyboard
	os clearScreen
	
quit:
	os useVideoBuffer1
	mov eax, text.DEFAULT_FOREGROUND
	mov ebx, text.DEFAULT_BACKGROUND
	os text.setColor
	os clearScreen
	
	os terminate
	
;Functions
;

;________________________________________________
;Check if game will end
;IN/OUT: nothing
;
checkGameOver:
	mov ecx, TOTAL_BLOCKS
	mov ebx, board
	
.checkZerosLoop:
	cmp dword[ebx], 0
	je .noGameOver
	add ebx, 4
	loop .checkZerosLoop
	
	call checkEqualELements
	cmp eax, 0
	jne .noGameOver
	
	call rotateBoard
	call checkEqualELements
	
	push eax
	call rotateBoard
	call rotateBoard
	call rotateBoard
	pop eax
	
	cmp eax, 0
	jne .noGameOver
	
	call gameOver
	ret
	
.noGameOver:
	ret
	
;________________________________________________
;Check equal elements in columns
;IN: nothing
;OUT: eax = 0 on no equal elements
;
checkEqualELements:
	mov dword[.r], 0
	mov dword[.c], 0

	mov ecx, COLUMNS
.row:
	push ecx
	mov ecx, ROWS-1
.column:	
	push ecx
	mov al, [.r]
	mov ah, [.c]
	call getElement
	
	mov [.n], eax
	
	inc dword[.r]
	
	mov al, [.r]
	mov ah, [.c]
	call getElement
	
	cmp eax, [.n]
	je .equalFound
	pop ecx
	loop .column
	
	pop ecx
	inc dword[.c]
	mov dword[.r], 0
	loop .row
	
	xor eax, eax
	ret
	
.equalFound:
	pop eax
	pop eax
	mov eax, 1	;non-zero
	ret
	
.n:	dd 0
.c:	dd 0
.r:	dd 0

;________________________________________________
;Generate 2 in a board's random empty element
;IN/OUT: nothing
;
generate2Random:
	call getEmptyElements
	cmp eax, 0
	je gameOver
	
	os rand
	mov esi, emptyElements
	mov eax, [esi+eax*4]
	push eax
	mov eax, 10
	os rand
	cmp eax, 5
	jle .two
	pop eax
	mov dword[eax], 4
	jmp .end
.two:
	pop eax
	mov dword[eax], 2
.end:
	ret
	
;________________________________________________
;Print background
;IN/OUT: nothing
;
drawBackground:
	mov eax, FILLER/2
	mov ebx, FILLER/2
	mov esi, (FILLER/2)+(128*COLUMNS)+(FILLER/2)
	mov edi, (FILLER/2)+(128*ROWS)+(FILLER/2)
	mov edx, 0x00bbada0
	os graphicsDrawBlock
	ret
	
;________________________________________________
;Print the board
;IN/OUT: nothing
;
printBoard:
	
	mov eax, 0x009c9b98
	mov ebx, 0x00f9f6f2
	os text.setColor
	
	mov dl, 3
	mov dh, 0
	os setCursor
	
	mov esi, scoreString
	os printString
	
	mov eax, [score]
	os printIntDec

	xor eax, eax
	mov ecx, ROWS
	
.rows:
	push ecx
	
	mov ecx, COLUMNS
.columns:

	push eax
	push ecx
	
	call printTile

	pop ecx
	pop eax
	
	inc ah
	loop .columns
	
	inc al
	xor ah, ah
	
	pop ecx
	loop .rows
	
	mov dl, 0xff
	mov dh, 0xff
	os setCursor
	
	os screenRefresh
	ret
	
win:
	cmp byte[winf], 1
	je mergeColumnElements.winNext
	
	pushad
	
	mov byte[winf], 1
	
	mov eax, 0x003c3a32
	mov ebx, 0x00f9f6f2
	os text.setColor
	os clearScreen
	mov dl, 2
	mov dh, 2
	os setCursor
	mov esi, gameWinMessage
	os printString
	os screenRefresh
	os waitKeyboard
	os clearScreen
	
	mov eax, 0x00f9f6f2
	mov ebx, 0x00f9f6f2
	os text.setColor
	
	call drawBackground
	popad
	jmp mergeColumnElements.winNext
	
;________________________________________________
;Print a tile
;IN: al row, ah column
;
printTile:
	push eax
	call getElement
	mov [.n], eax
	
	cmp eax, 0
	je .n0
	cmp eax, 2
	je .n2
	cmp eax, 4
	je .n4
	cmp eax, 8
	je .n8
	cmp eax, 16
	je .n16
	cmp eax, 32
	je .n32
	cmp eax, 64
	je .n64
	cmp eax, 128
	je .n128
	cmp eax, 256
	je .n256
	cmp eax, 512
	je .n512
	cmp eax, 1024
	je .n1024
	cmp eax, 2048
	je .n2048

.other:
	mov dword[.c], 0x003c3a32
	mov dword[.f], 0x00f9f6f2
.next:
	pop eax
	
	movzx ebx, al
	movzx eax, ah
	shl eax, 7
	shl ebx, 7
	mov [.x], eax
	mov [.y], ebx
	
	add eax, FILLER
	add ebx, FILLER
	mov esi, TILE_SIZE
	mov edi, TILE_SIZE
	mov edx, [.c]
	os graphicsDrawBlock
	
	cmp dword[.n], 0
	je .notWrite
	
	mov eax, [.f]
	mov ebx, [.c]
	os text.setColor
	
	mov eax, [.x]
	mov ebx, fonts.width
	xor edx, edx
	div ebx
	
	mov dl, al
	add dl, ((TILE_SIZE/2)+FILLER)/fonts.width
	push edx
	
	mov eax, [.y]
	mov ebx, fonts.height
	xor edx, edx
	div ebx
	
	pop edx
	mov dh, al
	add dh, ((TILE_SIZE/2)+FILLER)/fonts.height
	
	os setCursor

	mov eax, [.n]
	os printIntDec
	
.notWrite:
	ret
.n:	dd 0		;Number
.x:	dd 0
.y:	dd 0
.c:	dd 0		;Color (background)
.f:	dd 0		;Color (font)

FILLER = 40

;Colors
;
.n0:
	mov dword[.c], 0x00cbbfb4
	mov dword[.f], 0x00ffffff
	jmp .next
.n2:
	mov dword[.c], 0x00eee4da
	mov dword[.f], 0
	jmp .next
.n4:
	mov dword[.c], 0x00ede0c8
	mov dword[.f], 0
	jmp .next
.n8:
	mov dword[.c], 0x00f2b179
	mov dword[.f], 0x00f9f6f2
	jmp .next
.n16:
	mov dword[.c], 0x00f59563
	mov dword[.f], 0x00f9f6f2
	jmp .next
.n32:
	mov dword[.c], 0x00f67c5f
	mov dword[.f], 0x00f9f6f2
	jmp .next
.n64:
	mov dword[.c], 0x00f65e3b
	mov dword[.f], 0x00f9f6f2
	jmp .next
.n128:
	mov dword[.c], 0x00edcf72
	mov dword[.f], 0x00f9f6f2
	jmp .next
.n256:
	mov dword[.c], 0x00edcc61
	mov dword[.f], 0x00f9f6f2
	jmp .next
.n512:
	mov dword[.c], 0x00edc850
	mov dword[.f], 0x00f9f6f2
	jmp .next
.n1024:
	mov dword[.c], 0x00edc53f
	mov dword[.f], 0x009f6f2
	jmp .next
.n2048:
	mov dword[.c], 0x00edc22e
	mov dword[.f], 0x00f9f6f2
	jmp .next
	
;________________________________________________
;Set a number in the board
;IN:	ecx number	
;OUT:	al row, ah column
;
setElement:
	call elementPosition
	mov [eax], ecx
	ret

;________________________________________________
;Get a number from the board
;IN: 	al row, ah column
;OUT: 	eax number
;	
getElement:
	push ebx
	call elementPosition
	mov eax, [eax]
	pop ebx
	ret
	
;________________________________________________
;Get the position of an element in the board
;IN: 	al row, ah column
;OUT: 	eax pointer to the element
;	
elementPosition:
	movzx ebx, ah
	and eax, 0xff
	
	shl eax, 4	;Row * 16 (4 * COLUMNS)
	shl ebx, 2	;Column * 4 (4 bytes per element)
	
	add eax, ebx	;Add column to index
	add eax, board	;Actual position in the board
	ret

;________________________________________________
;Remove extra zero from a column
;IN: 	cl column
;OUT: 	nothing
;
removeColumnZeros:
	mov [.c], cl
	mov dword[.n], 0
	mov ecx, ROWS
	
.checkNonZero:
	mov al, 4
	sub al, cl
	mov ah, [.c]
	call getElement
	cmp eax, 0
	je .checkNonZero.zero
	mov ebx, [.n]
	mov [.nz+ebx*4], eax
	inc dword[.n]
	
.checkNonZero.zero:
	loop .checkNonZero

	mov ecx, [.n]	
	cmp ecx, 0
	je .next
	
.removeZeros:
	mov ebx, [.n]
	sub ebx, ecx
	push ecx
	mov al, [.n]
	sub al, cl
	mov ah, [.c]
	mov ecx, [.nz+ebx*4]
	call setElement
	pop ecx
	loop .removeZeros

	mov ecx, ROWS
	sub ecx, [.n]
	cmp ecx, 0
	je .next
	
.fillZerosAtEnd:
	push ecx
	mov al, cl
	add al, [.n]
	dec al
	mov ah, [.c]
	mov ecx, 0
	call setElement
	pop ecx
	loop .fillZerosAtEnd
.next:	
	ret
.n: dd 0
.nz: dd 0, 0, 0, 0 ;Non-zero
.c: db 0

;________________________________________________
;Rotate the board
;IN/OUT: nothing
;
rotateBoard:
	mov dword[.c], 0
	mov ebx, 0
	mov ecx, COLUMNS
	
.columns:
	mov dword[.r], 3
	push ecx	
	mov ecx, ROWS
.rows:
	mov al, [.r]
	mov ah, [.c]
	call getElement
	mov [board2+ebx*4], eax
	inc ebx
	dec dword[.r]
	loop .rows
	inc dword[.c]
	pop ecx
	loop .columns

;Copy board2 to original board
;
	push es
	push ds
	pop es
	
	mov esi, board2
	mov edi, board
	mov ecx, TOTAL_BLOCKS
	rep movsd
	pop es
	
	ret
	
.c:	dd 0
.r:	dd 0

board2:	dd 0, 0, 0, 0
	dd 0, 0, 0, 0
	dd 0, 0, 0, 0
	dd 0, 0, 0, 0
	
;________________________________________________
;Merge two same elements to get a new one in a column
;IN: cl column
;OUT: nothing
;	
mergeColumnElements:
	mov [.c], cl
	mov dword[.r], 0
	call removeColumnZeros

	mov ecx, ROWS-1
.allRows:
	push ecx
	
	mov al, [.r]
	mov ah, [.c]
	call getElement
	mov ebx, eax
	
	mov al, [.r]
	inc al		;Next element
	mov ah, [.c]
	call getElement
	
	cmp eax, ebx
	jne .next

;Elements are same, merge them into one
;
	shl ebx, 1	;Mutiply by 2 (add two same elements)
	add [score], ebx
	cmp ebx, 2048
	je win
.winNext:
	
	mov ecx, ebx
	mov al, [.r]
	mov ah, [.c]
	call setElement
	
	xor ecx, ecx
	mov al, [.r]
	inc al		;Next element
	mov ah, [.c]
	call setElement
	
.next:
	inc dword[.r]
	pop ecx
	loop .allRows
	
	mov cl, [.c]
	call removeColumnZeros
	
	ret
.c: 	dd 0
.r: 	dd 0
	
;________________________________________________
;Get an array of and number of the empty elements
;IN: 	nothing
;OUT: 	eax number of empty elements
;	esi pointer to array of empty elements
;	
getEmptyElements:
	mov ecx, TOTAL_BLOCKS
	mov esi, board
	mov edi, emptyElements
	xor eax, eax
	
.nextElement:
	mov edx, [esi]
	cmp edx, 0
	jne .nextElement.next
		
.freeElementFound:
	mov [edi], esi
	add edi, 4
	inc eax
	
.nextElement.next:
	add esi, 4
	loop .nextElement

	mov esi, emptyElements
	ret

;________________________________________________
;Copy the board to board3
;IN/OUT: nothing
;
copyBoards:
	push es
	push ds
	pop es
	mov esi, board
	mov edi, board3
	mov ecx, TOTAL_BLOCKS
	rep movsd
	pop es
	ret

;________________________________________________
;Add '2' or '4' if changes occured
;IN/OUT: nothing
;
checkBoard:
	push es
	push ds
	pop es
	mov esi, board
	mov edi, board3
	mov ecx, TOTAL_BLOCKS
	rep cmpsd
	pop es
	jne .notEqual
	call checkGameOver
	ret
.notEqual:
	call generate2Random
	ret
	
;The array
;
emptyElements:	dd 0, 0, 0, 0
		dd 0, 0, 0, 0
		dd 0, 0, 0, 0
		dd 0, 0, 0, 0

;Data
;
board:		dd 0, 0, 0, 0
		dd 0, 0, 0, 0
		dd 0, 0, 0, 0
		dd 0, 0, 0, 0

board3:		dd 0, 0, 0, 0
		dd 0, 0, 0, 0
		dd 0, 0, 0, 0
		dd 0, 0, 0, 0

gameOverStr: db 'Game Over!', 0
gameWinMessage: db 'Congratulation! You win! (Press any key to continue playing)', 0
scoreString:	db '2048 Game   Score: ', 0
score:		dd 0
winf:		dd 0

;Constant definitions
;
ROWS	= 4
COLUMNS	= 4

TOTAL_BLOCKS	= ROWS * COLUMNS

TILE_SIZE	= 120

BACKGROUND	= 0x00ffffff
