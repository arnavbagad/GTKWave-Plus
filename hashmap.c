#include "hashmap.h"

size_t hash(CSlice slice)
{
    size_t out = 5381;
    for (size_t i = 0; i < slice.len; i++)
    {
        char const c = slice.start[i];
        out = out * 33 + c;
    }
    return out % 1000;
}

void addLinked(LinkedList* list[], Variable* v, int index){
    if(list[index] == NULL){
        list[index] = malloc(sizeof(LinkedList));
        list[index]->var = v;
        list[index]->next = NULL;
        return;
    }
    LinkedList *temp = list[index];
    while(temp -> next != NULL){
        if(sliceEquals(*(temp->var->slice), *v -> slice)){
            free(temp -> var);
            temp -> var = v;
            
            return;
        }
        temp = temp -> next;
    }
    if (sliceEquals(*(temp->var->slice), *v->slice))
    {
        free(temp->var);
        temp->var = v;

        return;
    }
    LinkedList *newNode = malloc(sizeof(LinkedList));
    newNode->var = v;
    newNode->next = NULL;
    temp->next = newNode;
}
HashMap* initMap(){
    HashMap *map = malloc(sizeof(HashMap));
    for(int i = 0; i < 1000;i++){
        LinkedList* l = NULL;
        map -> table[i] = l;
    }

    return map;
}
Variable *makeVar(CSlice* s, uint64_t v)
{
    Variable *var = malloc(sizeof(Variable));
    var->slice = s;
    var->val = v;
    return var;
}

void add(HashMap* map, CSlice* slice, Variable* var){
    size_t index = hash(*slice);
    addLinked(map -> table, var, index);
}

Variable* get(HashMap* map, CSlice* slice){
    size_t index = hash(*slice);
    LinkedList* list = map -> table[index];
    LinkedList* temp = list;
    while(temp != NULL){
        if(sliceEquals(*(temp->var->slice), *slice)){
            return (temp->var);
        }
        temp = temp -> next;
    }
    return NULL;
}

Function *makeFunc(char const *s, bool is)
{
    Function *f = malloc(sizeof(Function));
    f->start = s;
    f->isStatement = is;
    f -> arg = 0;
    return f;
}
