bits 32
; Set up temporary paging
%define   PAGE_DIR             0x9C000
%define   PAGE_TABLE_0         0x9D000
%define   PAGE_TABLE_768       0x9E000
%define   PAGE_TABLE_ENTRIES   1024

%define   PRIV                  3

EnablePaging:
  pusha

; Identity mapping
  mov eax, PAGE_TABLE_0
  mov ebx, 0x0 | PRIV
  mov ecx, PAGE_TABLE_ENTRIES
.loop:
  mov dword [eax], ebx        ; write page entry to PAGE_TABLE_0
  add eax, 4                  ; go to next page entry (4 bytes per entry)
  add ebx, 4096               ; next frame address (each frame is 4096)
  loop .loop

; Map kernel to 3GB virtual address. i.e. map physical 0x00100000
; to virtual 0xc0000000
  mov eax, PAGE_TABLE_768
  mov ebx, 0x00100000 | PRIV
  mov ecx, PAGE_TABLE_ENTRIES
.loop2:
  mov dword [eax], ebx
  add eax, 4
  add ebx, 4096
  loop .loop2  

  ; Add default page directory entry to directory table
  mov eax, PAGE_TABLE_0 | PRIV
  mov dword [PAGE_DIR], eax

  mov eax, PAGE_TABLE_768 | PRIV
  mov dword [PAGE_DIR + (768*4)], eax


  ; Install directory table
  mov eax, PAGE_DIR
  mov cr3, eax

  ; Enable paging
  mov eax, cr0
  or eax, 0x80000000
  mov cr0, eax

  popa
  ret
