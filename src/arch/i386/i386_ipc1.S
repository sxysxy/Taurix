; src/arch/i386/i386_ipc1.S
;   author: hfcloud(sxysxygm@gmail.com)
;     date: 2019.08.27

global ipc_send, ipc_recv, ipc_notify, ipc_post, ipc_peek

ipc_send:
    mov eax, 0              ;eax<-ipc_send
    mov ebx, [esp+4]        ;ebx<-message
    mov ecx, [esp+8]        ;ecx<-receiver_pid

    int 0x9f
    ret

ipc_recv:
    mov eax, 1
    mov ebx, [esp+4]

    int 0x9f
    ret

ipc_notify:
    mov eax, 2
    mov ebx, [esp+4]

    int 0x9f
    ret


ipc_post:

    ret

ipc_peek:

    ret