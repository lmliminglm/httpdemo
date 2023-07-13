
/*[
 *      Name:                   adm_string.c
 *
 *      Project:                adm
]*/

/*! \internal
 * \file
 *          Provides a reference implementation of a string handling API
 *          required by ADM.
 *
 * \brief   String handling APIs
 */

#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <adm_typedefs.h>
#include <adm_string.h>
#include <adm_memory.h>
int adm_tolower(int c)  
{  
	if (c >= 'A' && c <= 'Z')  
	{  
		return c + 'a' - 'A';  
	}  
	else  
	{  
		return c;  
	}  
}  
int htoi(char s[])  
{  
    int i;  
    int n = 0;  
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X'))  
    {  
        i = 2;  
    }  
    else  
    {  
        i = 0;  
    }  
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i)  
    {  
        if (adm_tolower(s[i]) > '9')  
        {  
            n = 16 * n + (10 + adm_tolower(s[i]) - 'a');  
        }  
        else  
        {  
            n = 16 * n + (adm_tolower(s[i]) - '0');  
        }  
    }  
    return n;  
}
/*!
*******************************************************************************
 * Calculates the length of a null terminated byte character string.
 *
 * \param   string      The string.
 *
 * \return  The length of the string in bytes.
*******************************************************************************
 */
D32U adm_strlen(const char *string)
{
	if(!string) return 0;
	
    return strlen(string);
}

/*!
*******************************************************************************
 * Calculates the length of a null terminated byte character string with  protected max len.
 *
 * \param   string      The string.
 * \param   max_len     The max length of string.
 *
 * \return  The length of the string in bytes.
*******************************************************************************
 */
D32U adm_strnlen(const char *str, D32U max_len)
{
	if(!str || max_len==0) return 0;
	
    const char * end = (const char *)adm_memchr(str, '\0', max_len);
    if (end == NULL)
        return max_len;
    else
        return end - str;
}


/*!
*******************************************************************************
 * Compare two null terminated byte character strings ignoring case.
 *
 * \param   string1     A string.
 * \param   string2     A string.
 *
 * \return  0 if string1 == string2
 *          < 0 if string1 < string2
 *          > 0 if string1 > string2
*******************************************************************************
 */

D32S adm_strcasecmp(const char *string1, const char *string2)
{
    int c1, c2;
    do {
        c1 = tolower(*string1++);
        c2 = tolower(*string2++);
    } while ((c1 == c2) && (c1 != '\0') && (c2 != '\0'));

    return (D32S) c1 - c2;
}

/*!
*******************************************************************************
 * Concatenate a null terminated byte character string to the specified null
 * terminated byte character string.
 *
 * \param   ioDest      The destination string.
 * \param   inSrc       The source string to concatenate.
 * \param   dstLlen     The destination buf size include dest and source string total length+1.
 *
 * \return  A pointer to the destination null terminated byte character string.
*******************************************************************************
 */ 
DCHAR *adm_strcat (DCHAR *ioDest, const DCHAR *append, DSIZE dstlen)
{
	DSIZE len1=0,len2=0;

	if (!ioDest || !append || (dstlen==0))
		return NULL;

	len1 = strlen(ioDest);
	len2 = strlen(append);

	if (dstlen < (len1 + len2 + 1))
		return NULL;

#if 1
	memcpy(ioDest + len1, append, len2 + 1);
	return ioDest;
#else
	return strcat(ioDest, append);
#endif

}

/*!
*******************************************************************************
 * Concatenate a null terminated byte character string to the specified null
 * terminated byte character string .
 * It will append at most size - strlen(ioDest) - 1 bytes, NUL-terminating the
 * result.
 * 把append所指字符串的前appendLen个字符添加到ioDest所指字符串的结尾处，并覆盖ioDest所指
 * 字符串结尾的'\0'，从而实现字符串的连接
 * 
 * \param   ioDest     The destination string.
 * \param   append     The source string to concatenate. 要追加的字符串
 * \param   appendLen  Size of source string to concatenate. 要追加的字符个数
 * \param   dstLen     Size of destination buffer with null terminated char. 目标缓冲区最大长度（包括额外的空字符）
 *
 * \return  The destination string pointer.返回一个指向最终的目标字符串ioDest的指针
*******************************************************************************
 */
DCHAR *adm_strncat (DCHAR *ioDest, const DCHAR *append, DSIZE appendLen, DSIZE dstLen)
{
  DSIZE len1=0,len2=0;

  if (!ioDest || !append || (appendLen==0) || (dstLen==0))
	  return NULL;

  len1 = strlen(ioDest);
  len2 = strnlen(append, appendLen);

  if (dstLen < (len1 + len2 + 1))
    return NULL;

  /* memmove() all but the NUL, since it might not actually be NUL */
  memcpy(ioDest + len1, append, len2);
  ioDest[len1 + len2] = '\0';

  return ioDest;
}


/*!
*******************************************************************************
 * Search for a character in a null terminated byte character string.
 *
 * \param   inString        The string.
 * \param   inChar          The character.
 *
 * \return  A pointer to the first occurrence of the character in the string,
 *          or NULL if the character is not found.
*******************************************************************************
 */
char *adm_strchr(const char *inString, char inChar)
{
    char *retval = (char *)inString;

    if (!inString)
        return NULL;

    retval = strchr(inString, inChar);

    return retval;
}

/*!
*******************************************************************************
 * Compare for two strings in a null terminated.
 *
 * \param   inString1        The string.
 * \param   inString2        The character.
 *
 * \return  
 *          
*******************************************************************************/
D32S adm_strcmp(const char *inString1, const char *inString2)
{
    D32S retval;

    if (!inString1 && inString2) {
        retval = -1;
        goto end;
    }
    if (inString1 && !inString2) {
        retval = 1;
        goto end;
    }
    if (!inString1 && !inString2) {
        retval = 0;
        goto end;
    }
    retval = strcmp(inString1, inString2);

end:
    return retval;
}

/*!
*******************************************************************************
 * Copy a null terminated byte character string to the other null
 * terminated byte character string.
 *
 * \param   outDest      The destination string.
 * \param   inSrc        The source string.
 * \param   dstLen       Size of the destination buffer.
 *
 * \return  pointer the destination string outDest. 该函数返回一个指向目标存储区outDest的指针
 *          
*******************************************************************************/
DCHAR *adm_strcpy(DCHAR *outDest, const DCHAR *inSrc, DSIZE dstLen)
{
	DSIZE len=0;

	if (!outDest || !inSrc || (dstLen==0))
		return NULL;

	len = adm_strlen(inSrc);
	if (len >= dstLen)
		return NULL;

	//return memcpy(outDest, inSrc, len + 1);
	return strcpy(outDest, inSrc);
}


//-----------------------
// adm_snprintf
//-----------------------
D32S adm_snprintf(DCHAR *outBuffer, D32U inCount, DCHAR *inFormat, ...)
{
    D32S result=0;

	if(!outBuffer)
		return 0;
	
    va_list argp;

    va_start(argp, inFormat);

    //adm_snprintf() should return -1 if the buffer is too short
    result = vsnprintf(outBuffer, inCount, inFormat, argp);

    if (result >= inCount)
        result = -1;

    va_end(argp);

    return result;
}

DCHAR *adm_strjoin(DCHAR *inSeparator, ...)
{
    size_t len = 0;
    size_t separator_len;
    char *retbuf;
    va_list argp;
    char *p;

    if (!inSeparator)
        inSeparator = "";

    separator_len = strlen(inSeparator);
    va_start(argp, inSeparator);
    p = va_arg(argp, char *);
    if (p) {
        /* calculate total length of result string*/

        len = 1 + strlen(p); /* +1 for trailing \0 */

        while ((p = va_arg(argp, char *))) {
            len += separator_len + strlen(p);
        }
        va_end(argp);

        /* now build the result string */
        retbuf = adm_malloc(len);
        if (!retbuf)
            return NULL;        /* error */

        /* copy first string */
        va_start(argp, inSeparator);
        p = va_arg(argp, char *);
        strcpy(retbuf, p);

        /* concatenate the rest of the strings
           separated by the separator */
        while ((p = va_arg(argp, char *))) {
            strcat(retbuf, inSeparator);
            strcat(retbuf, p);
        }
    } else {
        retbuf = adm_malloc(2);
        if (retbuf) {
            *retbuf = 0;
        }
    }
    va_end(argp);

    return retbuf;
}

DCHAR *adm_strlwr(DCHAR *string)
{
    char *i=NULL;
	if(!string)
		return NULL;
	
    for (i = string; *i; i++)
        *i = (char) tolower(*i);
    return string;
}

/*!
*******************************************************************************
 * Compare len characters of two null terminated byte character strings ignoring
 * case.
 *
 * \param   string1     A string.
 * \param   string2     A string.
 * \param   len         Number of characters to compare
 *
 * \return  0 if string1 == string2
 *          < 0 if string1 < string2
 *          > 0 if string1 > string2
*******************************************************************************
 */
D32S adm_strncasecmp(const char *string1, const char *string2, D32U len)
{
    D32U counter = 0;
    int c1, c2;

	if(!string1 || !string2)
		return 0;

    if (!len)
        return 0;

    do {
        c1 = tolower(*string1++);
        c2 = tolower(*string2++);
        counter++;
    } while ((c1 == c2) && (c1 != '\0') && (c2 != '\0') && (counter < len));

    return (D32S)c1 - c2;
}

D32S adm_strncmp(const char *inString1, const char *inString2, D32U inLen)
{
    if (!inString1 && inString2) {
        return -1;
    }
	
    if (inString1 && !inString2) {
        return 1;
    }
	
    if (!inString1 && !inString2) {
        return 0;
    }

    return strncmp(inString1, inString2, inLen);
}

/*!
*******************************************************************************
 * Copy src to dest with checking of destination buffer overflow
 * terminated byte character string.
 *
 * \param   inSrc        The source string.
 * \param   outDest      The destination string.
 * \param   srcLen       Size of the source string which copy. 要从源中复制的字符数
 * \param   dstLen       Size of the destination string buffer. 
 *
 * \return  NULL if copy fail
 * \return  dest pointer if success
 *          
*******************************************************************************/
DCHAR *adm_strncpy(DCHAR *outDest, const DCHAR *inSrc, DSIZE srcLen, DSIZE dstLen)
{
	DSIZE len = 0;
	
    if (!outDest || !inSrc || (srcLen==0) || (dstLen==0))
    {
        return NULL;
    }
	
	len = adm_strnlen(inSrc, dstLen);
	if (len >= dstLen)
	  return NULL;//__chk_fail ();

	if(len > srcLen)
		len = srcLen;
	
    return strncpy (outDest, inSrc, len);
    //return adm_memcpy (outDest, inSrc, len + 1);
}

/*!
*******************************************************************************
 * Copy src to dest with checking of destination buffer overflow
 * terminated byte character string.
 *
 * \param   inSrc        The source string.
 * \param   outDest      The destination string.
 * \param   srcLen       Size of the source string which copy. 要从源中复制的字符数
 * \param   dstLen       Size of the destination string buffer. 
 *
 * \return  0 if copy fail
 * \return  Size of copy string
 *          
*******************************************************************************
*/
DSIZE adm_strlcpy(DCHAR *outDest, DCHAR *inSrc, DSIZE srcLen, DSIZE dstLen)
{
	DSIZE len = 0;
	
    if (!outDest || !inSrc || (srcLen==0) || (dstLen==0))
    {
        return 0;
    }
	
	len = adm_strnlen(inSrc, dstLen);
	if (len >= dstLen)
	  return 0;//__chk_fail ();

	if(len > srcLen)
		len = srcLen;
	
    strncpy (outDest, inSrc, len);

	return len;
}

char *adm_strrchr(const char *inString, char inChar)
{
    char *retval = (char *)inString;

    if (!inString)
        goto end;

    retval = strrchr(inString, inChar);

end:
    return retval;
}

char *adm_strstr(const char *inHaystack, const char *inNeedle)
{
    if (!inNeedle || !inHaystack)
        return NULL;
	
    return strstr(inHaystack, inNeedle);
}

/*
 * Convert a string to an unsigned long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
unsigned long adm_strtoul(const char *inString, char **ioEndString, D8U inRadix, DBOOL *outIsSuccess)
{
    DBOOL result = TRUE;
	int c;
    int any;
    int cutlim;
	int neg = 0;
    int base = (int)inRadix;
	unsigned long acc;
	unsigned long cutoff;
	const char *s = inString;

    if (!inString) {
	    acc = ULONG_MAX;
		result = FALSE;
        goto end;
    }
	/*
	 * See strtol for comments as to the logic used.
	 */
	do {
		c = *s++;
	} while (isspace(c));

	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;

	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}

	if (base == 0)
		base = (c == '0') ? 8 : 10;

	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;

	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}

	if (any < 0) {
		acc = ULONG_MAX;
		result = FALSE;
	} else if (neg)
		acc = -acc;

	if (ioEndString != 0)
		*ioEndString = (char *) (any ? s - 1 : inString);

end:
    /* Return a success/fail indication */
    if (outIsSuccess) {
        *outIsSuccess = result;
    }

	return (acc);
}

/*! \internal
 *  \file
 *      Convert an ascii string representing a positive integer value expressed
 *      in decimal or hexadecimal into an D32U value.
 *
 *  \brief  Convert a string into an D32U
 */


/*
 * This value is used in the parsing code to trap cases where the value so far
 * parsed will become too large to be legal when the next digit is appended.
 */
#define MAX_D32U_DIV_TEN    ( (D32U)0xffffffff / 10 )

/*
 * The largest allowable value is 4294967295. If the value so far parsed is
 * equal to MAX_D32U_DIV_TEN, check the digit being appended is less than or
 * equal to one of these.
 */
#define MAX_D32U_MOD_TEN    ( (D32U)0xffffffff % 10 )

D32U adm_atoD32U(const char *inString, D8U inRadix, DBOOL *outIsSuccess)
{
    D32U    myVal = 0;
    char    maxLastDigit = (char)('0' + MAX_D32U_MOD_TEN);
    int     digitsParsed = 0;
    char    nextChr;
    DBOOL   result = FALSE;

    if (!inString)
        goto end;

    if (10 == inRadix) {
        /*
         * This code will allow the following to be considered legal.
         * '0003' parses to 3
         * '0000000000000000018' parses to 18
         * '+0' is illegal
         * '-0' is illegal
         */
        nextChr = *inString++;

        while ((nextChr >= '0') && (nextChr <= '9')) {
            if ((myVal > MAX_D32U_DIV_TEN) ||
                ((myVal == MAX_D32U_DIV_TEN) && (nextChr > maxLastDigit))) {
                /* Value too large */
                break;
            }
            myVal *= 10;
            myVal += (D32U)(nextChr - '0');
            nextChr = *inString++;
            ++digitsParsed;
        }

        /* Check that
         * - We DID parse at least 1 digit
         * - We've reached the NUL. If we haven't, there are either invalid
         *   characters following the value, or the value is too large to fit
         *   into an D32U.
         */
        if (('\0' == nextChr) && (0 < digitsParsed)) {
            result = TRUE; /* Success */
        }
    } else {
        char    cVal;

        /* Any other radix is treated as 16 */

        if (('0' == *inString) && (('x' == *(inString + 1)) || ('X' == *(inString + 1))))
            inString += 2;

        cVal = *inString++;

        while ('\0' != cVal) {
            if (('0' <= cVal) && (cVal <= '9')) {
                myVal = (myVal << 4) | (D32U)(cVal - '0');
            } else if (('A' <= cVal) && (cVal <= 'F')) {
                myVal = (myVal << 4) | (D32U)(cVal - 'A' + 10);
            } else  if (('a' <= cVal) && (cVal <= 'f')) {
                myVal = (myVal << 4) | (D32U)(cVal - 'a' + 10);
            } else
                break;

            cVal = *inString++;

            if (++digitsParsed == 8)
                break;
        }

        /*
         * Parsing a hexadecimal value is made stricter than a decimal value
         * Maximum of 8 digits,
         * All the characters must be valid hexadecimal digits
         * Allows the value to start '0x' or '0X'
         * No signs allowed
         */
        if ((0 < digitsParsed) && (digitsParsed < 9) && ('\0' == cVal)) {
            result = TRUE; /* Successfully parsed the value */
        }
    }

    /* Return a success/fail indication */
    if (outIsSuccess)
        *outIsSuccess = result;

end:
    return myVal;
}

DBOOL adm_isspace(const char inChar)
{
    return (isspace(inChar)) ? TRUE : FALSE;
}

/*!
*******************************************************************************
 * Convert an integer value into a decimal or hexadecimal string
 * representation. The supplied buffer should be large enough to receive the
 * generated string (11 bytes for decimal, 9 bytes for hexadecimal). Note that
 * this function does NOT prepend the value with '0x' for hexadecimal values.
 *
 * \param   value       The integer value to convert
 *
 * \param   radix       The base to use for conversion (10 decimal, 16 hex)
 *
 * \retval  string      Pointer to a buffer to receive the ascii
 *                      representation.
*******************************************************************************
 */
void adm_D32Utoa(D32U value, char *string, D8U radix)
{
    int     i = 0;
    D32U    v = value;

    if (16 == radix) {

        do {
            v = v >> 4;
            ++i;
        } while (v != 0);

        /*
         * Add NUL terminator.
         */
        string[i] = '\0';

        /*
         * Write the digits in reverse order.
         */
        for (--i; i >= 0; --i) {
            char ch = (char)('0' + (value & 0xF));
            if (ch > '9') {
                ch = (char)(ch + 'A' - ('9' + 1));
            }

            string[i] = ch;
            value = value >> 4;
        }

    } else {
        /*
         * Any other radix will be treated as decimal
         */

        /*
         * Work out the number of digits.
         */
        do {
            i++;
            v /= 10;
        } while (v != 0);

        /*
         * Add NUL terminator.
         */
        string[i] = 0;

        /*
         * Write the digits in reverse order.
         */
        for (--i; i >= 0; --i) {
            string[i] = (char)('0' + value % 10);
            value /= 10;
        }
    }
}


DCHAR *_adm_strdup(const DCHAR *str, const DCHAR *funcname, D32U line)
{
	DSIZE len=0;
	DCHAR *newstr=NULL;

	if(!str)
	return (DCHAR *)NULL;

	len = adm_strlen(str);

	//if(len >= ((DSIZE)-1) / sizeof(DCHAR))
	//  return (DCHAR *)NULL;
	//need if max len limite
	if(len==0)
		return (DCHAR *)NULL;

	newstr = __adm_malloc(len*sizeof(DCHAR)+1, funcname, line);
	if(!newstr)
		return (DCHAR *)NULL;

	adm_memset(newstr, 0, len*sizeof(DCHAR)+1, len*sizeof(DCHAR)+1);
	adm_memcpy(newstr, (void*)str, len, (len)*sizeof(DCHAR)+1);

	return newstr;

}


