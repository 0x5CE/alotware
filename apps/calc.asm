;---------------------------------------------------------;
;            	    Simple Calculator		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;---------------------------------------------------------;

include "alotware.inc"
use32

start:

;Print start message
;
	mov esi, startMessage
	os printString
	
calculate:

;Get first number
;
	mov esi, firstNumberMsg
	os printString
	
	
	call getNumber
	mov dword[n1st], eax		;Save first number

;Get second number
;	
	mov esi, secondNumberMsg
	os printString
	
	
	call getNumber
	mov dword[n2nd], eax		;Save second number
	
;Ask for operation to perform.
;
	mov esi, operationMsg
	os printString
	
	os waitKeyboard
	
	
	cmp al, '0'
	je addNumbers
	cmp al, '1'
	je subtract
	cmp al, '2'
	je multiply
	cmp al, '3'
	je divide

addNumbers:
	mov eax, dword[n1st]
	mov ebx, dword[n2nd]
	add eax, ebx		;eax = eax + ebx
	mov dword[ans], eax
	
	jmp printAns
	
subtract:
	mov eax, dword[n1st]
	mov ebx, dword[n2nd]
	sub eax, ebx		;eax = eax - ebx
	mov dword[ans], eax
	
	jmp printAns
multiply:
	mov eax, dword[n1st]
	mov ebx, dword[n2nd]
	mul ebx			;eax = eax * ebx
	mov dword[ans], eax
	
	jmp printAns
divide:
	cmp ebx, 0
	je .divideByZero
		
	mov eax, dword[n1st]
	mov ebx, dword[n2nd]
	mov edx, 0
	div ebx			;eax = eax / ebx
	mov dword[ans], eax

	jmp printAns
	
.divideByZero:
	mov esi, divideByZeroMsg
	os printString
	jmp printAns.next
	
printAns:

	newLine
	
	mov esi, answerMsg
	os printString
	
	mov eax, dword[ans]
	os printIntDec
	
.next:
	newLine
	newLine
	
	jmp calculate
	
;________________________________________________
;Get a number from keyboard
;IN: 	nothing
;OUT: 	eax Number
;
getNumber:

	mov al, 10			;Maximum 10 characters to get
	os getString
	os stringTrim
	os stringToInt
	
	push eax
	
	newLine
	
	pop eax
	ret
	

;----------Data-----------;

startMessage:		db 10, '                          '
			db 'Calculator application', 10,10,0

divideByZeroMsg: 	db 'Divide by zero not Allowed!', 0

firstNumberMsg:		db 'Enter 1st Number         >> ', 0
secondNumberMsg:	db 'Enter 2nd Number         >> ', 0
operationMsg:		db '0.Add 1.Sub 2.Mul 3.Div  >> ', 0
answerMsg:		db 'Answer is                 = ', 0

n1st: 			dd 0
n2nd:			dd 0
ans:			dd 0
