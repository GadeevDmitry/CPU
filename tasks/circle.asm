push 0
push 0
push 0
pop rex           
pop rfx         
pop rgx

call fill_ram
hlt

fill_ram:

    push rex
    push 49         
    sub
    push rex
    push 49
    sub
    mul             

    push rfx
    push 49
    sub
    push rfx
    push 49
    sub
    mul             

    add             

    push 49         
    push 49
    mul             

    jbe fill_cell   
    jmp next_cell   

    fill_cell:      
        push 1
        pop [rgx]
    
    next_cell:
        push rgx
        push 1
        add
        pop rgx     

        push rex
        push 1
        add
        pop rex    

        push rex    
        push 100
        je new_line

        call fill_ram
        ret

        new_line:
            push rfx 
            push 1
            add
            pop rfx

            push 0
            pop rex

            push rfx 
            push 100
            je stop_fill_ram

            call fill_ram
            ret

            stop_fill_ram:
                ret
hlt