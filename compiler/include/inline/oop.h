#ifndef _INLINE_OOP_H
#define _INLINE_OOP_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef OOP_BASE_NAME
#define OOP_BASE_NAME OOPBase
#endif

#define NewObjectA(classPtr, classID, tagList) \
	LP3(0x1e, APTR, NewObjectA, struct IClass *, classPtr, a0, UBYTE *, classID, a1, struct TagItem *, tagList, a2, \
	, OOP_BASE_NAME)

#define GetID(stringID) \
	LP1(0x24, STRPTR, GetID, STRPTR, stringID, a0 \
	, OOP_BASE_NAME)

#define DoSuperMethodA(class, object, msg) \
	LP3(0x2a, IPTR, DoSuperMethodA, struct IClass *, classPtr, a0, Object *, object, a1, Msg, msg, a2, \
	, OOP_BASE_NAME)

/* More functions missing */

#endif /* _INLINE_OOP_H */
