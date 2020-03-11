#include "string_operate.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h> 
#include <string.h> 


/**
 * @brief  十六进制字符串转换为字节流
 *         exp：src[]="AD12E3C5"-->dst[]={0XAD,0X12,0XE3,0XC5}
 * @param  src: 源十六进制字符串
 * @param  dst: 目标字节流数组
 * @param  src_len: 字符串长度strlen(source)
 * @retval None
 */
void hex_string_to_bytes(const char* src, unsigned char* dst, int src_len)
{
    unsigned char high_byte;
	unsigned char low_byte;

    for (short i = 0; i < src_len; i += 2)
    {
        high_byte = toupper(src[i]); //将小写字母转换为大写字母
        low_byte  = toupper(src[i + 1]);

        if (high_byte > 0x39)
            high_byte -= 0x37;
        else
            high_byte -= 0x30;

        if (low_byte > 0x39)
            low_byte -= 0x37;
        else
            low_byte -= 0x30;

        dst[i/2] = (high_byte << 4) | low_byte;
    }
}

/**
 * @brief  字节流数组转换为十六进制字符串
 *         exp：src[]={0XAD,0X12,0XE3,0XC5}-->dst[]="AD12E3C5"
 * @param  src: 源字节流数组
 * @param  dst: 目标十六进制字符串
 * @param  src_len: 字节流数组长度sizeof(src)
 * @param  upper_lower: 0转换后的字母为大写，1转换后的字母为小写
 * @retval None
 */
void bytes_to_hex_string(const unsigned char* src, char* dst, int src_len, unsigned char upper_lower)
{
    char tmp[3];

    for(int i = 0; i < src_len; i++)
    {
		(upper_lower) ? 
			(sprintf(tmp, "%02x", (unsigned char)src[i])) : //利用库函数将字节数组转换为16进制字符串（小写）
			(sprintf(tmp, "%02X", (unsigned char)src[i]));	//利用库函数将字节数组转换为16进制字符串（大写）
        memcpy(&dst[i*2], tmp, 2);
    }
	
	dst[src_len*2] = '\0';

    return ;
}

/**
 * @brief  字节流数组转换为字符串
 *         exp：src[]={0X31,0X32,0X33,0X34}-->dst[]="1234"
 * @param  src: 源字节流数组
 * @param  dst: 目标字符串
 * @param  src_len: 字节流数组长度sizeof(src)
 * @retval None
 */
void bytes_to_char(const unsigned char* src, char* dst, int src_len)
{
	memcpy(dst, src, src_len);
}

/**
 * @brief  在字符串中查找目标字符串
 *         exp：src[]="abcdef" dst[]="de"
 * @param  src: 源字符串
 * @param  dst: 目标字符串
 * @retval -1：查找失败
 *         其它：目标字符串在源字符串中出现的起始位置
 */
int string_contains(const char *src, const char *dst)
{
    int i = 0;
    while (src[i] != '\0') 
	{
        if (src[i] != dst[0])
		{
            i++;
            continue;
        }
		
		int j = 0;
        while (src[i+j] != '\0' && dst[j] != '\0') 
		{
            if (src[i+j] != dst[j]) 
			{
                break;
            }
            j++;
        }
		
        if (dst[j] == '\0') return i;
        i++;
    }
	
    return -1;
}

/**
 * @brief  数字字符串转换为整数
 *         exp：src[]="-1234" dst=-1234
 * @param  src: 源字符串
 * @param  dst: 目标整数
 * @retval -1：源字符串中包含非法字符串
 *         0：转换成功
 */
int string_to_integer(const char* src, int* dst)
{
	const char* str = src;
	
	if(*str != '-' && !isdigit(*str))
	{
		return -1;
	}
	
	str++;
	
	while(*str)
	{
		if(!isdigit(*str)) //判断字符是否是数字字符
		{
			return -1;
		}
		str++;
	}
	
	*dst = atoi(src); //库函数ascii转换为int
	
	return 0;
}

/**
 * @brief  数字字符串转换为长整数
 *         exp：src[]="-1234567890123" dst=-1234567890123
 * @param  src: 源字符串
 * @param  dst: 目标长整数
 * @retval -1：源字符串中包含非法字符串
 *         0：转换成功
 */
int string_to_long_integer(const char* src, long long* dst)
{
	const char* str = src;
	
	if(*str != '-' && !isdigit(*str))
	{
		return -1;
	}
	
	str++;
	
	while(*str)
	{
		if(!isdigit(*str)) //判断字符是否是数字字符
		{
			return -1;
		}
		str++;
	}
	
	*dst = atoll(src); //库函数ascii转换为long
	
	return 0;
}

/**
 * @brief  数字字符串转换为浮点数
 *         exp：src[]="-0.3254" dst=-0.3254
 * @param  src: 源字符串
 * @param  dst: 目标浮点数
 * @retval -1：源字符串中包含非法字符串
 *         0：转换成功
 */
int string_to_float(const char* src, float* dst)
{
	const char* str = src;
	
	if(*str != '-' && !isdigit(*str) && *str != '.')
	{
		return -1;
	}
	
	str++;
	unsigned char decimal_point = 0;
	
	while(*str)
	{
		if(*str == '.')
		{
			if(++decimal_point > 1 && *(str+1) != '\0')
			{
				return -1;
			}
			
			str++;
			continue;
		}
		
		if(!isdigit(*str)) //判断字符是否是数字字符
		{
			return -1;
		}
		
		str++;
	}
	
	*dst = atof(src); //库函数ascii转换为float
	
	return 0;
}

/**
 * @brief  判断一个字符是否是16进制字符
 * @param  src: 源字符
 * @retval 0：源字符不是16进制字符
 *         其它：源字符是16进制字符
 */
int is_hex_or_digit(const char src)
{
	return isxdigit(src);  //库函数判断一个字符是否是16进制字符
}


