#include <string>
#include <sstream>
#include "adm_string.h"

template <typename T>
static void adm_stringToConvert(const std::string& inString, T* outValue)
{
    std::string str = inString;
    std::stringstream ss;
    ss.clear();

    ss << str;
    ss >> *outValue;
    return;
}

D32S adm_hexStringToInt32(const DCHAR* inHexString, D32S* outValue)
{
    if (!inHexString) {
        return -1;
    }
    std::stringstream ss;
    ss.clear();

    ss << std::hex << inHexString;
    ss >> std::hex >> *outValue;
    return 0;
}

D32S adm_stringToInt32(const DCHAR* inString, D32S* outValue)
{
    if (!inString) {
        return -1;
    }
    adm_stringToConvert(inString, outValue);
    return 0;
}

D32S adm_stringToUint32(const DCHAR* inString, D32U* outValue)
{
    if (!inString) {
        return -1;
    }
    adm_stringToConvert(inString, outValue);
    return 0;
}

D32S adm_stringToInt64(const DCHAR* inString, D64S* outValue)
{
    if (!inString) {
        return -1;
    }
    adm_stringToConvert(inString, outValue);
    return 0;
}

D32S adm_stringToUint64(const DCHAR* inString, D64U* outValue)
{
    if (!inString) {
        return -1;
    }
    adm_stringToConvert(inString, outValue);
    return 0;
    return 0;
}

D32S adm_stringToFloat(const DCHAR* inString, DFLOAT* outValue)
{
    if (!inString) {
        return -1;
    }
    adm_stringToConvert(inString, outValue);
    return 0;
}

D32S adm_stringToDouble(const DCHAR* inString, DOUBLE* outValue)
{
    if (!inString) {
        return -1;
    }
    adm_stringToConvert(inString, outValue);
    return 0;
}