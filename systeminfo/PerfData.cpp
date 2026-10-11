#include "stdafx.h"

#define TOTALBYTES    8192
#define BYTEINCREMENT 1024

PPERF_OBJECT_TYPE FirstObject(PPERF_DATA_BLOCK PerfData)
{
    return( (PPERF_OBJECT_TYPE)((PBYTE)PerfData + PerfData->HeaderLength) );
}

PPERF_OBJECT_TYPE NextObject(PPERF_OBJECT_TYPE PerfObj)
{
    return( (PPERF_OBJECT_TYPE)((PBYTE)PerfObj + PerfObj->TotalByteLength) );
}

PPERF_INSTANCE_DEFINITION FirstInstance(PPERF_OBJECT_TYPE PerfObj)
{
    return( (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfObj + PerfObj->DefinitionLength) );
}

PPERF_INSTANCE_DEFINITION NextInstance(PPERF_INSTANCE_DEFINITION PerfInst)
{
    PPERF_COUNTER_BLOCK PerfCntrBlk;
    PerfCntrBlk = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst + PerfInst->ByteLength);
    return( (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfCntrBlk + PerfCntrBlk->ByteLength) );
}

PPERF_COUNTER_DEFINITION FirstCounter(PPERF_OBJECT_TYPE PerfObj)
{
    return( (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj + PerfObj->HeaderLength) );
}

PPERF_COUNTER_DEFINITION NextCounter(PPERF_COUNTER_DEFINITION PerfCntr)
{
    return( (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr + PerfCntr->ByteLength) );
}

LONG GetUpTimeSecs()
{
    PPERF_DATA_BLOCK PerfData = NULL;
    PPERF_OBJECT_TYPE PerfObj;
    PPERF_COUNTER_DEFINITION PerfCntr;
    PPERF_COUNTER_BLOCK PtrToCntr;
    DWORD BufferSize = TOTALBYTES;
    DWORD j;
	LONGLONG result = -1;

	// Allocate the buffer for the performance data.
    PerfData = (PPERF_DATA_BLOCK) malloc( BufferSize );

    while( RegQueryValueEx( HKEY_PERFORMANCE_DATA, "2", NULL, NULL,
                               (LPBYTE) PerfData, &BufferSize ) == ERROR_MORE_DATA )
    {
	    // Get a buffer that is big enough.
        BufferSize += BYTEINCREMENT;
        PerfData = (PPERF_DATA_BLOCK) realloc( PerfData, BufferSize );
    }

	// Get the first object type.
    PerfObj = FirstObject(PerfData);

	// Get the first counter.
    PerfCntr = FirstCounter(PerfObj);

	// Get the counter block.
    PtrToCntr = (PPERF_COUNTER_BLOCK) ((PBYTE)PerfObj + PerfObj->DefinitionLength);

	// Go through counter until we find the System Up Time counter
    for( j=0; j < PerfObj->NumCounters; j++ )
    {
		// Find index 674, which is the System Up Time
		if (PerfCntr->CounterNameTitleIndex == 674)
		{
			LONGLONG x,y,tb;
			LPVOID pb = (LPVOID)((PBYTE)PtrToCntr + PerfCntr->CounterOffset);
			x = *(LONGLONG*)pb;
			y = (LONGLONG)PerfObj->PerfTime.QuadPart;
			tb = (LONGLONG)PerfObj->PerfFreq.QuadPart;
			result = (y - x)/tb;
			break;
		}

		// Get the next counter
        PerfCntr = NextCounter(PerfCntr);
    }

	free(PerfData);

	// Weren't able to find the correct index for System Up Time
	return (LONG)result;
}
