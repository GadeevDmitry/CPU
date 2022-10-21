push 0
push 0
push 0
pop rex             #координата по горизонтали
pop rfx             #координата по вертикали
pop rgx             #номер текущего элемента

call fill_ram

hlt

fill_ram:

    push rex
    push 49         #(49, 49) - координаты центра круга
    sub
    push rex
    push 49
    sub
    mul             #квадрат расстояния между элементом и центром по горизонтали

    push rfx
    push 49
    sub
    push rfx
    push 49
    sub
    mul             #квадрат расстояния по вертикали

    add             #квадрат расстояния

    push 49         #радиус R = 49
    push 49
    mul             #квадрат радиуса

    jbe fill_cell   #если ячейка внутри круга, заполняем её
    jmp next_cell   #иначе сразу пересчитываем параметры для следующей ячейки

    fill_cell:      #заполняем ячейку ненулевым значением
        push 1
        pop [rgx]
    
    next_cell:
        push rgx
        push 1
        add
        pop rgx     #увеличиваем номер текущего элемента

        push rex
        push 1
        add
        pop rex     #увеличиваем координату по горизонтали

        push rex    #проверяем, нужно ли переходить на новую строку
        push 100
        je new_line

        call fill_ram
        ret

        new_line:
            push rfx #увеличиваем координату по вертикали и обнулем по горизонтали
            push 1
            add
            pop rfx

            push 0
            pop rex

            push rfx #если мы обошли всю оперативку, останавливаем рекурсию
            push 100
            je stop_fill_ram

            call fill_ram
            ret

            stop_fill_ram:
                ret