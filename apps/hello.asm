;---------------------------------------------------------;
;               Alotware Test application	 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;						          ;
;---------------------------------------------------------;

include "alotware.inc"
use32

start:
	
	newLine	
;Pirnt message
;
	mov esi, message
	os printString

;Wait for key
;	
	os waitKeyboard
	
;Terminate the program
;
	os terminate

message: db "Alotware Application Demo.",0
