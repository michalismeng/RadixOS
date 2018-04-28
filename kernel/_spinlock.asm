bits 32
section .text

global acquire_lock
global release_lock

; void acquire_lock(int* lock) : *lock must be 4-byte aligned to ensure atomic read/write
acquire_lock:
    mov eax, dword [esp + 4]        ; copy lock pointer
    .lock_wait:
        test dword [eax], 1         ; check if lock is held by another process (ZF is set when ANDing the two operands equals zero => (ZF == 1 <=> [lock] == 0) )
        jnz .lock_wait              ; the lock is used by someone else (ZF == 0 <=> [lock] == 1)

    .lock_acquire:
        lock bts dword[eax], 0      ; set the first bit of the lock (old value is moved to CF)
        jc .lock_wait               ; if CF is set then the lock was previously held by someone else => more waiting

    ret

; void release_lock(int* lock) : *lock must be 4-byte aligned to ensure atomic read/write
release_lock:
    mov eax, dword [esp + 4]
    mov dword [eax], 0

    ret