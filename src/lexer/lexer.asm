section .text
global read_token
global read_token_unix

%define TOKEN_ERROR 256
%define TOKEN_IDENTIFIER 257
%define TOKEN_NUMBER 258
%define TOKEN_STRING 259

%define TOKEN_DOUBLE_EQUALS 260
%define TOKEN_NOT_EQUALS 261
%define TOKEN_GREATER_EQUALS 262
%define TOKEN_LESSER_EQUALS 263

%define TOKEN_GLOBAL 300
%define TOKEN_FUNCTION 301
%define TOKEN_IF 302
%define TOKEN_ELSE 303
%define TOKEN_WHILE 304
%define TOKEN_RETURN 305
%define TOKEN_DO 306
%define TOKEN_LET 307
%define TOKEN_TRUE 308
%define TOKEN_FALSE 309
%define TOKEN_IMPORT 310
%define TOKEN_NULL 311

; typedef struct Token {
;    const char* start;
;    const char* end;
;    uint64_t type;
; } Token;
; Token read_token(const char* lexer);
;
; RCX - destination token ptr
; RDX - current stream ptr
read_token_unix:
	; the parameters go in RDI and RSI on SysV so was just move those
	; into the ones where we expect things RCX and RDX.
    mov rcx, rdi
    mov rdx, rsi

read_token:
	; branchless space skip
	xor r8d, r8d
	cmp byte [rdx], 0x20
	sete r8b
	add rdx, r8

	; read first byte
	movzx rax, byte [rdx]

	; start classifying the token
	cmp rax, 0
	jz read_null
	
	; **********************************
	; WHITESPACE
	; **********************************
	cmp rax, 0x09 ; tab
	je read_space
	cmp rax, 0x20 ; space
	je read_space
	cmp rax, 0x0D ; carriage return
	je read_space
	cmp rax, 0x0A ; newline
	je read_space

	; **********************************
	; IDENTIFIERS
	; **********************************
	; classify uppercase letters
	lea r8, [rax - 'A']
	cmp r8, 26
	jb read_ident
	
	; classify lowercase letters
	lea r8, [rax - 'a']
	cmp r8, 26
	jb read_ident

	; classify underscore
	cmp rax, '_'
	je read_ident

	; **********************************
	; NUMBERS
	; **********************************
	lea r8, [rax - '0']
	cmp r8, 10
	jb read_number

	; **********************************
	; STRINGS
	; **********************************
	cmp rax, '"'
	je read_string

	; **********************************
	; SYMBOLS
	; **********************************
	; [\]^_@
	lea r8, [rax - '[']
	cmp r8, 7
	jb read_symbol
	; !"#$%&`()*+,-./
	lea r8, [rax - '!']
	cmp r8, 15
	jb read_symbol
	; :;<=>?@
	lea r8, [rax - ':']
	cmp r8, 8
	jb read_symbol
	; {|}~
	lea r8, [rax - '{']
	cmp r8, 5
	jb read_symbol

	; **********************************
	; NOTHING! JUST ERROR OUT
	; **********************************
.exit:
	mov qword [rcx], 0
	mov qword [rcx + 8], 0
	mov qword [rcx + 16], TOKEN_ERROR
	ret

read_null:
	mov qword [rcx], 0
	mov qword [rcx + 8], 0
	mov qword [rcx + 16], 0
	ret

read_space:
	add rdx, 1 ; increment stream ptr
	jmp read_token

read_symbol:
	cmp word [rdx], '=='
	je .double_equals
	cmp word [rdx], '!='
	je .not_equals
	cmp word [rdx], '<='
	je .greater_equals
	cmp word [rdx], '>='
	je .lesser_equals
	cmp word [rdx], '/*'
	je read_comment
	jmp .normal
.not_equals:
	mov qword [rcx], rdx
	add rdx, 2 ; increment stream ptr

	mov qword [rcx + 8], rdx
	mov qword [rcx + 16], TOKEN_NOT_EQUALS
	ret
.double_equals:
	mov qword [rcx], rdx
	add rdx, 2 ; increment stream ptr

	mov qword [rcx + 8], rdx
	mov qword [rcx + 16], TOKEN_DOUBLE_EQUALS
	ret
.greater_equals:
	mov qword [rcx], rdx
	add rdx, 2 ; increment stream ptr

	mov qword [rcx + 8], rdx
	mov qword [rcx + 16], TOKEN_GREATER_EQUALS
	ret
.lesser_equals:
	mov qword [rcx], rdx
	add rdx, 2 ; increment stream ptr

	mov qword [rcx + 8], rdx
	mov qword [rcx + 16], TOKEN_LESSER_EQUALS
	ret
.normal:
	mov qword [rcx], rdx
	add rdx, 1 ; increment stream ptr

	mov qword [rcx + 8], rdx
	mov qword [rcx + 16], rax
	ret

read_comment:
	add rdx, 2 ; increment stream ptr
.loop:
	cmp word [rdx], '*/' ; end comment
	je .exit
	add rdx, 1
	jmp .loop
.exit:
	add rdx, 2 ; skip end comment
	jmp read_token

read_string:
	mov r9, rdx ; save the start position (starting after the quotes)
.loop:
	add rdx, 1

	cmp word [rdx], 0x275C ; \'
	je .escapes
	cmp word [rdx], 0x225C ; \"
	je .escapes
	cmp word [rdx], 0x5C5C ; \\
	je .escapes
	
	; Read normal character, exit if end quote
	cmp byte [rdx], '"'
	je .loop_exit
	jmp .loop
.escapes:
	; We need to skip both the backslash and escape char
	add rdx, 2
	jmp .loop
.loop_exit:
	; output a token
	add rdx, 1 ; skip end quote

	mov qword [rcx], r9
	mov qword [rcx + 8], rdx
	mov qword [rcx + 16], TOKEN_STRING
	ret

read_ident:
	mov r9, rdx ; save the start position
	add rdx, 1 ; increment stream ptr
.loop:
	; Read character
	movzx rax, byte [rdx]
	add rdx, 1

	; Check if the character is still an identifier
	; if it's not return
	; classify uppercase letters
	lea r8, [rax - 'A']
	cmp r8, 26
	jb .loop
	; classify lowercase letters
	lea r8, [rax - 'a']
	cmp r8, 26
	jb .loop
	; classify numbers
	lea r8, [rax - '0']
	cmp r8, 10
	jb .loop
	; classify underscore
	cmp rax, '_'
	je .loop
	; Not an identifier anymore return this token
.loop_exit:
	; subtract one since the last token did not match
	sub rdx, 1

	; Default token type for identifiers
	; If it matches a keyword it'll be changed
	mov rax, TOKEN_IDENTIFIER
	
	; Get length
	mov r10, rdx
	sub r10, r9
	cmp r10, 8
	ja .default_ident
	mov r11, .jump_table
	jmp [r11 + r10 * 8]
.ident_length2:
	movzx r11d, word [r9]
	mov r10d, 0x6F64 ; do
	cmp r11d, r10d
	mov r10d, TOKEN_DO
	cmove rax, r10

	mov r10d, 0x6669 ; if
	cmp r11d, r10d
	mov r10d, TOKEN_IF
	cmove rax, r10
	jmp .default_ident
.ident_length3:
	mov r11d, dword [r9]
	and r11d, 0xFFFFFF ; we only need 3 bytes
	mov r10d, 0x74656C ; let
	cmp r11d, r10d
	mov r10d, TOKEN_LET
	cmove rax, r10
	jmp .default_ident
.ident_length4:
	mov r11d, dword [r9]
	mov r10d, 0x65757274 ; true
	cmp r11d, r10d
	mov r10d, TOKEN_TRUE
	cmove rax, r10

	mov r10d, 0x65736C65 ; else
	cmp r11d, r10d
	mov r10d, TOKEN_ELSE
	cmove rax, r10

	mov r10d, 0x6C6C756E ; null
	cmp r11d, r10d
	mov r10d, TOKEN_NULL
	cmove rax, r10
	jmp .default_ident
.ident_length5:
	mov r10, 0xFFFFFFFFFF
	mov r11, qword [r9]
	and r11, r10 ; Mask out 5 bytes
	mov r10, 0x65736C6166 ; false
	cmp r11, r10
	mov r10d, TOKEN_FALSE
	cmove rax, r10

	mov r10, 0x656C696877 ; while
	cmp r11, r10
	mov r10d, TOKEN_WHILE
	cmove rax, r10
	jmp .default_ident
.ident_length6:
	mov r10, 0xFFFFFFFFFFFF
	mov r11, qword [r9]
	and r11, r10 ; Mask out 6 bytes
	mov r10, 0x6E7275746572 ; return
	cmp r11, r10
	mov r10d, TOKEN_RETURN
	cmove rax, r10

	mov r10, 0x6C61626F6C67 ; global
	cmp r11, r10
	mov r10d, TOKEN_GLOBAL
	cmove rax, r10
	jmp .default_ident

	mov r10, 0x74726F706D69 ; import
	cmp r11, r10
	mov r10d, TOKEN_IMPORT
	cmove rax, r10
	jmp .default_ident
.ident_length8:
	mov r11, qword [r9]
	mov r10, 0x6E6F6974636E7566 ; function
	cmp r11, r10
	mov r10d, TOKEN_FUNCTION
	cmove rax, r10
	; fallthrough
.default_ident:
	; output a token
	mov qword [rcx], r9
	mov qword [rcx + 8], rdx
	mov qword [rcx + 16], rax
	ret
.jump_table:
	dq .default_ident
	dq .default_ident
	dq .ident_length2
	dq .ident_length3
	dq .ident_length4
	dq .ident_length5
	dq .ident_length6
	dq .default_ident
	dq .ident_length8

read_number:
	mov r9, rdx ; save the start position
	add rdx, 1 ; increment stream ptr
.loop:
	; Read character
	movzx rax, byte [rdx]
	add rdx, 1

	; Check if the character is still a number, 
	; if it's not return
	lea r8, [rax - '0']
	cmp r8, 10
	jb .loop
	; Not an identifier anymore return this token
.loop_exit:
	; subtract one since the last token did not match
	sub rdx, 1
 
	; output a token
	mov qword [rcx], r9
	mov qword [rcx + 8], rdx
	mov qword [rcx + 16], TOKEN_NUMBER
	ret
