// libc includes (available in both C and C++)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// Implementation includes
#include "cslice.h"
#include "hashmap.h"


typedef struct Interpreter{
    char const * program;
    char const * current;
    HashMap* map;
} Interpreter;

Interpreter* inter;

Interpreter* create_interpeter(char const * program){
    Interpreter* i = malloc(sizeof(Interpreter));
    HashMap* map = initMap();
    i -> program = program;
    i -> current = program;
    i -> map = map;
    return i;
}
// func defintions
extern uint64_t expression(bool effects);
extern void statements(bool effects);
extern bool statement(bool effects);

// fun function helpers
Function *functions[1000];
int fnctn_count = 1;
uint64_t last_return = 0;
bool boolreturn = false;
uint64_t *curr_arg = NULL;

void fail()
{
    printf("failed at offset %ld\n", (size_t)(inter->current - inter->program));
    printf("%s\n", inter->current);
    exit(1);
}

void end_or_fail()
{
    while (isspace(*inter->current))
    {
        inter->current += 1;
    }
    if (*inter->current != 0)
        fail(inter);
}

void skip()
{
    while (isspace(*inter->current))
    {
        inter->current += 1;
    }
}

bool consume(const char *str)
{
    skip(inter);

    size_t i = 0;
    while (true)
    {
        char const expected = str[i];
        char const found = inter->current[i];
        if (expected == 0)
        {
            /* survived to the end of the expected string */
            inter->current += i;
            return true;
        }
        if (expected != found)
        {
            
            return false;
        }
        // assertion: found != 0
        i += 1;
    }
}
bool check(const char *str)
{
    skip(inter);

    size_t i = 0;
    while (true)
    {
        char const expected = str[i];
        char const found = inter->current[i];
        if (expected == 0)
        {
            /* survived to the end of the expected string */
            //inter->current += i; DO NOT UPDATE POSITION, JUST A CHECK
            return true;
        }
        if (expected != found)
        {

            return false;
        }
        // assertion: found != 0
        i += 1;
    }
}

void consume_or_fail(const char *str)
{
    if (!consume(str))
    {
        fail(inter);
    }
}

CSlice* consume_identifier()
{
    skip(inter);

    if (isalpha(*inter->current))
    {
        char const *start = inter->current;
        do
        {
            inter->current += 1;
        } while (isalnum(*inter->current));
        CSlice *slice = makeSlice(start, (size_t)(inter->current - start));
        return slice;
    }
    return NULL;
    
}

uint64_t* consume_literal()
{
    skip(inter);

    if (isdigit(*inter->current))
    {
        uint64_t *v = malloc(sizeof(uint64_t));
        do
        {
            *v = 10 * *v + ((*inter->current) - '0');
            inter->current += 1;
        } while (isdigit(*inter->current));
        return v;
    }
    else
    {
        return NULL;
    }
}

// The plan is to honor as many C operators as possible with
// the same precedence and associativity
// e<n> implements operators with precedence 'n' (smaller is higher)

// () [] . -> ...
uint64_t e1(bool effects)
{
    // should not consume "fun"
    if(check("fun")){
        return 0;
    }
    if(curr_arg != NULL && consume("it")){ // handles case when "it" is used as a function argument
        return *curr_arg;
    }
    CSlice* id = consume_identifier();

    if (id != NULL && id->start != 0)
    {
        Variable *v = get(inter->map, id);
        free(id);
        if(v == NULL){
            return 0;
        }
        else{
            return v -> val;
        }
    }
    else{
        uint64_t* v = consume_literal();
        if(v != NULL)
        {
            uint64_t out = *v;
            free(v);
            return out;
        }
        else {
            if (consume("("))
            {
                uint64_t v = expression(effects);
                consume(")");
                return v;
            }
            else
            {
                fail(inter);
                return 0;
            }
        }
    }
}

uint64_t e1point5(bool effects)
{
    uint64_t v = e1(effects);
    while(true){
        // ignore fun used as variable
        if(check("fun =")){
            return v;
        }
        if(consume("fun")){
            //add function to function list
            if(consume("{")){
                if(effects){
                    functions[fnctn_count] = makeFunc(inter->current, true);
                }
                statements(false);
            } else{
                if(effects){
                    functions[fnctn_count] = makeFunc(inter->current, false);
                }
                statement(false); 
            }
            if(effects){
                fnctn_count++;
                v = fnctn_count - 1;
                //wrap around if overflowed function
                if (fnctn_count >= 1000)
                {
                    fnctn_count = 1;
                }
            }
        }
        // function call
        else if(consume("(")){
            // preserve old argument for recursive function case
            uint64_t* old_arg = curr_arg;
            uint64_t e = expression(effects);
            curr_arg = &e;
            boolreturn = false;
            consume(")");
            if(effects){
                last_return = 0;
                char const* start = functions[v] -> start;
                char const* resume = inter -> current;
                if(functions[v] -> isStatement){
                    inter -> current = start;
                    statements(effects);
                } else{
                    inter -> current = start;
                    statement(effects);
                }
                inter -> current = resume;
                v = last_return;
            }
            boolreturn = false;
            curr_arg = old_arg;
        }
        else{
            return v;
        }
    }
}

// ++ -- unary+ unary- ... (Right)
uint64_t e2(bool effects)
{
    return e1point5(effects);
}

// * / % (Left)
uint64_t e3(bool effects)
{
    uint64_t v = e2(effects);

    while (true)
    {
        if (consume("*"))
        {
            v = v * e2(effects);
        }
        else if (consume("/"))
        {
            uint64_t right = e2(effects);
            v = (right == 0) ? 0 : v / right;
        }
        else if (consume("%"))
        {
            uint64_t right = e2(effects);
            v = (right == 0) ? 0 : v % right;

        }
        else
        {
            return v;
        }
    }
}

// (Left) + -
uint64_t e4(bool effects)
{
    uint64_t v = e3(effects);

    while (true)
    {
        if (consume("+"))
        {
            v = v + e3(effects);
        }
        else if (consume("-"))
        {
            v = v - e3(effects);
        }
        else
        {
            return v;
        }
    }
}

// << >>
uint64_t e5(bool effects)
{
    return e4(effects);
}

// < <= > >=
uint64_t e6(bool effects)
{
    uint64_t v = e5(effects);

    while (true)
    {
        if (consume("<="))
        {
            if (v <= e5(effects))
            {
                v = 1;
            }
            else
            {
                v = 0;
            }
        }
        else if (consume(">="))
        {
            if (v >= e5(effects))
            {
                v = 1;
            }
            else
            {
                v = 0;
            }
        }
        else if (consume(">"))
        {
            if (v > e5(effects))
            {
                v = 1;
            }
            else
            {
                v = 0;
            }
        }
        else if (consume("<"))
        {
            if (v < e5(effects))
            {
                v = 1;
            }
            else
            {
                v = 0;
            }
        }
        else
        {
            return v;
        }
    }
    
    return v;
    
    
}

// == !=
uint64_t e7(bool effects)
{
    uint64_t v = e6(effects);
    while(true){
        if (consume("=="))
        {
            if (v == e6(effects)) {
                v = 1;
            } else
            {
                v = 0;
            }
        }
        else if (consume("!="))
        {
            if (v != e6(effects)) {
                v = 1;
            } else {
                v = 0;
            }
        }
        else{
            return v;
        }
    }
}

// (left) &
uint64_t e8(bool effects)
{
    uint64_t v = e7(effects);

    while (true)
    {
        // check to avoid confusion with &&
        if (!check("&&"))
        {
            if (consume("&")){
                v = v & e7(effects);
            }
            else{
                return v;
            }
        } else {
            return v;
        }
    }
    
}

// ^
uint64_t e9(bool effects)
{
    return e8(effects);
}

// |
uint64_t e10(bool effects)
{
    return e9(effects);
}

// &&
uint64_t e11(bool effects)
{
    uint64_t v = e10(effects);
    while (true)
    {
        if (consume("&&"))
        {
            if(!v){ // v is not true, will short circuit
                e10(false);
            }
            else if(!e10(effects)){ // must check other side
                v = 0;
            } else{ // both sides are true
                v = 1;
            }
        }
        else
        {
            return v;
        }
    }
}

// ||
uint64_t e12(bool effects)
{
    uint64_t v = e11(effects);

    while (true)
    {
        if (consume("||"))
        {
            if(v){ // v is true, will short circuit
                e11(false);
                v = 1;
            }
            else if(e11(effects)){ // must check other side
                v = 1;
            } 
            else{ // both aren't true, return false
                v = 0;
            }
        }
        else
        {
            return v;
        }
    }
}

// (right with special treatment for middle expression) ?:
uint64_t e13(bool effects)
{
    return e12(effects);
}

// = += -= ...
uint64_t e14(bool effects)
{
    uint64_t v = e13(effects);
    while(consume(",")){
        v = e13(effects);
    }
    return v;
}

// ,
uint64_t e15(bool effects)
{
    return e14(effects);
}

uint64_t expression(bool effects)
{
    return e15(effects);
}

bool statement(bool effects)
{
    if(consume("}")){ // breaks out of recursive statement calls (from if, while, functinos)
        return false;
    } else if(consume("return")){ // similar to "}", but for return statements
        if(effects){
            boolreturn = true; //will prevent further statements in function from executing
            last_return = expression(effects); // stores return value in return variable
        } else{
            expression(effects);
        }
        return !effects;
    }
    else if(consume("if")){
        uint64_t v = expression(effects);
        bool ifparen = consume("{");
        if(effects){
            if(v){ // if statement is true, execute if statements
                if(ifparen){
                    statements(true);
                }
                else{
                    if (!statement(true)) { // handles case when returning inside non-bracketed if statement
                        return false;
                    }
                }
                if (consume("else")) { // checks for else, doesn't execute code in here
                    if (consume("{")) {
                        statements(false);
                    } else{
                        statement(false);
                    }
                }
            } else{ // if statement is false, don't execute if statements
                if (ifparen) {
                    statements(false);
                }
                else {
                    statement(false);
                }

                if (consume("else")) { // checks for else, executes code in here
                    if (consume("{")) {
                        statements(true);
                    } else {
                        if (!statement(true)) { // handles case when returning inside non-bracketed if statement
                            return false;
                        }
                    }
                }
            }
        } else{ // doesn't execute code in both if and else statements, just consumes
            if(ifparen){
                statements(false);
            } else{
                statement(false);
            }
            if (consume("else")) {
                if (consume("{")) {
                    statements(false);
                } else {
                    statement(false);
                }
            }
        }
        return true;
    }
    else if (consume("while"))
    {
        char const *start = inter->current; // stores while start
        uint64_t v = expression(effects); // stores condition
        bool ifparen = consume("{");

        while (v && effects)
        {
            if(ifparen){
                statements(true);
            }
            else{
                statement(true);
            }
            if (boolreturn) // checks if function return occurs in while, will break if it does
            {
                return false;
            }
            inter->current = start; // goes back to start and checks condition again
            v = expression(effects);
            consume("{");
        }
        // goes through while code, doesn't excute
        if(ifparen){
            statements(false);
        }
        else{
            statement(false);
        }
        return true;
    }

    else if (consume("print"))
    {
        // print ...
        int64_t v = expression(effects);
        if (effects && !boolreturn) // doesn't run print statements after return (boolreturn is similar to effects)
        {
            printf("%ld\n", v);
        }
        return true;
    }
    else {
        if(curr_arg != NULL && consume("it")){ // handles case when "it" is used as a function argument
            if (consume("="))
            {
                uint64_t v = expression(effects);
                if (effects)
                {
                    *curr_arg = v;

                }
                return true;
            }
            else
            {
                fail();
            }
        } else{
            CSlice* id = consume_identifier(inter);
            
            if (id != NULL)
            {
                // x = ...
                if (consume("="))
                {
                    Variable* v = makeVar(id, expression(effects));
                    if (effects)
                    {
                        add(inter->map, id, v);
                    }
 
                    return true;
                }
                else
                {
                    fail();
                }
            }
        }
        return false;
    }
}

void statements(bool effects)
{
    while (statement(effects));


}

void run()
{
    statements(true);
    end_or_fail();
}

int main(int argc, const char *const *const argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <file name>\n", argv[0]);
        exit(1);
    }

    // open the file
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    // determine its size (std::filesystem::get_size?)
    struct stat file_stats;
    int rc = fstat(fd, &file_stats);
    if (rc != 0)
    {
        perror("fstat");
        exit(1);
    }

    // map the file in my address space
    char const *prog = (char const *)mmap(
        0,
        file_stats.st_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0);
    if (prog == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    inter = create_interpeter(prog);

    run();

    return 0;
}