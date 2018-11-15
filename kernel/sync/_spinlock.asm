bits 32
section .text

global acquire_spinlock
global release_spinlock

; void acquire_spinlock(int* lock) : *lock must be 4-byte aligned to ensure atomic read/write
acquire_spinlock:
    mov eax, dword [esp + 4]        ; copy lock pointer
    .lock_wait:
        test dword [eax], 1         ; check if lock is held by another process (ZF is set when ANDing the two operands equals zero => (ZF == 1 <=> [lock] == 0) )
        jnz .lock_wait              ; the lock is used by someone else (ZF == 0 <=> [lock] == 1)

    .lock_acquire:
        lock bts dword[eax], 0      ; try to set the first bit of the lock (old value is moved to CF)
        jc .lock_wait               ; if CF is set then someone else took the lock => more waiting

    ret

; void release_spinlock(int* lock) : *lock must be 4-byte aligned to ensure atomic read/write
release_spinlock:
    mov eax, dword [esp + 4]
    btr dword [eax], 0

    ret