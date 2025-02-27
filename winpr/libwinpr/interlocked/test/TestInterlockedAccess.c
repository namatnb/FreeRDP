#include <stdio.h>
#include <winpr/crt.h>
#include <winpr/windows.h>
#include <winpr/interlocked.h>

int TestInterlockedAccess(int argc, char* argv[])
{
	int index = 0;
	LONG* Addend = NULL;
	LONG* Target = NULL;
	LONG oldValue = 0;
	LONG* Destination = NULL;
	LONGLONG oldValue64 = 0;
	LONGLONG* Destination64 = NULL;
	WINPR_UNUSED(argc);
	WINPR_UNUSED(argv);
	/* InterlockedIncrement */

	Addend = winpr_aligned_malloc(sizeof(LONG), sizeof(LONG));
	if (!Addend)
	{
		printf("Failed to allocate memory\n");
		return -1;
	}

	*Addend = 0;

	for (index = 0; index < 10; index++)
		InterlockedIncrement(Addend);

	if (*Addend != 10)
	{
		printf("InterlockedIncrement failure: Actual: %" PRId32 ", Expected: 10\n", *Addend);
		return -1;
	}

	/* InterlockedDecrement */

	for (index = 0; index < 10; index++)
		InterlockedDecrement(Addend);

	if (*Addend != 0)
	{
		printf("InterlockedDecrement failure: Actual: %" PRId32 ", Expected: 0\n", *Addend);
		return -1;
	}

	/* InterlockedExchange */

	Target = winpr_aligned_malloc(sizeof(LONG), sizeof(LONG));

	if (!Target)
	{
		printf("Failed to allocate memory\n");
		return -1;
	}

	*Target = 0xAA;

	oldValue = InterlockedExchange(Target, 0xFF);

	if (oldValue != 0xAA)
	{
		printf("InterlockedExchange failure: Actual: 0x%08" PRIX32 ", Expected: 0xAA\n", oldValue);
		return -1;
	}

	if (*Target != 0xFF)
	{
		printf("InterlockedExchange failure: Actual: 0x%08" PRIX32 ", Expected: 0xFF\n", *Target);
		return -1;
	}

	/* InterlockedExchangeAdd */

	*Addend = 25;

	oldValue = InterlockedExchangeAdd(Addend, 100);

	if (oldValue != 25)
	{
		printf("InterlockedExchangeAdd failure: Actual: %" PRId32 ", Expected: 25\n", oldValue);
		return -1;
	}

	if (*Addend != 125)
	{
		printf("InterlockedExchangeAdd failure: Actual: %" PRId32 ", Expected: 125\n", *Addend);
		return -1;
	}

	/* InterlockedCompareExchange (*Destination == Comparand) */

	Destination = winpr_aligned_malloc(sizeof(LONG), sizeof(LONG));
	if (!Destination)
	{
		printf("Failed to allocate memory\n");
		return -1;
	}

	*Destination = (LONG)0xAABBCCDDL;

	oldValue = InterlockedCompareExchange(Destination, (LONG)0xCCDDEEFFL, (LONG)0xAABBCCDDL);

	if (oldValue != (LONG)0xAABBCCDDL)
	{
		printf("InterlockedCompareExchange failure: Actual: 0x%08" PRIX32
		       ", Expected: 0xAABBCCDD\n",
		       oldValue);
		return -1;
	}

	if ((*Destination) != (LONG)0xCCDDEEFFL)
	{
		printf("InterlockedCompareExchange failure: Actual: 0x%08" PRIX32
		       ", Expected: 0xCCDDEEFF\n",
		       *Destination);
		return -1;
	}

	/* InterlockedCompareExchange (*Destination != Comparand) */

	*Destination = (LONG)0xAABBCCDDL;

	oldValue = InterlockedCompareExchange(Destination, -857870593L, 0x66778899L);

	if (oldValue != (LONG)0xAABBCCDDL)
	{
		printf("InterlockedCompareExchange failure: Actual: 0x%08" PRIX32
		       ", Expected: 0xAABBCCDD\n",
		       oldValue);
		return -1;
	}

	if ((*Destination) != (LONG)0xAABBCCDDL)
	{
		printf("InterlockedCompareExchange failure: Actual: 0x%08" PRIX32
		       ", Expected: 0xAABBCCDD\n",
		       *Destination);
		return -1;
	}

	/* InterlockedCompareExchange64 (*Destination == Comparand) */

	Destination64 = winpr_aligned_malloc(sizeof(LONGLONG), sizeof(LONGLONG));
	if (!Destination64)
	{
		printf("Failed to allocate memory\n");
		return -1;
	}

	*Destination64 = 0x66778899AABBCCDD;

	oldValue64 =
	    InterlockedCompareExchange64(Destination64, 0x8899AABBCCDDEEFF, 0x66778899AABBCCDD);

	if (oldValue64 != 0x66778899AABBCCDD)
	{
		printf("InterlockedCompareExchange failure: Actual: 0x%016" PRIX64
		       ", Expected: 0x66778899AABBCCDD\n",
		       oldValue64);
		return -1;
	}

	if ((*Destination64) != (LONGLONG)0x8899AABBCCDDEEFFLL)
	{
		printf("InterlockedCompareExchange failure: Actual: 0x%016" PRIX64
		       ", Expected: 0x8899AABBCCDDEEFF\n",
		       *Destination64);
		return -1;
	}

	/* InterlockedCompareExchange64 (*Destination != Comparand) */

	*Destination64 = 0x66778899AABBCCDDLL;

	oldValue64 = InterlockedCompareExchange64(Destination64, 0x8899AABBCCDDEEFFLL, 12345);

	if (oldValue64 != 0x66778899AABBCCDDLL)
	{
		printf("InterlockedCompareExchange failure: Actual: 0x%016" PRIX64
		       ", Expected: 0x66778899AABBCCDD\n",
		       oldValue64);
		return -1;
	}

	if (*Destination64 != 0x66778899AABBCCDDLL)
	{
		printf("InterlockedCompareExchange failure: Actual: 0x%016" PRIX64
		       ", Expected: 0x66778899AABBCCDD\n",
		       *Destination64);
		return -1;
	}

	winpr_aligned_free(Addend);
	winpr_aligned_free(Target);
	winpr_aligned_free(Destination);
	winpr_aligned_free(Destination64);

	return 0;
}
