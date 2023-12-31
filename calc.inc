%include "lib.inc"
MAX_SIZE equ 256

section .data
data_buffer db MAX_SIZE dup (0)
;r15 = strlen + null pointer
%macro IntToStr 2

    push   rax             ; Preserve the value of the integer register
    push   rcx             ; Preserve the value of the loop counter register
    push   rdx             ; Preserve the value of the remainder register
    push   rdi

    mov    r15 , 1 ;for null byte 
    mov    rdi , data_buffer         ; Store the pointer to the destination string
    add    rdi , MAX_SIZE   ; Move the pointer to the end of the reserved bytes
    dec    rdi             ; move to one byte from the end
    mov    byte [rdi], 0   ; add null byte
    mov    rcx , 10         ; Set divisor for dividing by 10
    mov    rax , %1         ; Load the integer value to be converted

    cmp    rax, 0          ; Check if the number is zero
    jnz    %%start_loop     ; If it's not zero, continue with the loop

    ; If it's zero, directly store '0' and null-terminate the string
    inc    r15
    mov    rdi, %2 
    mov    byte [rdi], '0'
    mov    byte [rdi + 1] , 0
    jmp    %%exit

%%start_loop:
    xor    rdx , rdx        ; Clear the remainder register -->  avoid remainder and result concatenation
    div    rcx             ; Divide the number by 10 (quotient in RAX, remainder in RDX)
    add    dl , '0'         ; Convert the remainder (single digit) to ASCII
    dec    rdi             ; Move the pointer one position back
    inc    r15
    mov    [rdi] , dl       ; Store the digit in the string
    test   rax , rax        ; Check if quotient is zero
    jnz    %%start_loop     ; If not, continue the loop

%%end:

    mov    rdx , %2
    xor    rbx , rbx ; counter

%%storeLoop:
    mov al , byte [rdi]
    mov byte [rdx] , al
    inc rdi
    inc rdx
    inc rbx 
    cmp rbx , r15
    jne %%storeLoop

    mov rdi , data_buffer
    add rdi , MAX_SIZE
    sub rdi , r15
    xor rbx , rbx ; counter

%%clearLoop:
    mov byte [rdi] , 0
    inc rbx 
    inc rdi
    cmp rbx , r15 
    jne %%clearLoop

%%exit:

    pop    rdi
    pop    rdx             ; Restore the value of the remainder register
    pop    rcx             ; Restore the value of the loop counter register
    pop    rax             ; Restore the value of the integer register


%endmacro


section .text 

_prompt:    
    mov rax , 1
    mov rdi , 1
    mov rdx , 2
    mov rsi , closing_angle_braquet
    syscall
    ret

_read_user_input:
    mov rax, 0 
    mov rdi, 0 
    mov rsi, user_input 
    mov rdx, 256
    syscall
    ret

_check_exit:
    xor rcx , rcx
    mov rsi , user_input ; has null terminator
    mov rdi , exit_cmd
    .check_if_exit_loop:
    mov al , byte [rsi]
    mov bl , byte [rdi]
    inc rsi 
    inc rdi 
    inc rcx 
    cmp rcx , 5; needs 5 iteration to get to the string end 
    je .exit_check 
    cmp al , bl
    je .check_if_exit_loop

    xor rcx , rcx
    mov rsi , user_input ; has null terminator
    mov rdi , exit_cmd2
    .check_if_exit_loop2:
    mov al , byte [rsi]
    mov bl , byte [rdi]
    inc rsi 
    inc rdi 
    inc rcx 
    cmp rcx , 5
    je .exit_check 
    cmp al , bl
    je .check_if_exit_loop2

    ret 

.exit_check:
    cmp al , 0xa ; check if the last char is the return char
    jne .return
    exit
    .return:
    ret 


_check_invalide_char:
    mov rsi , user_input ; has null terminator
    .check_if_invalide_loop:
    mov al , byte [rsi]
    inc rsi 
    cmp al, 10 ; check if end of char
    je .no_invalide_char
    cmp al , 32
    je .check_if_invalide_loop
    cmp al , 42
    je .check_if_invalide_loop 
    cmp al , 43
    je .check_if_invalide_loop
    cmp al , 45 
    je .check_if_invalide_loop
    cmp al , '^'
    je .check_if_invalide_loop
    cmp al , 47
    jl .invalide_char
    cmp al , 57
    jg .invalide_char
    
    jmp .check_if_invalide_loop ; if value between 47 and 57

    .invalide_char:
    mov rax , 1
    mov rdi , 1
    mov rdx , 30
    mov rsi , error_message_ic
    syscall
    mov r8 , 1
    ret 

    .no_invalide_char:
    xor r8 , r8
    ret

_check_invalide_division:    
    xor rcx , rcx ; 1 = found division
    mov rsi , user_input ; has null terminator
    .check_if_invalide_loop_d:
    mov al , byte [rsi]
    inc rsi 
    cmp al, 10 ; check if end of char
    je .no_invalide_division
    cmp al , 47
    je .division
    cmp al , 32 
    je .check_if_invalide_loop_d
    cmp al , 48
    je .zero

    xor rcx , rcx
    jmp .check_if_invalide_loop_d


    .zero:
    cmp rcx , 1
    je .invalide_math_cdz
    jmp .check_if_invalide_loop_d


    .division:
    mov rcx , 1
    jmp .check_if_invalide_loop_d

    .invalide_math_cdz:
    mov rax , 1
    mov rdi , 1
    mov rdx , 32
    mov rsi , error_message_cdz
    syscall
    mov r8 , 1
    ret 

    .no_invalide_division:
    xor r8 , r8
    ret


opperations_simplification:
    mov rsi , user_input
    FindChar user_input , '-'
    cmp rax , 1
    je .next

    FindChar user_input , '+'
    cmp rax , 1
    je .next

    jmp .return_no_print
    .next:

    ClearData buffer , MAX_SIZE
    mov rdi , buffer
    xor r9 , r9;0 = no opperation /  1 = first operation / 2 = second operation
    xor r10 , r10; 0 = found addition / 1 = found substraction --> for first operation
    xor r11 , r11; 0 = found addition / 1 = found substraction --> for second operation
    mov r12 , 2; space counter for copying '-' and '+' opperation
.loop:
    mov al , byte [rsi]
    inc rsi
    cmp al , 0
    je .return
    cmp al , 32
    je .space
    cmp al , '-'
    je .substraction
    cmp al , '+'
    je .addition
    jmp .other_char

    .addition:
    cmp r9 , 1
    je .second_operation_add
    mov r10 , 0
    inc r9
    jmp .loop
    .second_operation_add:
    mov r11 , 0
    inc r9
    jmp .loop

    .space:
    mov byte [rdi] , al 
    inc rdi 
    cmp r9 , 1
    jne .no_increment
    inc r12
    .no_increment:
    jmp .loop

    .substraction:
    cmp r9 , 1
    je .second_operation_sub
    mov r10 , 1
    inc r9
    jmp .loop
    .second_operation_sub:
    mov r11 , 1
    inc r9
    jmp .loop

    .other_char:
    ;copy char into buffer
    cmp r9 ,2
    jg .invalide_math_imo
    cmp r9 ,2 
    jne .no_simplify
    call _simplifying_opperation
    .no_simplify:
    cmp r9 , 1
    jne .no_sub_add
    mov rcx , rsi 
    sub rcx , r12
    mov r13b , byte [rcx]
    mov byte [rdi] , r13b
    inc rdi 
    mov r12 , 2
    .no_sub_add:
    ;copy char
    mov byte [rdi] , al 
    inc rdi 

    xor r9 , r9
    jmp .loop


.invalide_math_imo:
    mov rax , 1
    mov rdi , 1
    mov rdx , 36
    mov rsi , error_message_imo
    syscall
    mov r8 , 1
    jmp .return_no_print

.return:
    CopyChar user_input , buffer , MAX_SIZE

    ;mov rax , 1
    ;mov rdi , 1
    ;mov rsi , user_input
    ;mov rdx , 256
    ;syscall
.return_no_print:
ret

_simplifying_opperation:
    cmp r10 , r11
    jne .not_same_op
    jmp .simp_add
    
    .not_same_op:
    jmp .simp_sub

    .simp_add:
    mov byte [rdi] , '+' 
    inc rdi 
    jmp .end

    .simp_sub:
    mov byte [rdi] , '-' 
    inc rdi 

    .end:
    xor r9 , r9
    xor r10 , r10
    xor r11 , r11
    ; al = rsi - 1
ret 

_check_invalide_math:    
    xor rcx , rcx ; 1 = found operation
    mov rsi , user_input ; has null terminator
    .check_if_invalide_loop_m:
    mov al , byte [rsi]
    inc rsi 
    cmp al, 10 ; check if end of char
    je .no_invalide_math
    cmp al , 32 
    je .check_if_invalide_loop_m
    cmp al , 42
    je .math_operation
    cmp al , 43
    je .math_operation
    cmp al , 45
    je .math_operation
    cmp al , 47
    je .math_operation


    xor rcx , rcx
    jmp .check_if_invalide_loop_m


    .math_operation:
    cmp rcx , 1
    jl .no_next_check
    cmp al , '-'
    je .has_add_sub
    cmp al , '+'
    jne .invalide_math_imo

    .has_add_sub:
    mov rbx , rsi 
    sub rbx , 2; to get to the char before '+' or '-'
    .checking_mul_div_loop:
    mov dl , byte[rbx]
    dec rbx 
    cmp dl , 32
    je .checking_mul_div_loop
    cmp dl , '/'
    je .no_next_check
    cmp dl , '*'
    je .no_next_check
    jmp .invalide_math_imo; if no '*' , '/' or ' ' found

    .no_next_check:
    mov rcx , 1
    jmp .check_if_invalide_loop_m

    .invalide_math_imo:
    mov rax , 1
    mov rdi , 1
    mov rdx , 36
    mov rsi , error_message_imo
    syscall
    mov r8 , 1
    jmp .return 

    .no_invalide_math:
    xor r8 , r8
    jmp .return


.return:
    ret


_remove_space:
    mov rsi , user_input 
    mov rdi , user_input_no_space
    dec rdi
_remove_space_loop:
    mov al , byte [rsi]
    inc rsi 
    cmp al, 0; to keep new line character 
    je .return
    cmp al , 32
    jne .no_space
    jmp _remove_space_loop

    .no_space:
    inc rdi 
    mov byte [rdi], al
    jmp _remove_space_loop

.return:
    ret

square_handling:
    mov rsi , user_input_no_space
    FindChar user_input_no_space , '^'
    cmp rax , 1
    jne .return_no_print
    
    mov rdi , buffer
    mov rbx , first_number 
    mov rcx , second_number
    xor rdx , rdx ;last operation found pointer
    xor r10 , r10; 0 = first number / 1 = second number 
    xor r9 , r9; 0 = sqr operation / 1 = other operation
    mov r11 , user_input_no_space ;last copied char
    xor r13 , r13; 0 = no negative int / 1 = negative int
    xor r14 , r14 
    ClearData buffer , MAX_SIZE
    ClearData first_number , MAX_SIZE
    ClearData second_number , MAX_SIZE

.loop:
    mov al , byte [rsi]
    inc rsi
    cmp al , 10
    je .return
    cmp al , '^'
    je .sqr
    cmp al , 48
    jge .number
    jmp .other_opperation


    .sqr:
    cmp byte [first_number] , 0
    je .error_while_processing

    mov r9 , 0
    mov r10 , 1
    mov rdx , rsi - 1
    jmp .loop

    .other_opperation:
    cmp al , '-'
    je .minus_found
    .no_negative_number:
    mov r9 , 1
    mov r10 , 1
    mov rdx , rsi - 1
    jmp .loop

    .minus_found:
    mov r15b , '/' 
    cmp byte [rsi - 2] , r15b 
    je .negative_div_mul
    mov r15b , '*'
    cmp byte [rsi - 2] , r15b 
    je .negative_div_mul

    cmp byte [first_number] , 0
    jne .no_negative_number
    mov r13 , 1
    jmp .loop
    .negative_div_mul:
    mov r14 , 1
    jmp .loop

    .number:
    cmp r10 , 1
    je .second_number
    ;.first_number:
    mov r15b , [rsi - 1]
    mov byte [rbx] , r15b
    inc rbx

    cmp byte [rsi] , 10
    je .copy_end

    jmp .loop
    .second_number:
    mov r15b , [rsi - 1]
    mov byte [rcx] , r15b
    inc rcx 

    cmp byte [rsi] , '^'
    je .processing_math
    cmp byte [rsi] , 48
    jl .processing_math ; not a number ! --> processing math if the opperation is completed

    jmp .loop

    .processing_math:
    xor r10 , r10;to start again at first number
    cmp r9 , 1
    mov rbx , first_number 
    je .other_opperation_copy
    mov rcx , second_number 
    StrToInt rbx 
    StrToInt rcx
    call _process_sqr
    cmp byte [rsi] , 0
    je .return
    jmp .end

    .other_opperation_copy:
    mov rsi , rdx; move back pointer for next opperation

    ;for negative number
    cmp r13 , 1
    jne .copy_loop
    mov byte [rdi] , '-'
    inc rdi 

    .copy_loop:
    mov al , byte [rbx]
    mov byte [rdi] , al
    inc rbx 
    inc rdi 
    cmp byte [rbx] , 0
    jne .copy_loop
    ;copy opperator
    dec rdx 
    mov al , byte [rdx]
    mov byte [rdi] , al
    inc rdi
    ;for negative division and multiplication:
    cmp r14 ,1 
    jne .end
    mov al , '-'
    mov byte [rdi] , al
    inc rdi
    inc rsi 

    .end:
    xor r13 , r13
    xor r14 , r14

    ClearData first_number , MAX_SIZE
    ClearData second_number , MAX_SIZE
    mov rbx , first_number 
    mov rcx , second_number
    jmp .loop

.copy_end:
    mov rbx , first_number 

    .copy_last_int_Loop:
    mov al , byte [rbx]
    mov byte [rdi] , al
    inc rbx 
    inc rdi 
    cmp byte [rbx] , 0
    jne .copy_last_int_Loop
    
    mov byte [rdi] , 10
    inc rdi
    
    jmp .return


.error_while_processing:
    mov rax , 1
    mov rdi , 1
    mov rdx , 34
    mov rsi , error_message_pf
    syscall
    mov r8 , 1
    jmp .return_no_print

.return:
    cmp byte [first_number] , 0
    je .no_error
    cmp r10 , 1
    jne .no_error
    jmp .error_while_processing

    .no_error:

    StrLen user_input_no_space
    ClearData user_input_no_space , rax
    StrLen buffer;output = rax
    CopyChar user_input_no_space , buffer , rax
    
    FindChar user_input_no_space , '^'
    cmp rax , 1
    je square_handling

    mov rax , 1
    mov rdi , 1
    mov rsi , user_input_no_space
    mov rdx , 256
    syscall
.return_no_print:
ret
_process_sqr:

cmp rcx , 1
je .after_process
cmp rcx , 0
je .square_zero

mov r11 , 1
mov r12 , rbx 
.calc_loop:
imul rbx , r12
inc r11 
cmp r11 , rcx
jne .calc_loop
jmp .after_process

.square_zero:
mov rbx , 0

.after_process:
IntToStr rbx , first_number
mov rbx , first_number
;for first negative number
mov al , byte [user_input_no_space] 
cmp al , '-'
jne .store_loop
cmp r13 , 1
jne .store_loop
mov byte [rdi] , '-'
inc rdi
xor r13 , r13

.store_loop:
mov al , byte [rbx]
mov byte [rdi] , al
inc rbx
inc rdi
cmp byte [rbx] , 0
jne .store_loop

;move opperation:
mov al , byte [rsi]
mov byte [rdi] , al
inc rsi
inc rdi
ret 

resolve_mul_div:
    FindChar rsi , '*'
    cmp rax , 1
    je .next
    
    FindChar rsi , '/'
    cmp rax , 1
    je .next
    ret

    .next:
    xor r8 , r8 ; 0 = no error / 1 = error
    xor r9 , r9 ; 1 = div / 2 = mul 
    xor r10 , r10 ; 0 = writing first number / 1 = writing second number / ending first number
    xor r12 , r12 ; negative number handling
    mov rbx , first_number 
    mov rcx , second_number 
    ClearData first_number , MAX_SIZE
    ClearData second_number , MAX_SIZE
    ClearData buffer , MAX_SIZE
    _the_loop:
    cmp r10 , 2
    je .processing_math
    mov al , byte [rsi]
    inc rsi 
    cmp al , 10 ; check if end of string
    je .return
    cmp al , '+'
    je .opperation
    cmp al , '-'
    je .opperation
    cmp al , 42
    je .multiplication
    cmp al , 47
    je .division
    cmp al , 48
    jl _the_loop
    cmp al , 57
    jle .number
    
    jmp _the_loop

    .opperation:
    cmp byte [rsi - 2] , '/'
    je .make_operation 
    cmp byte [rsi - 2] , '*'
    je .make_operation

    mov rdi , rsi 
    .find_mul_div:
    mov r15b , byte [rdi]
    inc rdi 
    cmp r15b , '*'
    je .found_mul_div
    cmp r15b , '/'
    je .found_mul_div
    cmp r15b , 48
    jl .no_make_operation
    jmp .find_mul_div

    .make_operation:
    cmp al , '+'
    je .go_to_loop
    inc r12
    .go_to_loop:
    jmp _the_loop

    .found_mul_div:
    xor r10 , r10
    mov rbx , first_number 
    ClearData first_number , MAX_SIZE    
    jmp .make_operation
    .no_make_operation:
    jmp resolve_mul_div

    .multiplication:    
    mov r9 , 1
    inc r10
    jmp _the_loop

    .division:
    mov r9 , 0
    inc r10
    jmp _the_loop


    .number:
    cmp r10 , 1
    je .second_number
    ;.first_number:
    mov r15b , [rsi - 1]
    mov byte [rbx] , r15b
    inc rbx
    jmp _the_loop
    .second_number:
    mov r15b , [rsi - 1]
    mov byte [rcx] , r15b
    inc rcx 

    cmp byte [rsi] , 48
    jl .processing_math ; not a number !

    jmp _the_loop


    .processing_math:
    mov rbx , first_number 
    mov rcx , second_number 

    cmp byte [rbx] , 0
    je .error_while_processing
    cmp byte [rcx] , 0
    je .error_while_processing

    .no_error:
    StrToInt rbx 
    StrToInt rcx
    cmp r9 , 0
    je .prc_div
    call _perform_multiplication
    jmp .after_process

    .prc_div:
    call _perform_division
    cmp r8 , 1
    je .return_no_print

    .after_process:
    mov rdi , user_input_no_space
    FindChar rdi , '+'
    cmp rax , 1
    je .concat
    FindChar rdi , '-'
    cmp rax , 1
    je .concat

    cmp byte [rsi] , 10 ; 10 = new line
    je .print_result

    .concat:
    call _concat_buffer
    xor r12 , r12

    jmp resolve_mul_div

    .error_while_processing:
    mov rax , 1
    mov rdi , 1
    mov rdx , 34
    mov rsi , error_message_pf
    syscall
    mov r8 , 1
    ret

    .invalide_math_cdz:
    mov rax , 1
    mov rdi , 1
    mov rdx , 32
    mov rsi , error_message_cdz
    syscall
    mov r8 , 1
    ret 

.return:
    cmp r10 , 1
    je .processing_math
.print_result:
    mov rax , 1
    mov rdi , 1
    mov rsi , buffer
    mov rdx , r15
    inc rdx 
    syscall
    endl
.return_no_print:
ret

_perform_division:
cmp rcx , 0
je _the_loop.invalide_math_cdz

mov rdx , 0
mov rax , rbx
idiv rcx 
;negative number handling:
cmp r12 , 1
jne .no_negative
cmp rax , 0
je .no_negative

IntToStr rax , buffer + 1
mov al , '-'
mov byte [buffer], al 
inc r15
ret

.no_negative:

IntToStr rax , buffer + 1
mov al , '+'
mov byte [buffer], al 
inc r15
ret 

_perform_multiplication:
imul rbx , rcx
; negative number handling: 
cmp r12 , 1
jne .no_negative
cmp rbx , 0
je .no_negative

IntToStr rbx , buffer + 1
mov al , '-'
mov byte [buffer], al 
inc r15
ret

.no_negative:

IntToStr rbx , buffer + 1
mov al , '+'
mov byte [buffer], al 
inc r15
ret 

_concat_buffer:
mov rdi , buffer
dec r15; remove null byte
add rdi , r15;goto string end
;counting rsi length
mov rax , rsi 
xor rbx , rbx
.counting_loop:
mov cl , byte [rax]
inc rbx 
inc rax 
cmp cl , 10
jne .counting_loop

CopyChar rdi , rsi , rbx 
;check if operation
mov rsi , user_input_no_space ;debug here
xor rbx , rbx 
xor rcx , rcx; last operation location pointer
.checking_loop:
    mov al , byte [rsi]
    inc rsi 
    inc rbx
    cmp al , '+'
    je .has_opperation
    cmp al , '-'
    je .has_opperation
    cmp al , '*' 
    je .loop_end
    cmp al , '/'
    je .loop_end

    jmp .checking_loop

    .has_opperation:
    mov rcx , rbx 
    jmp .checking_loop
.loop_end:
    cmp rcx , 0
    je .print_data
    ; found opperation ! --> +5/-5
    ; remove + and - from user_input_no_space beging
    mov rax , user_input_no_space
    add rax , rcx 
    dec rax 
    cmp byte [rax] , '+'
    je .copy_minus_plus
    cmp byte [rax] , '-'
    jne .copy_no_minus_plus
    .copy_minus_plus:
    dec rcx 
    cmp rcx , 0
    je .copy_end
    CopyChar data_buffer , user_input_no_space , rcx
    jmp .copy_end
    .copy_no_minus_plus:
    CopyChar data_buffer , user_input_no_space , rcx 
    .copy_end:
    mov rsi , data_buffer 
    add rsi , rcx ;rsi = end of data_buffer
    StrLen buffer; output = rax
    CopyChar rsi , buffer , rax
    CopyChar buffer , data_buffer , MAX_SIZE
.print_data:
ClearData user_input_no_space , MAX_SIZE
CopyChar user_input_no_space , buffer , MAX_SIZE
StrLenLB user_input_no_space ;output = rax
mov rsi , user_input_no_space
mov rdi , 1
mov rdx , rax
mov rax , 1
syscall
ret 


;           _____  _____              _____ _    _ ____ 
;     /\   |  __ \|  __ \    ___     / ____| |  | |  _ \
;    /  \  | |  | | |  | |  ( _ )   | (___ | |  | | |_) |
;   / /\ \ | |  | | |  | |  / _ \/\  \___ \| |  | |  _ <
;  / ____ \| |__| | |__| | | (_>  <  ____) | |__| | |_) |
; /_/    \_\_____/|_____/   \___/\/ |_____/ \____/|____/


resolve_add_sub:
    ;check if there is addition or substraction
    FindChar rsi , '+'
    cmp rax , 1
    je .next
    
    FindChar rsi , '-'
    cmp rax , 1
    je .next
    ret

    .next:
    inc rsi 
    FindChar rsi , '+'
    cmp rax , 1
    je .next2
    FindChar rsi , '-'
    cmp rax , 1
    je .next2

    cmp byte [rsi] , 10; for '+' and '-' case
    jne .valid_int
    jmp _the_loop_as.error_while_processing 
    
    ;for '-{int}' or '+{int}' case
    .valid_int:
    jmp _the_loop_as.return_no_print  

    .next2:
    dec rsi 
    
    xor r8 , r8 ; 0 = no error / 1 = error
    xor r9 , r9 ; 0 = sub / 1 = add 
    xor r10 , r10 ; 0 = writing first number / 1 = writing second number / ending first number
    xor r12 , r12 ; negative number handling
    mov rbx , first_number 
    mov rcx , second_number 
    ClearData first_number , MAX_SIZE
    ClearData second_number , MAX_SIZE
    ClearData buffer , MAX_SIZE
    _the_loop_as:
    cmp r10 , 2
    je .processing_math
    mov al , byte [rsi]
    inc rsi 
    cmp al, 10 ; check if end of string
    je .return
    cmp al , '+'
    je .addition
    cmp al , '-'
    je .substraction
    cmp al , 48
    jl _the_loop_as
    cmp al , 57
    jle .number
    
    jmp _the_loop_as


    .addition:
    mov r9 , 1
    inc r10
    jmp _the_loop_as

    .substraction:
    cmp byte [first_number] , 0
    je .negative_first_number
    mov r9 , 0
    inc r10
    jmp _the_loop_as
    .negative_first_number:
    mov r12 , 1
    jmp _the_loop_as


    .number:
    cmp r10 , 1
    je .second_number
    ;.first_number:
    mov r15b , [rsi - 1]
    mov byte [rbx] , r15b
    inc rbx
    jmp _the_loop_as
    .second_number:
    mov r15b , [rsi - 1]
    mov byte [rcx] , r15b
    inc rcx 

    cmp byte [rsi] , 48
    jl .processing_math ; not a number !

    jmp _the_loop_as


    .processing_math:
    mov rbx , first_number 
    mov rcx , second_number 

    cmp byte [rcx] , 0
    je .error_while_processing

    StrToInt rbx 
    StrToInt rcx
    cmp r9 , 0
    je .prc_div
    call _perform_addition
    jmp .after_process

    .prc_div:
    call _perform_substraction
    
    .after_process:
    xor r12 , r12

    cmp byte [rsi] , 10 ; 10 = new line
    je .print_result
    call _concat_buffer_as
    jmp resolve_add_sub

    .error_while_processing:
    mov rax , 1
    mov rdi , 1
    mov rdx , 34
    mov rsi , error_message_pf
    syscall
    mov r8 , 1
    ret


.return:
    cmp r10 , 1
    je .processing_math
.print_result:
    mov rax , 1
    mov rdi , 1
    mov rsi , buffer
    mov rdx , r15
    inc rdx 
    syscall
    endl
.return_no_print:
ret

_perform_substraction:
cmp r12 , 0
je .first_number_positive

add rbx , rcx 
IntToStr rbx , buffer + 1

mov rdi , buffer
mov al , '-'
mov byte [rdi], al 
inc r15 ;for the '-' added to the string

jmp .return



.first_number_positive:

cmp rbx , rcx 
jl .negative_res

sub rbx , rcx 
IntToStr rbx , buffer
jmp .return 

.negative_res:
mov r10 , rbx 
mov rbx , rcx 
mov rcx , r10 
sub rbx , rcx 
IntToStr rbx , buffer + 1

mov rdi , buffer
mov al , '-'
mov byte [rdi], al 
inc r15 ;for the '-' added to the string

.return:
ret 

_perform_addition:
cmp r12 , 0
je .first_number_positive


cmp rbx , rcx 
jl .negative_res

sub rbx , rcx 
cmp rbx , 0
jne .not_zero
IntToStr rbx , buffer
jmp .return
.not_zero:
IntToStr rbx , buffer + 1
mov rdi , buffer
mov al , '-'
mov byte [rdi], al 
inc r15 ;for the '-' added to the string
jmp .return

.negative_res:
;switch rcx and rbx
mov r10 , rbx 
mov rbx , rcx 
mov rcx , r10 

sub rbx , rcx 
IntToStr rbx , buffer
jmp .return



.first_number_positive:
add rbx , rcx 
IntToStr rbx , buffer

.return:
ret 

_concat_buffer_as:
mov rdi , buffer
dec r15; remove null byte
add rdi , r15;goto string end
;counting rsi length
mov rax , rsi 
xor rbx , rbx
.counting_loop:
mov cl , byte [rax]
inc rbx 
inc rax 
cmp cl , 10
jne .counting_loop

CopyChar rdi , rsi , rbx 
mov rsi , buffer
ClearData user_input_no_space , MAX_SIZE
CopyChar user_input_no_space , rsi , MAX_SIZE
mov rsi , user_input_no_space
mov rdi , 1
mov rdx , 256
mov rax , 1
syscall

ret

;to do