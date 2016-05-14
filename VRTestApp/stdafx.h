// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma warning(disable: 4820 4668 4917 4514)

//conversion from (LONG) (UINT) signed/unsigned mismatch
#pragma warning(disable: 4365)
//conversion from (int) (size_t) signed/unsigned mismatch
#pragma warning(disable: 4245)

//conversion from (unsigned int) to (unsigned short), possible loss of data
#pragma warning(disable: 4242)
//conversion from (unsigned int) to (unsigned short), possible loss of data
#pragma warning(disable: 4244)


//unhandled enum in switch
#pragma warning(disable: 4061)

//assignment operator could not be generated
#pragma warning(disable: 4512)

//unreferenced formal parameter
#pragma warning(disable: 4100)
//local variable is initialized but not referenced
#pragma warning(disable: 4189)

//behavior change...
#pragma warning(disable: 4350)

//function not inlined
#pragma warning(disable: 4710)



#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
