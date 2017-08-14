;---------------------------------------------------------;
;              	    Master Boot Record			  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
; Searches for active partition and loads its bootsector  ;
;---------------------------------------------------------;

use16					;Tell assembler to generate 16-bit code

start:
;Set up stack segment and pointer
;
	cli				;Disable interrupts
	mov ax, 0xffff
	mov ss, ax
	mov sp, 0
	sti				;Enable interrupt

	mov bx, dx			;Save boot drive

	mov ax, 0
	mov es, ax
	mov ds, ax

;Locate away from 0x7c00 to 0x500
;	
	cli
	cld
	mov si, 0x7c00		;Source
	mov di, 0x500		;Destination
	mov ecx, 512
	rep movsb
	jmp 0x50:begin		;Load new CS and IP

;Begin executing at 0x500
;
begin:

;Load segment registers for new location
;	
	mov ax, 0x50
	mov ds, ax
	sti

;Check active partition and Load boot sector from it
;

checkPartition1:	
	cmp word[partition1.bootFlag], 0x80
	jne checkPartition2
	mov di, partition1	;Save active partition in DI
	jmp loadBootSect

checkPartition2:
	cmp word[partition2.bootFlag], 0x80
	jne checkPartition3
	mov di, partition2
	jmp loadBootSect

checkPartition3:
	cmp word[partition3.bootFlag], 0x80
	jne checkPartition4
	mov di, partition3
	jmp loadBootSect

checkPartition4:
	cmp word[partition4.bootFlag], 0x80
	jne noActivePartition
	mov di, partition4
	jmp loadBootSect

loadBootSect:
	mov eax, dword[di+8]	;Active partition's LBA
	mov dword[DAP.LBA], eax
	mov si, DAP
	mov ah, 0x42		;Extended load sector
	int 0x13		;BIOS disk services
	jnc diskReadOK

;Print disk error message
;
	mov esi, diskErrorMsg
	call print
	jmp $
		
diskReadOK:

;Point DS:SI to first entry
;
	mov ax, 0
	mov ds, ax
	lea si, [di+0x500]	;si = di+0x500

	mov dx, bx	 	;BX contained boot drive
	jmp 0x0000:0x7c00

noActivePartition:
	mov si, noActivePartitionMsg
	call print

	jmp $
DAP:
.size:		db 16
.reserved:	db 0
.sectorsToRead:	dw 1
.segmentOffset:	dd 0x00007c00
.LBA:		dd 000
		dd 0

noActivePartitionMsg: 	db 'Active partition not found!',0
diskErrorMsg:		db 'Disk error!', 0
	
;________________________________
;Funtion to print string
;
;IN: SI string
;OUT: nothing
;
print:
	lodsb
	or al, al
	jz .end
	mov ah, 0eh
	int 0x10
	jmp print

	.end: ret

times 0x1be-($-$$) db 0

partition1:
.bootFlag:		db 0x80		; 0x80 = active (bootable)
.startingHead:		db 0
.startingSector: 	db 2 
.startingCylinder: 	db 0	
.fileSystemID:		db 0x06		; 0x06 = FAT16
.endingHead:		db 255
.endingSector:		db 255
.endingCylinder:	db 255
.LBA:			dd 1		; Starting LBA for partation.
.totalSectors:		dd 73584	; Partation size ~ 37 mib

partition2:
.bootFlag:		db 0x00		; Not active
.startingHead:		db 0
.startingSector: 	db 0
.startingCylinder: 	db 0	
.fileSystemID:		db 0x00
.endingHead:		db 0
.endingSector:		db 0
.endingCylinder:	db 0
.LBA:			dd 0		; Starting LBA for partation.
.totalSectors:		dd 0		; Partation size = 512 mib.

partition3:
.bootFlag:		db 0x00		; Not active
.startingHead:		db 0
.startingSector: 	db 0
.startingCylinder: 	db 0	
.fileSystemID:		db 0x00
.endingHead:		db 0
.endingSector:		db 0
.endingCylinder:	db 0
.LBA:			dd 0		; Starting LBA for partation.
.totalSectors:		dd 0		; Partation size = 512 mib.

partition4:
.bootFlag:		db 0x00		; Not active
.startingHead:		db 0
.startingSector: 	db 0
.startingCylinder: 	db 0	
.fileSystemID:		db 0x00
.endingHead:		db 0
.endingSector:		db 0
.endingCylinder:	db 0
.LBA:			dd 0		; Starting LBA for partation.
.totalSectors:		dd 0		; Partation size = 512 mib.
	
times 510-($-$$) db 0
dw 0xaa55
