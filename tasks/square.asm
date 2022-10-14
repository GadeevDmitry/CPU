push 15.452
push -3.9038
push -19.9
pop rcx
pop rbx
pop rax



push rbx
push -1
mul

push 2
push rax
mul

div
pop [3]



push rbx
push rbx
mul

push 4
push rax
push rcx
mul
mul

sub
sqrt

push 2
push rax
mul

div
pop [4]



push [3]
push [4]
sub
out
pop void


push [3]
push [4]
add
out
pop void

hlt