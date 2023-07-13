
/*[
 *      Name:                   adm_os_map.c
 *
 *      Project:                adm
]*/

/*! \internal
 * \file
 *          Provides a reference implementation of map handling API
 *          required by ADM.
 *
 * \brief   Map handling APIs
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_map.h"


typedef struct ADM_UTIL_Map_pair_t {
    struct ADM_UTIL_Map_pair_t *next;
    void *key;
    void *value;
} ADM_UTIL_Map_pair_t;

struct ADM_UTIL_Map_t {
    ADM_UTIL_Map_pair_t *head;
    D32U size;
    ADM_UTIL_Map_Compare_CB_t compareFunc;
    ADM_UTIL_Map_Free_CB_t freeFunc;
    DBOOL allowDuplicates;
    void *context;
};


static ADM_UTIL_Map_pair_t *findPair(ADM_UTIL_Map_t *inMap, const void *inKey)
{
    ADM_UTIL_Map_pair_t *p = NULL;

    if (!inMap)
        goto end;

    for (p = inMap->head; p; p = p->next) {
        if (!inMap->compareFunc(inKey, p->key, inMap->context))
            break;
    }

end:
    return p;
}


ADM_UTIL_Map_t *adm_map_create(ADM_UTIL_Map_Compare_CB_t inCompareFunc,
                                  ADM_UTIL_Map_Free_CB_t inFreeFunc, void* inFuncContext,
                                  DBOOL inAllowDuplicates)
{
    ADM_UTIL_Map_t *map = (ADM_UTIL_Map_t* )adm_malloc(sizeof(ADM_UTIL_Map_t));

    if (map) {
        map->compareFunc = inCompareFunc;
        map->freeFunc = inFreeFunc;
        map->context = inFuncContext;
        map->allowDuplicates = inAllowDuplicates;
    }

    return map;
}

void adm_map_removeAll(ADM_UTIL_Map_t *inMap)
{
    if (!inMap)
        return;

    while (inMap->head) {
        ADM_UTIL_Map_pair_t *p = inMap->head;

        inMap->head = inMap->head->next;
        if (inMap->freeFunc)
            inMap->freeFunc(p->key, p->value, inMap->context);

        adm_free(p);
    }
    inMap->size = 0;
}

void adm_map_destroy(ADM_UTIL_Map_t **ioMap)
{
    if (!ioMap || !*ioMap)
        return;

    adm_map_removeAll(*ioMap);
    adm_free(*ioMap);
}

DBOOL adm_map_add(ADM_UTIL_Map_t *inMap, void *inKey,  void *inValue)
{
   
    ADM_UTIL_Map_pair_t *p, **tail = NULL;

    if (!inMap) {
       return FALSE;
    }

    if (!inMap->allowDuplicates && adm_map_find(inMap, inKey, NULL)) {
       
        return FALSE;
    }

    p = adm_malloc(sizeof(ADM_UTIL_Map_pair_t));
    if (!p) {
        return FALSE;
    }

    /* Assign values */
    p->key = inKey;
    p->value = inValue;

    /* Run to end of list and append */
    for (tail = &inMap->head; *tail; tail = &(*tail)->next);
    *tail = p;
    inMap->size++;


    return TRUE;
}

DBOOL adm_map_getNext(ADM_UTIL_Map_t *inMap, void **ioIter,
                             void **outKey, void **outValue)
{
   
    ADM_UTIL_Map_pair_t **p = (ADM_UTIL_Map_pair_t **)ioIter;

    *outKey = *outValue = NULL;

    if (!inMap) {
        return FALSE;
    }

    if (!*p)
        *p = inMap->head;

    if (*p) {
        *outKey = (*p)->key;
        *outValue = (*p)->value;
        *p = (*p)->next;
    } else
        return FALSE;


    return TRUE;
}

void adm_map_removeItem(ADM_UTIL_Map_t *inMap, void *inKey)
{
    ADM_UTIL_Map_pair_t **p;

    if (!inMap)
        return;

    for (p = &inMap->head; *p;) {
        if (!inMap->compareFunc(inKey, (*p)->key, inMap->context)) {
            ADM_UTIL_Map_pair_t *tmp = *p;

            *p = (*p)->next;
            if (inMap->freeFunc)
                inMap->freeFunc(tmp->key, tmp->value, inMap->context);
            adm_free(tmp);
            inMap->size--;
        } else
            p = &(*p)->next;
    }
}

void adm_map_setNull(ADM_UTIL_Map_t *inMap, void *inKey)
{
    ADM_UTIL_Map_pair_t **p;

    if (!inMap)
        return;

    for (p = &inMap->head; *p;) {
        if (!inMap->compareFunc(inKey, (*p)->key, inMap->context)) {
            ADM_UTIL_Map_pair_t *tmp = *p;

            *p = (*p)->next;
            if (inMap->freeFunc)
                inMap->freeFunc(tmp->key, tmp->value, inMap->context);
            
            inMap->size--;
        } else
            p = &(*p)->next;
    }
}

D32U adm_map_getSize(ADM_UTIL_Map_t *inMap)
{
    return inMap ? inMap->size : 0;
}

DBOOL adm_map_find(ADM_UTIL_Map_t *inMap, const void *inKey, void **outValue)
{
    ADM_UTIL_Map_pair_t *p = findPair(inMap, inKey);
    DBOOL isFound = FALSE;
    void *val = NULL;

    if (p) {
        val = p->value;
        isFound = TRUE;
    }
    if (outValue)
        *outValue = val;

    return isFound;
}

DBOOL adm_map_set(ADM_UTIL_Map_t *inMap, void *inKey, void *inValue)
{
    ADM_UTIL_Map_pair_t *p = findPair(inMap, inKey);
    DBOOL isFound = FALSE;

    if (p) {
		p->key = inKey;
		p->value = inValue;
		
        isFound = TRUE;
    }

    return isFound;
}

void adm_map_freeItem(void *inKey, void *inValue, void *inContext)
{

    adm_free(inKey);
    adm_free(inValue);
}

