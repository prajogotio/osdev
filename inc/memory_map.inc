bits 16

; MemoryMapEntry:
;    base address  8 byte
;    length        8 byte
;    type          4 byte   1 = avalable
;    acpi_null     4 byte
; total: 24 bytes

GetMemoryMap:
  pushad

  mov [pointer_to_table], di

  ; Provide 4 bytes for number of entry info
  add   di, 4

  xor   bp, bp        ; number of entries


  xor   ebx, ebx
  mov   edx, "PAMS"
  mov   eax, 0xe820
  mov   ecx, 24
  int   0x15

  jc    GetMemoryMap_Error
  cmp   eax, "PAMS"
  jne   GetMemoryMap_Error

  jmp   GetMemoryMap_Validate

GetMemoryMap_NextEntry:
  mov   edx, "PAMS"
  mov   ecx, 24
  mov   eax, 0xe820
  int   0x15

GetMemoryMap_Validate:
  jcxz   GetMemoryMap_SkipEntry       ; ECX stores the size read. if 0, then skip.

  ; Otherwise, check if the entry is a good entry
  ; The entry could potentially contain 0 length, so we can skip it
  mov ecx, dword [es:di + 8]
  or  ecx, dword [es:di + 12]

  jz  GetMemoryMap_SkipEntry

  ; We got a good entry! Update bp and store to list
  inc bp
  add di, 24


GetMemoryMap_SkipEntry:
  cmp ebx, 0
  jne GetMemoryMap_NextEntry
  ; Otherwise we are done

GetMemoryMap_Error:
  ; Give up lol

GetMemoryMap_Done:
  mov ebx, [pointer_to_table]
  mov word [ebx], bp
  popad
  ret



pointer_to_table:
  dd 0
