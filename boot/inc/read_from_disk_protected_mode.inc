; LBA 28 bit ATA PIO Implementation
; Reading from disk, polling methodA
;    eax: 28 bit lba to starting address
;     cl: number of sectors to load
;    edi: memory location to which data will be loaded
AtaPioLbaRead:
  pushad

  xor bx, bx
  mov bl, cl
  mov bp, bx        ; bp keeps track of the number of sectors to read
  mov ebx, eax      ; ebx contains the current lba to sector


  ; Before writing to command registers, make sure that controller is not busy and the driver is ready
  .is_controller_busy:
    mov dx, 0x1f7
    in  al, dx
    test al, 0x80     ; test for BUSY BUS bit to be cleared
    jne .is_controller_busy

  .is_driver_ready:
    mov dx, 0x1f7
    in al, dx
    test al, 0x40     ; test for DRIVE READY bit to be set
    je .is_driver_ready


  ; Initialization
  mov edx, 0x1f6    ; ATA drive port
  shr eax, 24       ; send 4 highest bit of lba
  or  al,  0xe0     ; 0xe0 | highest 4 bit address -> use ATA_PIO_MASTER
  out dx, al

  mov dx, 0x1f2 ; Port for number of sector
  mov al, cl
  out dx, al

  mov dx, 0x1f3  ; Send bit 0-7 of LBA
  mov eax, ebx
  out dx, al

  mov dx, 0x1f4  ; Send bit 8-15 of LBA
  mov eax, ebx
  shr eax, 8
  out dx, al

  mov dx, 0x1f5  ; Send bit 16-23 of LBA
  mov eax, ebx
  shr eax, 16
  out dx, al

; Before we send a command to the drive, make sure that it is not busy
.is_controller_busy2:
  mov dx, 0x1f7
  in al, dx
  test al, 0x80
  jne .is_controller_busy2     ; Note: jne means if ZF is not zero (not set)
                    ; Since test performs AND, if AND al, 0x80 is not zero,
                    ; ZF is set to 1 (i.e, controller is busy)

  mov edx, 0x1f7  ; Send READ SECTOR command to the command port
  mov al, 0x20
  out dx, al

.is_drq_ready:
  mov dx, 0x1f7
  in al, dx
  test al, 0x8        ; wait for DRQ to be set.
  je .is_drq_ready

  cld             ; so that REP INSW increases EDI

  mov ecx, 256      ; force read only 1 sector first
  mov edx, 0x1f0   ; Read from data port
  rep insw

  mov dx, 0x1f1
  in al, dx


  .400ns_delay:
  mov dx, 0x1f7
  in al, dx       
  in al, dx      
  in al, dx       
  in al, dx

.is_controller_busy3:
    mov dx, 0x1f7
  in al, dx
  test al, 0x80
  jne .is_controller_busy3

  dec bp
  cmp bp, 0
  jne .is_drq_ready

AtaPioLbaRead_Done:
  popad
  ret