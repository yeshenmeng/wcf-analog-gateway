#include "string_operate.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h> 
#include <string.h> 


/**
 * @brief  ʮ�������ַ���ת��Ϊ�ֽ���
 *         exp��src[]="AD12E3C5"-->dst[]={0XAD,0X12,0XE3,0XC5}
 * @param  src: Դʮ�������ַ���
 * @param  dst: Ŀ���ֽ�������
 * @param  src_len: �ַ�������strlen(source)
 * @retval None
 */
void hex_string_to_bytes(const char* src, unsigned char* dst, int src_len)
{
    unsigned char high_byte;
	unsigned char low_byte;

    for (short i = 0; i < src_len; i += 2)
    {
        high_byte = toupper(src[i]); //��Сд��ĸת��Ϊ��д��ĸ
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
 * @brief  �ֽ�������ת��Ϊʮ�������ַ���
 *         exp��src[]={0XAD,0X12,0XE3,0XC5}-->dst[]="AD12E3C5"
 * @param  src: Դ�ֽ�������
 * @param  dst: Ŀ��ʮ�������ַ���
 * @param  src_len: �ֽ������鳤��sizeof(src)
 * @param  upper_lower: 0ת�������ĸΪ��д��1ת�������ĸΪСд
 * @retval None
 */
void bytes_to_hex_string(const unsigned char* src, char* dst, int src_len, unsigned char upper_lower)
{
    char tmp[3];

    for(int i = 0; i < src_len; i++)
    {
		(upper_lower) ? 
			(sprintf(tmp, "%02x", (unsigned char)src[i])) : //���ÿ⺯�����ֽ�����ת��Ϊ16�����ַ�����Сд��
			(sprintf(tmp, "%02X", (unsigned char)src[i]));	//���ÿ⺯�����ֽ�����ת��Ϊ16�����ַ�������д��
        memcpy(&dst[i*2], tmp, 2);
    }
	
	dst[src_len*2] = '\0';

    return ;
}

/**
 * @brief  �ֽ�������ת��Ϊ�ַ���
 *         exp��src[]={0X31,0X32,0X33,0X34}-->dst[]="1234"
 * @param  src: Դ�ֽ�������
 * @param  dst: Ŀ���ַ���
 * @param  src_len: �ֽ������鳤��sizeof(src)
 * @retval None
 */
void bytes_to_char(const unsigned char* src, char* dst, int src_len)
{
	memcpy(dst, src, src_len);
}

/**
 * @brief  ���ַ����в���Ŀ���ַ���
 *         exp��src[]="abcdef" dst[]="de"
 * @param  src: Դ�ַ���
 * @param  dst: Ŀ���ַ���
 * @retval -1������ʧ��
 *         ������Ŀ���ַ�����Դ�ַ����г��ֵ���ʼλ��
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
 * @brief  �����ַ���ת��Ϊ����
 *         exp��src[]="-1234" dst=-1234
 * @param  src: Դ�ַ���
 * @param  dst: Ŀ������
 * @retval -1��Դ�ַ����а����Ƿ��ַ���
 *         0��ת���ɹ�
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
		if(!isdigit(*str)) //�ж��ַ��Ƿ��������ַ�
		{
			return -1;
		}
		str++;
	}
	
	*dst = atoi(src); //�⺯��asciiת��Ϊint
	
	return 0;
}

/**
 * @brief  �����ַ���ת��Ϊ������
 *         exp��src[]="-1234567890123" dst=-1234567890123
 * @param  src: Դ�ַ���
 * @param  dst: Ŀ�곤����
 * @retval -1��Դ�ַ����а����Ƿ��ַ���
 *         0��ת���ɹ�
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
		if(!isdigit(*str)) //�ж��ַ��Ƿ��������ַ�
		{
			return -1;
		}
		str++;
	}
	
	*dst = atoll(src); //�⺯��asciiת��Ϊlong
	
	return 0;
}

/**
 * @brief  �����ַ���ת��Ϊ������
 *         exp��src[]="-0.3254" dst=-0.3254
 * @param  src: Դ�ַ���
 * @param  dst: Ŀ�긡����
 * @retval -1��Դ�ַ����а����Ƿ��ַ���
 *         0��ת���ɹ�
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
		
		if(!isdigit(*str)) //�ж��ַ��Ƿ��������ַ�
		{
			return -1;
		}
		
		str++;
	}
	
	*dst = atof(src); //�⺯��asciiת��Ϊfloat
	
	return 0;
}

/**
 * @brief  �ж�һ���ַ��Ƿ���16�����ַ�
 * @param  src: Դ�ַ�
 * @retval 0��Դ�ַ�����16�����ַ�
 *         ������Դ�ַ���16�����ַ�
 */
int is_hex_or_digit(const char src)
{
	return isxdigit(src);  //�⺯���ж�һ���ַ��Ƿ���16�����ַ�
}


