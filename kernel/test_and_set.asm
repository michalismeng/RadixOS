bits 32
section .text

global test_and_set

; int test_and_set(int new_val, int* lock)
test_and_set:
    mov eax, dword [esp + 4]        ; copy new_val to eax
    mov ebx, dword [esp + 8]        ; copy lock pointer
    xchg eax, [ebx]                 ; inter-processor atomic exchange (xchg is locked automatically)

    ret                             ; after the exchange, eax holds the old value of the lock, so return it 