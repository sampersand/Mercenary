	// Mercenary lexer in ARM64 assembly
	// Compatible with Mac, Linux, and presumably Windows ARM64.
	.set TOKEN_ERROR, 256
	.set TOKEN_IDENTIFIER, 257
	.set TOKEN_NUMBER, 258
	.set TOKEN_STRING, 259

	.set TOKEN_DOUBLE_EQUALS, 260
	.set TOKEN_NOT_EQUALS, 261
	.set TOKEN_GREATER_EQUALS, 262
	.set TOKEN_LESSER_EQUALS, 263

	.set TOKEN_GLOBAL, 300
	.set TOKEN_FUNCTION, 301
	.set TOKEN_IF, 302
	.set TOKEN_ELSE, 303
	.set TOKEN_WHILE, 304
	.set TOKEN_RETURN, 305
	.set TOKEN_DO, 306
	.set TOKEN_LET, 307
	.set TOKEN_TRUE, 308
	.set TOKEN_FALSE, 309
	.set TOKEN_IMPORT, 310
	.set TOKEN_NULL, 311
	.set TOKEN_SET, 312

	.text
	.global read_token
	.global read_token_unix
	.p2align 2,0
	// typedef struct Token {
	//    const char* start;
	//    const char* end;
	//    uint64_t type;
	// } Token;
	// Token read_token(constl char* lexer);
	//
	// x8 - destination token ptr
	// x0 - current stream ptr
	// Thankfully, arm64 doesn't have weird ABI issues because
	// the calling convention is standardized by ARM itself.
read_token:
read_token_unix:
.Lread_space:
	str	x0, [x8]
	ldrb	w2, [x0], #1
	// no naughty unicode (￢‿￢)
	tbnz	w2, #7, .Lerror
	// load byte from jump table
	// aarch64 instructions are always 4 byte aligned
	// address = jump_table - (jump_table[i] << 2)
	adr	x9, .Ljump_table
	ldrb	w3, [x9, x2]
	sub	x9, x9, x3, lsl #2
	br	x9
	// unreachable
.Lread_null:
	sub	x0, x0, #1
	stp	x0, xzr, [x8, #8]
	ret
.Lread_symbol:
	ldrb	w10, [x0]
	cmp	w2, #'/'
	b.eq	.Lslash
.Lnot_comment:
	cmp	w10, #'='
	b.ne	.Lnot_eq
.Lcheck_for_second_eq:
	// branchless for the memes, this is definitely slower
	mov	w3, #TOKEN_DOUBLE_EQUALS
	cmp	w2, #'='         // ==
	csel	w2, w3, w2, eq
	cinc	x0, x0, eq
	cmp	w2, #'!'         // !=
	csinc	w2, w2, w3, ne
	cinc	x0, x0, eq
	add	w3, w3, #2
	cmp	w2, #'>'         // >=
	csel	w2, w3, w2, eq
	cinc	x0, x0, eq
	cmp	w2, #'<'         // <=
	csinc	w2, w2, w3, ne
	cinc	x0, x0, eq
.Lnot_eq:
	stp	x0, x2, [x8, #8]
	ret
.Lslash:
	cmp	w10, #'*'
	b.ne	.Lnot_comment // "/*" (for syntax highlighting */)
.Lread_comment: // skip comment
	add	x0, x0, #2
.Lread_comment.loop:
	ldrb	w10, [x0], #1
	// unterminated comment
	cbz	w10, .Lerror
	cmp	w10, #'*'
	b.ne	.Lread_comment.loop
	ldrb	w10, [x0], #1
	cbz	w10, .Lerror
	cmp	w10, #'/'
	b.eq	read_token
	b	.Lread_comment.loop
.Lescape:
	// ignore the escape sequence unless it is a null terminator
	ldrb	w4, [x0], #1
	cbz	w4, .Lerror
.Lread_string:
	ldrb	w4, [x0], #1
	cbz	w4, .Lerror
	cmp	w4, #'\\'
	b.eq	.Lescape
	cmp	w4, w2 // end quote
	bne	.Lread_string
	mov	w2, #TOKEN_STRING
	stp	x0, x2, [x8, #8]
	ret
.Lread_ident:
	// mask to determine the length
	// we shift right to count, and when it reaches
	// 8 it is zero.
	mov	w7, #1 << 8
.Lread_ident.loop:
	ldrb	w5, [x0], #1
	cmp	w5, #'_'
	b.eq	.Lread_ident.next
	// aarch64 mask immediates are weird
	// isalpha(w5)
	and	w6, w5, #0xffffffdf
	sub	w6, w6, #'A'
	cmp	w6, #26
	b.lo	.Lread_ident.next
	sub	w6, w5, #'0'
	cmp	w6, #9
	b.hi	.Lread_ident.end
.Lread_ident.next:
	// Shift into x2 which contains the last 8 characters
	// It will be in little endian order
	orr	x2, x5, x2, lsl #8
	lsr	w7, w7, #1
	b	.Lread_ident.loop
.Lread_ident.end:
	// Check each keyword
	mov	w10, #TOKEN_IDENTIFIER
	// If w7 is zero, the identifier is longer than 8 bytes.
	cbz	w7, .Lread_ident.keyword_end
	adr	x11, .Ltul_drowyek
	mov	w9, #TOKEN_GLOBAL - 1
.Lkeyword_loop:
	ldr	x12, [x11], #8
	cbz	x12, .Lread_ident.keyword_end
	cmp	x12, x2
	add	w9, w9, #1
	b.ne	.Lkeyword_loop
	mov	w10, w9
.Lread_ident.keyword_end:
	sub	x0, x0, #1
	stp	x0, x10, [x8, #8]
	ret
.Lread_number:
	ldrb	w3, [x0], #1
	sub	w3, w3, #'0'
	cmp	w3, #9
	b.ls	.Lread_number
.Lread_number.end:
	mov	w2, #TOKEN_NUMBER
	sub	x0, x0, #1
	stp	x0, x2, [x8, #8]
	ret
.Lerror:
	mov	w2, #TOKEN_ERROR
	stp	x0, x2, [x8, #8]
	ret
	// .drowyek dengila etyb 8 setaerC
.macro dwk rst
1:
	.ascii	"\rst"
	.zero	8 - (. - 1b)
.endm

	// .redro naidne elttil ni sdrowkeK
	.p2align 3,0
.Ltul_drowyek:
	dwk	"labolg"
	dwk	"noitcnuf"
	dwk	"fi"
	dwk	"esle"
	dwk	"elihw"
	dwk	"nruter"
	dwk	"od"
	dwk	"tel"
	dwk	"erut"
	dwk	"eslaf"
	dwk	"tropmi"
	dwk	"llun"
	dwk	"tes"
	.xword	0 // etanimret llun

.macro case dst
	.byte (.Ljump_table - \dst) >> 2
.endm
.macro cases n, dst
.rept \n
	case \dst
.endr
.endm
	.p2align 2,0
	// jump table
.Ljump_table:
	case	.Lread_null
	cases	8, .Lerror
	case	.Lread_space // tab
	case	.Lread_space // newline
	cases	2, .Lerror
	case	.Lread_space // carriage return
	cases	31 - 13, .Lerror
	case	.Lread_space // space
	case	.Lread_symbol // !
 	case	.Lread_string // "
	cases	4, .Lread_symbol // #$%&
 	case	.Lread_string // '
	cases	8, .Lread_symbol // ()*+,-./
	cases	10, .Lread_number // 0-9
	cases	7, .Lread_symbol // :;<=>?@
	cases	26, .Lread_ident // A-Z
	cases	4, .Lread_symbol // [\]^
	case	.Lread_ident // _
	case	.Lread_symbol // `
	cases	26, .Lread_ident // a-z
	cases	4, .Lread_symbol // {|}~
	case	.Lerror // 0x7F
