push 1
push 6
pop rbx
pop rax

call fact

push rax
out

hlt


fact:
    push rbx
    push 1
    ja continue

    push 2
    push 1
    pop rax
    pop rbx

    ret

    continue:
        push rbx
        push 1
        sub
        pop rbx

        call fact
        
        push rax
        push rbx
        mul
        pop rax

        push rbx
        push 1
        add
        pop rbx

        ret
