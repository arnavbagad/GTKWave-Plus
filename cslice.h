#pragma once

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct CSlice{
    char const* start;  // where does the string start in memory?
                       // The 2 uses of 'const' express different ideas:
                       //    * the first says that we can't change the
                       //      character we point to
                       //    * the second says that we can't change the
                       //      place we point to
    size_t len;        // How many characters in the string


} CSlice;

CSlice* makeSlice(char const* start, size_t len);
bool sliceEquals(CSlice s1, CSlice s2);
bool is_identifier(CSlice s);
void print(CSlice s);
// template <> struct std::hash<Slice>
// {
//     std::size_t operator()(const Slice &key) const
//     {
//         // djb2
//         size_t out = 5381;
//         for (size_t i = 0; i < key.len; i++)
//         {
//             char const c = key.start[i];
//             out = out * 33 + c;
//         }
//         return out;
//     }
// };