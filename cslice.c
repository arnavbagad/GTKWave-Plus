#include "cslice.h"

CSlice *makeSlice(char const* s, size_t l)
{
    CSlice *slice = malloc(sizeof(CSlice));
    slice -> start = s;
    slice -> len = l;
    return slice;
}

bool sliceEquals(CSlice s1, CSlice s2)
{
    if (s1.len != s2.len)
        return false;
    for (size_t i = 0; i < s1.len; i++)
    {
        if (s2.start[i] != s1.start[i])
            return false;
    }
    return true;
}
bool is_identifier(CSlice s)
{
    if (s.len == 0)
        return 0;
    if (!isalpha(s.start[0]))
        return false;
    for (size_t i = 1; i < s.len; i++)
        if (!isalnum(s.start[i]))
            return false;
    return true;
}

void print(CSlice s)
{
    for (size_t i = 0; i < s.len; i++)
    {
        printf("%c", s.start[i]);
    }
}