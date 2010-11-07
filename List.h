#ifndef LIST_H
#define LIST_H

#include <Windows.h>

typedef struct CHAR_LIST CHAR_LIST;
typedef struct CHAR_LIST {
    CHAR        c;
    CHAR_LIST*  next;
} CHAR_LIST;

DWORD AddToBack(CHAR_LIST** p, CHAR* psBuf, DWORD dwLength);
DWORD GetFromList(CHAR_LIST* p, UINT ordinal);
CHAR* RemoveFromFront(CHAR_LIST** p, DWORD dwLength);

#endif
