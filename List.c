#include "List.h"

DWORD AddToBack(CHAR_LIST** pHead, CHAR* psBuf, DWORD dwLength) {
    CHAR_LIST*  newNode = NULL;
    CHAR_LIST*  p       = NULL;
    DWORD       dwCount = 0;
    DWORD       i       = 0;

    p = *pHead;

    if (p == NULL) {
        newNode         = (CHAR_LIST*) malloc(sizeof(CHAR_LIST));
        newNode->c      = psBuf[i++];
        newNode->next   = NULL;
        p               = newNode;
        *pHead			= newNode;
    } else {

        while (p->next != NULL) {
            p = p->next;
            dwCount++;
        }
    }
	dwCount++;
        
    for (i; i < dwLength; i++) {
        newNode         = (CHAR_LIST*) malloc(sizeof(CHAR_LIST));
        newNode->c      = psBuf[i];
        newNode->next   = NULL;
        p->next         = newNode;
        p               = p->next;
        dwCount++;
    }

    return dwCount;
}


DWORD GetFromList(CHAR_LIST* p, UINT ordinal) {
    UINT i = 0;

    for (i = 1; i < ordinal; i++) {
        p = p->next;
    }
    return p->c;
}


CHAR* RemoveFromFront(CHAR_LIST** pHead, DWORD dwLength) {
    CHAR_LIST*  p       = NULL;
    CHAR_LIST*  tracer  = NULL;
    UINT        i       = 0;
    CHAR*       removed = NULL;
    
    p       = *pHead;
    removed = (CHAR*) malloc(sizeof(CHAR) * dwLength);

    for (i = 0; i < dwLength; i++) {
        removed[i]  = p->c;
        tracer      = p;
        p           = p->next;
        free(tracer);
    }
    *pHead = p;
    return removed;
}