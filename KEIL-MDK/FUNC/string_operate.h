#ifndef __STRING_OPERATE_H__
#define __STRING_OPERATE_H__


void hex_string_to_bytes(const char* src, unsigned char* dst, int src_len);
void bytes_to_hex_string(const unsigned char* src, char* dst, int src_len, unsigned char upper_lower);
void bytes_to_char(const unsigned char* src, char* dst, int src_len);
int string_contains(const char *src, const char *dst);
int string_to_integer(const char* src, int* dst);
int string_to_long_integer(const char* src, long long* dst);
int string_to_float(const char* src, float* dst);
int is_hex_or_digit(const char src);


#endif




