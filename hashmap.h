#pragma once

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "cslice.h"


typedef struct Variable
{
    CSlice* slice;
    uint64_t val;
} Variable;

Variable *makeVar(CSlice *s, uint64_t v);

typedef struct LinkedList
{
    Variable* var;
    struct LinkedList *next;
} LinkedList;

void addLinked(LinkedList *list[], Variable *v, int index);

typedef struct HashMap
{
    LinkedList *table[1000];
} HashMap;

HashMap *initMap();

void add(HashMap * map, CSlice * slice, Variable * var);

Variable *get(HashMap * map, CSlice * slice);

typedef struct Function
{
    char const* start;
    bool isStatement;
    int arg;
} Function;

Function *makeFunc(char const* s, bool is);