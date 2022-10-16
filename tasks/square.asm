push 1
push -9.735
push 23.6925562
pop rcx
pop rbx
pop rax



push rax
push 0
je linear_equation



push rbx
push rbx
mul

push  4
push rax
push rcx
mul
mul

sub
pop rdx



push rdx
push 0
jb no_roots

push rdx
push 0
je one_root



push rbx
push -1
mul

push rdx
sqrt

add

push 2
push rax
mul

div

pop [1]


push rbx
push -1
mul

push rdx
sqrt

sub

push 2
push rax
mul

div

pop [2]



push 2
out
push [1]
out
push [2]
out

hlt




linear_equation:

push rbx
push 0
je zero_equation

push rcx
push -1
mul

push rbx

div

push 1
out
out

hlt



zero_equation:

push rcx
push 0
je infinite_roots

jmp no_roots



no_roots:

push 0
out

hlt



one_root:

push rbx
push -1
mul

push 2
push rax
mul

div

push 1
out
out
hlt

infinite_roots:

push 3
out
hlt