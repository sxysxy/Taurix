; src/arch/i386/i386_ipc1.S
;   author: hfcloud(sxysxygm@gmail.com)
;     date: 2019.08.27

global ipc_send, ipc_recv, ipc_notify, ipc_post, ipc_peek

ipc_send:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    
    mov eax, 0              ;eax<-ipc_send
    mov ebx, [ebp+8]        ;ebx<-message
    mov ecx, [ebp+12]       ;ecx<-receiver_pid

    int 0x9f

    pop ecx
    pop ebx
    leave
    ret

ipc_recv:
    push ebp
    mov ebp, esp
    push ebx

    mov eax, 1
    mov ebx, [ebp+8]

    int 0x9f

    pop ebx
    leave
    ret

ipc_notify:
    push ebp
    mov ebp, esp
    push ebx

    mov eax, 2
    mov ebx, [ebp+8]

    int 0x9f

    pop ebx
    leave
    ret


ipc_post:

    ret

ipc_peek:

    ret