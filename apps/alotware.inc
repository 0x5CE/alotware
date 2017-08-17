;---------------------------------------------------------;
;                  API Include file	   		  ;
;                                                         ;
;            Written by: Muazzam Ali Kazmi		  ;
;                    Public Domain                        ;
;						          ;
;                                                         ;
;             To be used with applications 	          ;
;---------------------------------------------------------;


;-----------------System calls------------------;
;						;

;-----------------------------------------------;
;           Screen Related functions 		;
;-----------------------------------------------;

printString	=	1			;Print 0 terminated string at cursor position. 
						;IN: esi string.
						;OUT: nothing.						 

printIntDec	=	2			;Print Integer in Decimal. 
						;IN: eax integer
						;OUT: nothing

printIntBin	=	3			;Print Integer in Binary.
						;IN: eax integer
						;OUT: nothing

printIntHex	=	4			;Print Integer in Hexadecimal.
						;IN: eax integer
						;OUT: nothing

clearScreen	=	5			;Clear the screen. 
						;IN/OUT: nothing

clearLine	=	6			;Clear a specified line on screen. 
						;IN: al line. 
						;OUT: nothing

setGraphicsMode	=	7			;Set graphics mode.
						;IN: nothing. 
						;OUT: esi video memory pointer

setTextMode	=	8			;Set 80x25 text mode. 
						;IN/OUT: nothing

scrollDown	=	9		        ;Scroll down screen one line.
						;IN/OUT: nothing

setCursor	=	10			;Set cursor at specific location. 
						;IN: dl X, dh Y. 
						;OUT: nothing.

getCursor	=	36			;Get cursor position.
						;IN: nothing
						;OUT: dl X, dh Y
						 
putChar		=	30			;Print character at cursor position. 
						;IN: al character.
						;OUT: nothing.
		
text.setColor 	=	39			;Set current text background
						;and foreground color
						;IN: eax Foreground (hex RGB),
						;ebx Background (hex RGB).
						;OUT: nothing
						;Note: Text mode is black and white only.
						 
text.getColor	=	40			;Get current text background
						;and foreground color
						;IN: nothing
						;OUT: eax Foreground (hex RGB),
						;ebx Background (hex RGB).
						;Note: Text mode is black and white only.

putCharO 	=	41			;Same as putChar but does not move cursor
						;IN/OUT: same as putChar

getScreenInfo	=	42			;Get screen information.
						;IN: nothing
						;OUT: eax Resolution X (bits 0..15), Y (bits 16..31),
						;ebx Columns (bit 0..7), Rows (8..15), Bits Per Pixel (16..23),
						;edx Frame buffer starting address.
						;CF sets on text mode
						 
useVideoBuffer2 =	43			;Use back buffer (double buffering).
						;IN/OUT: nothing
						 
useVideoBuffer1 = 	44			;Use real video frame buffer.
						;IN/OUT: nothing

screenRefresh	=	45			;Copy back end buffer to video memory.
						;IN/OUT: nothing
			
rand		=	47			;Generate a pseudo random
						;IN: eax maximum
						;OUT: eax number

srand		=	48			;Seed the random number generator
						;IN: eax number
						;OUT: nothing
			 
;-----------------------------------------------;
;          Keyboard Related functions		;
;-----------------------------------------------;

waitKeyboard	=	11			;Wait for keyboard key press. IN: nothing. 
						;OUT: al Char, ah scan code
							  
getString	=	31		 	;Get string from keyboard.
						;IN: al Maximum characters to get
						;OUT: esi String
						 
getKeysStatus	=	46			;Get special keys status.
						;IN: 	nothing
						;OUT:	eax Keys status
						;
						;Format:
						;bit 0: Control key
						;bit 1: Shift key
						;bit 2-31: Reserved
						;
							  
;-----------------------------------------------;
;            String Related Functions	        ;
;-----------------------------------------------;

stringLength	=	12			;Get length of string. 
						;IN: esi String. 
						;OUT: ax String length

stringCompare	=	13			;Compare two strings. 
						;IN: esi First string, 
						;edi second string.
						;OUT: CF sets on =al

stringUpperCase =	14			;Convert string to upper-case. 
						;IN: esi string. 
						;OUT: nothing

stringLowerCase	=	15			;Convert string to lower-case. 
						;IN: esi string. 
						;OUT: nothing

stringTrim	=	16			;Remove extra white spaces from string.
						;IN: esi string.
						;OUT: nothing

stringFindChar	=	17			;Find specific char in string.
						;IN: esi string, al char to find.
						;OUT: eax number of char occurrence
						;CF sets on char not found.
							  

stringToInt	=	18			;Convert string number to integer.
						;IN: esi string. 
						;OUT: eax integer.
						;CF sets on wrong number string.

stringWordsCompare = 	34			;Compare first words of two strings. 
						;IN: esi First string, 
						;edi second string.
						;OUT: CF sets on =al

stringRemoveChar   =	37			;Remove a character from 
						;specific position of string.
						;IN: esi string,
						;eax character position
						;OUT: nothing

stringInsertChar   = 	38			;Insert a character to 
						;Specific position of string.
						;IN: esi string,
						;edx character position,
						;al character to insert
						;OUT: nothing

;-----------------------------------------------;
;            Disk Related functions	        ;
;-----------------------------------------------;

filesList	=	19			;Get list of files.
						;IN: nothing.			
						;OUT: esi Pointer to files list,
						;eax total files

fileLoad	=	20			;Load the file at specific location. 
						;IN: esi File name to load, 
						;edi Buffer to load file onto
						;OUT: CF sets on file not exists
						;or wrong file name

fileExists	=	21			;Check if the file exists. 
						;IN: esi file name to check
						;OUT: eax file size,
						;CF set on file not exists.					


fileSave	=	33		   	;Save a file to disk.
						;IN: esi Pointer to file name,
						;edi Pointer to file data,
						;eax size of file.
						;OUT: CF sets on error or
						;file exists.
						 
fileDelete	=	35			;Remove a file from disk.
						;IN: esi Pointer to file name.
						;OUT: nothing.
						 
;-----------------------------------------------;
;               Kernel functions	        ;
;-----------------------------------------------;

loadProgram	=	22			;Load program from disk and execute it.
						;IN: esi program name,
						;edi Program arguments. 
						;eax=0 if no arguments.
						;OUT: CF sets on error or program not found

terminate	=	23			;Terminate the current running program. 
						;IN/OUT: nothing

memoryAllocate	=	24			;Allocate memory. 
						;IN: number of 512 byte blocks to allocate.
						;OUT: esi pointer to allocated memory

memoryDeallocate =	25			;Deallocate memory. 
						;IN: number of 512 byte blocks to deallocate.
						;OUT: nothing

installISR	 =	26			;Install interrupt service routine. 
						;IN: eax interrupt number,
						;esi pointer to ISR. 
						;OUT: nothing

reboot		=	51			;Reboot the computer.
						;IN/OUT: nothing

;-----------------------------------------------;
;                Mouse functions	        ;
;-----------------------------------------------;

waitMouse	=	27			;Wait For mouse event.
						;IN: nothing.
						;OUT: eax X, ebx Y, edx Buttons.

mouseGet	=	49			;Get current mouse position and buttons
						;IN: nothing.
						;OUT: eax X, ebx Y, edx Buttons.

mouseSet	=	50			;Set new mouse position
						;IN: eax X, ebx Y.
						;OUT: nothing

;-----------------------------------------------;
;              Graphics functions	        ;
;-----------------------------------------------;

graphicsPutPixel =	28		  	;Put a pixel on screen.
						;IN: eax X, ebx Y, edx Hex Color.
						;OUT: nothing

graphicsDrawBlock =	29			;Draw a block of specified color.
						;IN: eax X, ebx Y, esi width,
						;edi Height, edx hex color.
						;OUT: nothing
;-----------------------------------------------;

text:
.DEFAULT_BACKGROUND = 0xffffffff
.DEFAULT_FOREGROUND = 0

fonts:
.width		= 8
.height		= 16

macro os op1
{
	push op1				;System call
	int 100					;API interrupt
}

macro newLine
{
	mov al, 10				;10 is new line character
	os putChar
}
