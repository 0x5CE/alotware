;---------------------------------------------------------;
;                   FAT (16) Bootsector 		  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;						          ;
;   Loads and executes flat binary ALOTWARE.OS at 0x500   ;
;---------------------------------------------------------;

use16					;Tell assembler to generate 16-bit code

jmp short start				;Goto start, skip BIOS-Parameter-Block
nop	


;__________________________________________________________
;BIOS Parameter Block (BPB)
;Needed to identify the disk		  
;                                                         
BPB:
OEMName:		db 'ALOTWARE'	;Name of Orignal Equipment Manufacturer
bytesPerSector:	 	dw 512		;Number of bytes in each sector
sectPerCluster:		db 8		;Sectors in one cluster (allocation unit)
reservedSectors:	dw 16		;Sectors reserved after boot sector
totalFATs:		db 2		;Number of FAT tables.
rootDirEntries:		dw 512		;Total files or folders in root directory
smallSectors:		dw 0		;Total small Sectors in disk 
mediaType:		db 0xf8 	;Media type. f8 for removable drives
sectorsPerFAT:		dw 16		;Sectors used in FAT
sectorsPerTrack:	dw 63		;Total sectors in a track
totalHeads:		dw 255		;Number of disk heads
hiddenSectors:		dd 0		
totalSectors:		dd 73584	;Disk size will be ~ 37 mib
driveNumber:		db 0x80		;Drive Number 0x80 for hard-disks 
			db 0
diskSignature:		db 0
volumeID:		dd 0		;Any number
volumeLabel:		db 'ALOTWARE   ';Any 11-char name
fileSystem:		db 'FAT16   '	;Name of File system
;__________________________________________________________

start:
	
BOOT_SEG 	equ 	0x2000		;Segment to relocate bootloader
PROG_SEG 	equ 	0x50		;Segment to load kernel file

;Set up stack segment and pointer
;
	cli				;Disable interrupts
	mov ax, 0x5000
	mov ss, ax
	mov sp, 0
	sti				;Enable interrupt

;Save partition LBA address
;
	mov ebp, dword[si+8]		;Partition LBA address

;Locate away from 0x7c00 to 0x20000
;	
	mov ax, 0
	mov ds, ax
	mov ax, BOOT_SEG
	mov es, ax

	cli
	cld				;Clear direction flag
	mov si, 0x7c00			;Source (ds:si)
	mov di, 0			;Destination (es:di)
	mov ecx, 512			;Total bytes to move
	rep movsb

	jmp BOOT_SEG:begin		;Load new CS and IP

;Begin executing at 0x20000
;
begin:
	
;Load segment registers for new location
;	
	mov ax, BOOT_SEG
	mov ds, ax
	mov es, ax
	sti

	mov byte[driveNumber], dl	;Save drive number

;Calculate root directory size
;
;Formula:
;Size  = (rootDirEntries * 32) / bytesPerSector
;
	mov ax, word[rootDirEntries]
	shl ax, 5			;Multiply by 32
	mov bx, word[bytesPerSector]
	xor dx, dx			;dx = 0
	div bx				;ax = ax / bx
	mov word[rootDirSize], ax	;Save root dir size

;Calculate size of all (both) FAT tables
;
;Formula:
;Size  = totalFATs * sectorsPerFAT
;
	mov ax, word[sectorsPerFAT]
	movzx bx, byte[totalFATs]
	xor dx, dx			;dx = 0
	mul bx				;ax = ax * bx
	mov word[FATsSize], ax		;Save FATs' size

;Calculate all reserved sectors
;
;Formula:
;reservedSecotors + partition LBA
;
	add word[reservedSectors], bp	;bp is partition LBA

;Calculate data area address
;
;Forumula:
;reservedSectors + FATsSize + rootDirSize
;
	movzx eax, word[reservedSectors]	
	add ax, word[FATsSize]
	add ax, word[rootDirSize]
	mov dword[dataArea], eax
	
	
;Calculate root directory LBA address and Load it
;
;Formula:
;LBA  = reservedSectors + FATsSize
;
	movzx esi, word[reservedSectors]
	add si, word[FATsSize]

	mov ax, word[rootDirSize]
	mov di, diskBuffer
		
	call loadSector

;Search the root directory to find the file to load
;
	mov cx, word[rootDirEntries]
	mov bx, diskBuffer

	cld				;Clear direction flag
findLoop:

;Match 11-character file name with 
;file to load's file name.
;
	xchg cx, dx			;Save loop counter
	mov cx, 11
	mov si, fileToLoad
	mov di, bx
	rep cmpsb			;Compare ecx characters at di and si
	je fileFound

	add bx, 32			;Go to next directory entry
	xchg cx, dx			;Restore loop counter
	loop findLoop

;If there is no file then
;print file not found message
;and hang.
;
	mov si, fileNotFound
	call print
	jmp $

fileFound:
	mov si, word[bx+26]		
	mov word[cluster], si		;Save first Cluster


;Load FAT table to find to find all clusters of file
;
	mov ax, word[sectorsPerFAT]	;Total sectors to load
	mov si, word[reservedSectors]	;LBA
	mov di, diskBuffer		;Buffer to load into

	call loadSector

;Calculate cluster size in bytes
;
;Forumula:
;sectPerCluster * bytesPerSector
;
	movzx eax, byte[sectPerCluster]
	movzx ebx, word[bytesPerSector]
	xor edx, edx	
	mul ebx				;ax = ax * bx	
	mov ebp, eax			;Save cluster size
	
	mov ax, PROG_SEG		;Kernel loading segment
	mov es, ax
	mov edi, 0			;Buffer to load file (kernel)

;Find clusters and load cluster chain
;

clustersLoadLoop:

;Converting logical address [cluster] to LBA (physical address)
;
;Forumula:
;((cluster - 2) * sectPerCluster) + dataArea
; 
	movzx esi, word[cluster]		
	sub esi, 2

	movzx ax, byte[sectPerCluster]		
	xor edx, edx			;dx = 0
	mul esi				;(cluster - 2) * sectPerCluster
	
	mov esi, eax	

	add esi, dword[dataArea]

	movzx ax, byte[sectPerCluster]	;Total sectors to load
	
	call loadSector
	
;Find next cluster from FAT table
;
	mov bx, word[cluster]
	shl bx, 1			;bx * 2 (2 bytes in entry)
	add bx, diskBuffer		;FAT location

	mov si, word[bx]		;SI contains next cluster

	mov word[cluster], si		;Save it

	cmp si, 0xfff8			;0xfff8 is End Of File (EOF) marker
	jae finished


;Add free space size for new cluster 
;
	
	add edi, ebp			;ebp is cluster size
	jmp clustersLoadLoop

finished:
	mov ebp, BPB + (BOOT_SEG * 16)	;Point ebp to BIOS Parameter Block
	mov dl, byte[driveNumber]
	jmp PROG_SEG:0x0000		;Load CS:IP and start executing

;____________________________________
;Variables
;
	
cluster:	dw 0

fileNotFound:	db 'ALOTWARE.OS not found!', 0
error:		db 'Disk error!', 0	;Disk error message
		
fileToLoad:	db 'ALOTWAREOS '	;File name to load from disk

rootDirSize:	dw 0			;Root directory size (in sectors)
FATsSize:	dw 0			;FAT tables size (in sectors)
dataArea:	dd 0			;dataArea start address (LBA)

;____________________________________
;Funtion to print string
;
;IN: SI string
;OUT: nothing
;
print:
	lodsb		;mov al, [si] & inc si
	or al, al	;cmp al, 0
	jz .end
	mov ah, 0eh
	int 0x10	;Put si to screen
	jmp print
.end: 
	ret

;____________________________________
;Load sector from current boot drive
;
;IN:	ax Total sectors to load,
;	esi LBA address,
;	es:di Location to load sectors.
;OUT: 	nothing
;
loadSector:
	push si

	mov word[DAP.totalSectors], ax
	mov dword[DAP.LBA], esi
	mov word[DAP.segment], es
	mov word[DAP.offset], di

	mov dl, byte[driveNumber]
	mov si, DAP
	mov ah, 0x42			;Extended sector read funtion
	int 0x13			;BIOS disk services
	jnc .finish			

;If there are error in disk then
;print error message and hang.
;
	mov si, error			
	call print
	jmp $

.finish:	
	pop si
	ret
DAP:
.size:		db 16
.reserved:	db 0
.totalSectors:	dw 0
.offset:	dw 0x0000
.segment:	dw 0
.LBA:		dd 0
		dd 0
;___________________________________

TIMES 510 - ($-$$)	db 0	 	;Make bootloader exectly 512 bytes
bootSignature: 		dw 0xAA55	;Bootable flag
diskBuffer:
