/* File: src/utils/sprintf.c
	provides ru_sprintf_s 
		author: JazzLeee
		  date: 2019.07.29
 */

#include <taurix.h>

// 修改這裡會改變 printf 的規則和輸出

// 字符類型
typedef char CharType;
// 填充空格
#define char_space ' '
// 填充 0
#define char_zero '0'
// 正號
#define char_plus '+'
// 負號
#define char_minus '-'
// 格式標誌
#define char_format '%'
// 左對齊標誌
#define char_flag_left_justify '-'
// 補0標誌
#define char_flag_padding_zero '0'
// 強制顯示正號標誌
#define char_flag_force_plus_sign '+'
// 強制空格佔符號位標誌
#define char_flag_force_space_sign ' '
// 指示符標誌
#define char_flag_specifier '#'
// %c
#define char_c 'c'
// %s
#define char_s 's'
// %d
#define char_d 'd'
// %i
#define char_i 'i'
// %u
#define char_u 'u'
// %o
#define char_o 'o'
// %x
#define char_x 'x'
// %X
#define char_X 'X'
// %p
#define char_p 'p'
// %P
#define char_P 'P'
// 八進制指示符
#define specifier_o "0o"
// 八進制指示符長度
#define specifier_o_size 2
// 小寫十六進制指示符
#define specifier_x "0x"
// 小寫十六進制指示符長度
#define specifier_x_size 2
// 大寫十六進制指示符
#define specifier_X "0X"
// 大寫十六進制指示符長度
#define specifier_X_size 2
// 場寬字符進制基數
#define width_digit_base 10
// 判斷場寬字符
static inline int is_width_digit(char c) {
	return '0' <= c && c <= '9';
}
// 場寬字符 轉為數字
static inline int width_digit_to_number(char c) {
	return (int)(c - '0');
}
// 八進制字符
static const CharType digits_o[] = {
	'0', '1', '2', '3', '4', '5', '6', '7'
};
// 十進制字符
static const CharType digits[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
};
// 小寫十六進制字符
static const CharType digits_x[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};
// 大寫十六進制字符
static const CharType digits_X[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static inline int32 internal_abs_log_ceil(int32 x, int32 y) {
	if (x < 0)
		x = -x;
	int32 r = 0;
	for (; x > 0; x /= y)
		++r;
	return r;
}

static inline int internal_print_string(
	int(*putchar)(void *o, CharType c),
	void *o,
	const CharType *str
) {
	int v = 0;
	while (*str)
		v += putchar(o, *str++);
	return v;
}

static inline int internal_print_number(
	int(*putchar)(void *o, CharType c),
	void *o,
	uint32 value
) {
	if (value) {
		int v = 0;
		if (value / 10)
			v += internal_print_number(putchar, o, value / 10);
		return v + putchar(o, digits[value % 10]);
	}
	else
		return putchar(o, char_zero);
}

static inline int internal_print_number_o(
	int(*putchar)(void *o, CharType c),
	void *o,
	uint32 value
) {
	if (value) {
		int v = 0;
		if (value / 8)
			v += internal_print_number_o(putchar, o, value / 8);
		return v + putchar(o, digits_o[value % 8]);
	}
	else
		return putchar(o, char_zero);
}

static inline int internal_print_number_x(
	int(*putchar)(void *o, CharType c),
	void *o,
	uint32 value
) {
	if (value) {
		int v = 0;
		if (value / 16)
			v += internal_print_number_x(putchar, o, value / 16);
		return v + putchar(o, digits_x[value % 16]);
	}
	else
		return putchar(o, char_zero);
}

static inline int internal_print_number_X(
	int(*putchar)(void *o, CharType c),
	void *o,
	uint32 value
) {
	if (value) {
		int v = 0;
		if (value / 16)
			v += internal_print_number_X(putchar, o, value / 16);
		return v + putchar(o, digits_X[value % 16]);
	}
	else
		return putchar(o, char_zero);
}

static inline int internal_print_i(
	int(*putchar)(void *o, CharType c),
	void *o,
	int32 value,
	int32 width,
	int flag_left_justify,
	int flag_padding_zero,
	int flag_force_plus_sign,
	int flag_force_space_sign
) {
	int v = 0;
	if (flag_left_justify && flag_padding_zero || flag_force_plus_sign && flag_force_space_sign)
		return v;
	int32 w = (flag_force_plus_sign || flag_force_space_sign || value < 0 ? 1 : 0) + (value ? internal_abs_log_ceil(value, 10) : 1);
	if (flag_padding_zero) {
		if (value < 0)
			v += putchar(o, char_minus);
		else if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_zero);
		if (value < 0)
			v += internal_print_number(putchar, o, -value);
		else
			v += internal_print_number(putchar, o, value);
	}
	else if (flag_left_justify) {
		if (value < 0)
			v += putchar(o, char_minus);
		else if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (value < 0)
			v += internal_print_number(putchar, o, -value);
		else
			v += internal_print_number(putchar, o, value);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
	}
	else {
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
		if (value < 0)
			v += putchar(o, char_minus);
		else if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (value < 0)
			v += internal_print_number(putchar, o, -value);
		else
			v += internal_print_number(putchar, o, value);
	}
	return v;
}

static inline int internal_print_u(
	int(*putchar)(void *o, CharType c),
	void *o,
	uint32 value,
	int32 width,
	int flag_left_justify,
	int flag_padding_zero,
	int flag_force_plus_sign,
	int flag_force_space_sign
) {
	int v = 0;
	if (flag_left_justify && flag_padding_zero || flag_force_plus_sign && flag_force_space_sign)
		return v;
	int32 w = (flag_force_plus_sign || flag_force_space_sign ? 1 : 0) + (value ? internal_abs_log_ceil(value, 10) : 1);
	if (flag_padding_zero) {
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_zero);
		v += internal_print_number(putchar, o, value);
	}
	else if (flag_left_justify) {
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		v += internal_print_number(putchar, o, value);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
	}
	else {
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		v += internal_print_number(putchar, o, value);
	}
	return v;
}

static inline int internal_print_o(
	int(*putchar)(void *o, CharType c),
	void *o,
	uint32 value,
	int32 width,
	int flag_left_justify,
	int flag_padding_zero,
	int flag_force_plus_sign,
	int flag_force_space_sign,
	int flag_specifier
) {
	int v = 0;
	if (flag_left_justify && flag_padding_zero || flag_force_plus_sign && flag_force_space_sign)
		return v;
	int32 w = (flag_force_plus_sign || flag_force_space_sign ? 1 : 0) + (value ? internal_abs_log_ceil(value, 8) : 1) + (flag_specifier ? specifier_o_size : 0);
	if (flag_padding_zero) {
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_o);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_zero);
		v += internal_print_number_o(putchar, o, value);
	}
	else if (flag_left_justify) {
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_o);
		v += internal_print_number_o(putchar, o, value);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
	}
	else {
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_o);
		v += internal_print_number_o(putchar, o, value);
	}
	return v;
}

static inline int internal_print_x(
	int(*putchar)(void *o, CharType c),
	void *o,
	uint32 value,
	int32 width,
	int flag_left_justify,
	int flag_padding_zero,
	int flag_force_plus_sign,
	int flag_force_space_sign,
	int flag_specifier
) {
	int v = 0;
	if (flag_left_justify && flag_padding_zero || flag_force_plus_sign && flag_force_space_sign)
		return v;
	int32 w = (flag_force_plus_sign || flag_force_space_sign ? 1 : 0) + (value ? internal_abs_log_ceil(value, 16) : 1) + (flag_specifier ? specifier_x_size : 0);
	if (flag_padding_zero) {
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_x);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_zero);
		v += internal_print_number_x(putchar, o, value);
	}
	else if (flag_left_justify) {
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_x);
		v += internal_print_number_x(putchar, o, value);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
	}
	else {
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_x);
		v += internal_print_number_x(putchar, o, value);
	}
	return v;
}

static inline int internal_print_X(
	int(*putchar)(void *o, CharType c),
	void *o,
	uint32 value,
	int32 width,
	int flag_left_justify,
	int flag_padding_zero,
	int flag_force_plus_sign,
	int flag_force_space_sign,
	int flag_specifier
) {
	int v = 0;
	if (flag_left_justify && flag_padding_zero || flag_force_plus_sign && flag_force_space_sign)
		return v;
	int32 w = (flag_force_plus_sign || flag_force_space_sign ? 1 : 0) + (value ? internal_abs_log_ceil(value, 16) : 1) + (flag_specifier ? specifier_X_size : 0);
	if (flag_padding_zero) {
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_X);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_zero);
		v += internal_print_number_X(putchar, o, value);
	}
	else if (flag_left_justify) {
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_X);
		v += internal_print_number_X(putchar, o, value);
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
	}
	else {
		for (int32 i = width - w; i > 0; --i)
			v += putchar(o, char_space);
		if (flag_force_plus_sign)
			v += putchar(o, char_plus);
		else if (flag_force_space_sign)
			v += putchar(o, char_space);
		if (flag_specifier)
			v += internal_print_string(putchar, o, specifier_X);
		v += internal_print_number_X(putchar, o, value);
	}
	return v;
}

static inline int internal_print_p(
	int(*putchar)(void *o, CharType c),
	void *o,
	void *ptr
) {
	int v = 0;
	for (int i = sizeof(ptr) - 1; i >= 0; --i) {
		uint8 hex = ((uint8*)&ptr)[i];
		v += putchar(o, digits_x[(hex >> 4) % 16]);
		v += putchar(o, digits_x[hex % 16]);
	}
	return v;
}

static inline int internal_print_P(
	int(*putchar)(void *o, CharType c),
	void *o,
	void *ptr
) {
	int v = 0;
	for (int i = sizeof(ptr) - 1; i >= 0; --i) {
		uint8 hex = ((uint8*)& ptr)[i];
		v += putchar(o, digits_X[(hex >> 4) % 16]);
		v += putchar(o, digits_X[hex % 16]);
	}
	return v;
}

static inline int internal_vprintf(
	int(*putchar)(void *o, CharType c),
	void *o,
	const CharType *fmt,
	va_list ap
) {
	// 0: normal
	// 1: char_format
	// 2: char_format width_digit
	int state = 0;
	int v = 0;
	int flag_left_justify,
		flag_padding_zero,
		flag_force_plus_sign,
		flag_force_space_sign,
		flag_specifier,
		width;
	while (1) {
		CharType c = *fmt++;
		loop:
		if (c) {
			switch (state) {
			case 0:
				if (c == char_format) {
					state = 1;
					flag_left_justify = 0;
					flag_padding_zero = 0;
					flag_force_plus_sign = 0;
					flag_force_space_sign = 0;
					flag_specifier = 0;
					width = 0;
				}
				else
					v += putchar(o, c);
				break;
			case 1:
				switch (c) {
				case char_format:
					v += putchar(o, char_format);
					break;
				case char_flag_left_justify:
					flag_left_justify = 1;
					break;
				case char_flag_padding_zero:
					flag_padding_zero = 1;
					break;
				case char_flag_force_plus_sign:
					flag_force_plus_sign = 1;
					break;
				case char_flag_force_space_sign:
					flag_force_space_sign = 1;
					break;
				case char_flag_specifier:
					flag_specifier = 1;
					break;
				case char_c:
					v += putchar(o, va_arg(ap, char));
					state = 0;
					break;
				case char_s:
					v += internal_print_string(putchar, o, va_arg(ap, const char*));
					state = 0;
					break;
				case char_d:
				case char_i:
					v += internal_print_i(putchar, o, va_arg(ap, int32), width, flag_left_justify, flag_padding_zero, flag_force_plus_sign, flag_force_space_sign);
					state = 0;
					break;
				case char_u:
					v += internal_print_u(putchar, o, va_arg(ap, uint32), width, flag_left_justify, flag_padding_zero, flag_force_plus_sign, flag_force_space_sign);
					state = 0;
					break;
				case char_o:
					v += internal_print_o(putchar, o, va_arg(ap, uint32), width, flag_left_justify, flag_padding_zero, flag_force_plus_sign, flag_force_space_sign, flag_specifier);
					state = 0;
					break;
				case char_x:
					v += internal_print_x(putchar, o, va_arg(ap, uint32), width, flag_left_justify, flag_padding_zero, flag_force_plus_sign, flag_force_space_sign, flag_specifier);
					state = 0;
					break;
				case char_X:
					v += internal_print_X(putchar, o, va_arg(ap, uint32), width, flag_left_justify, flag_padding_zero, flag_force_plus_sign, flag_force_space_sign, flag_specifier);
					state = 0;
					break;
				case char_p:
					v += internal_print_p(putchar, o, va_arg(ap, void*));
					state = 0;
					break;
				case char_P:
					v += internal_print_P(putchar, o, va_arg(ap, void*));
					state = 0;
					break;
				default:
					if (is_width_digit(c)) {
						width = width_digit_to_number(c);
						state = 2;
					}
					else
						state = 0;
				}
				break;
			case 2:
				if (is_width_digit(c))
					width = width * width_digit_base + width_digit_to_number(c);
				else {
					state = 1;
					goto loop;
				}
				break;
			}
		}
		else
			return v + putchar(o, '\0');
	}
}

struct String {
	char *buf;
	size_t size;
};

static inline int String_putchar(void *ptr, CharType c) {
	struct String *string = (struct String*)ptr;
	if (string->size > 1) {
		*string->buf++ = c;
		--string->size;
		return 1;
	}
	else if (string->size == 1 && c == '\0') {
		*string->buf++ = '\0';
		--string->size;
	}
	return 0;
}

int ru_vsprintf_s(char *buf, size_t size, const CharType *fmt, va_list ap) {
	struct String string = { buf, size };
	return internal_vprintf(String_putchar, &string, fmt, ap);
}

int ru_sprintf_s(char *buf, size_t size, const CharType *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int v = ru_vsprintf_s(buf, size, fmt, ap);
	va_end(ap);
	return v;
}