
/*
 *  CUnit - A Unit testing framework library for C.
 *  Copyright (C) 2001  Anil Kumar
 *  
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stringbuf_test.h"
#include "../lib/libcitadel.h"

static int success_init(void) { return 0; }
static int success_clean(void) { return 0; }

static void testSuccess1(void) { CU_ASSERT(1); }
static void testSuccess2(void) { CU_ASSERT(1); }
static void testSuccess3(void) { CU_ASSERT(1); }

static int group_failure_init(void) { return 1;}
static int group_failure_clean(void) { return 1; }

static void testGroupFailure1(void) { CU_ASSERT(0); }
static void testGroupFailure2(void) { CU_ASSERT(2); }

static void testfailure1(void) { CU_ASSERT(12 <= 10); }
static void testfailure2(void) { CU_ASSERT(2); }
static void testfailure3(void) { CU_ASSERT(3); }
/*
static void test1(void)
{
	CU_ASSERT((char *)2 != "THis is positive test.");
	CU_ASSERT((char *)2 == "THis is negative test. test 1");
}

static void test2(void)
{
	CU_ASSERT((char *)2 != "THis is positive test.");
	CU_ASSERT((char *)3 == "THis is negative test. test 2");
}
*/


static void TestRevalidateStrBuf(StrBuf *Buf)
{
	CU_ASSERT(strlen(ChrPtr(Buf)) == StrLength(Buf));



}


static void TestCreateBuf(void)
{
	StrBuf *Buf;
	StrBuf *Buf2;
	long len;
	long i;

	Buf = NewStrBuf();
	CU_ASSERT(Buf != NULL);
	FreeStrBuf(&Buf);

	CU_ASSERT(Buf == NULL);
	Buf = NewStrBufPlain(HKEY("ABC"));
	TestRevalidateStrBuf(Buf);
	CU_ASSERT(StrLength(Buf) == 3);
	CU_ASSERT_NSTRING_EQUAL("ABC", ChrPtr(Buf), 3);

	len = StrLength(Buf);
	for (i=0; i< 500; i ++)
	{
		StrBufAppendBufPlain(Buf, HKEY("ABC"), 0);
		len += 3;
		CU_ASSERT(StrLength(Buf) == len);
	}	
	StrBufShrinkToFit(Buf, 1);
	FreeStrBuf(&Buf);
	CU_ASSERT(Buf == NULL);
	
	Buf = NewStrBufPlain(HKEY("ABC"));
	TestRevalidateStrBuf(Buf);
	len = StrLength(Buf);
	for (i=0; i< 500; i ++)
	{
		StrBufAppendPrintf(Buf, "%s", "ABC");
		len += 3;
		CU_ASSERT(StrLength(Buf) == len);		
	}
	TestRevalidateStrBuf(Buf);
	StrBufShrinkToFit(Buf, 1);
	TestRevalidateStrBuf(Buf);

	Buf2 = NewStrBufDup(Buf);
	CU_ASSERT(StrLength(Buf) == StrLength(Buf2));		
	
	CU_ASSERT_NSTRING_EQUAL(ChrPtr(Buf2), ChrPtr(Buf), StrLength(Buf2));
	
	CU_ASSERT(StrBufIsNumber(Buf) == 0);

	FlushStrBuf(Buf2);
	CU_ASSERT(StrLength(Buf2) == 0);

	FLUSHStrBuf(Buf);
	CU_ASSERT(StrLength(Buf) == 0);

	FreeStrBuf(&Buf);
	FreeStrBuf(&Buf2);
	CU_ASSERT(Buf == NULL);
	CU_ASSERT(Buf2 == NULL);


	Buf = NewStrBufPlain(HKEY("123456"));
///	CU_ASSERT(StrBufIsNumber(Buf) == 1); Todo: this is buggy.
	FreeStrBuf(&Buf);
	
}

static void NextTokenizerIterateBuf(StrBuf *Buf, int NTokens)
{
	const char *pCh = NULL;
	StrBuf *Buf2;
	long CountTokens = 0;
	long HaveNextToken = 0;
	long HaveNextTokenF = 0;

	printf("\n\nTemplate: >%s<\n", ChrPtr(Buf));
	TestRevalidateStrBuf(Buf);
			     
	Buf2 = NewStrBuf();
	while (HaveNextToken = StrBufHaveNextToken(Buf, &pCh),
	       HaveNextTokenF = StrBufExtract_NextToken(Buf2, Buf, &pCh, ','),
	       (HaveNextTokenF>= 0))
	{
		CountTokens++;
		
		printf("Token: >%s< >%s< %ld:%ld\n", 
		       ChrPtr(Buf2), 
		       ((pCh != NULL) && (pCh != StrBufNOTNULL))? pCh : "N/A", 
		       HaveNextToken, 
		       HaveNextTokenF);
		TestRevalidateStrBuf(Buf2);

		CU_ASSERT(HaveNextToken == (HaveNextTokenF >= 0));
		
		CU_ASSERT(CountTokens <= NTokens);
	} 
	CU_ASSERT(HaveNextToken == (HaveNextTokenF >= 0));
}

static void TestNextTokenizer_EndWithEmpty(void)
{
	StrBuf *Buf;

	Buf = NewStrBufPlain(HKEY("abc,abc, 1, ,,"));
	NextTokenizerIterateBuf(Buf, 7);
	FreeStrBuf(&Buf);
}

static void TestNextTokenizer_StartWithEmpty(void)
{
	StrBuf *Buf;

	Buf = NewStrBufPlain(HKEY(",cde,abc, 1, ,,bbb"));
	NextTokenizerIterateBuf(Buf, 8);
	FreeStrBuf(&Buf);
}

static void TestNextTokenizer_Empty(void)
{
	StrBuf *Buf;

	Buf = NewStrBufPlain(HKEY(""));
	NextTokenizerIterateBuf(Buf, 8);
	FreeStrBuf(&Buf);
}

static void TestNextTokenizer_TwoEmpty(void)
{
	StrBuf *Buf;

	Buf = NewStrBufPlain(HKEY(","));
	NextTokenizerIterateBuf(Buf, 8);
	FreeStrBuf(&Buf);
}

static void TestNextTokenizer_One(void)
{
	StrBuf *Buf;

	Buf = NewStrBufPlain(HKEY("one"));
	NextTokenizerIterateBuf(Buf, 8);
	FreeStrBuf(&Buf);
}

static void TestNextTokenizer_Sequence(void)
{
	StrBuf *Buf;
        char *teststring = "40:24524,24662,24673,27869:27935,28393,28426,31247:31258,31731,31749,31761,31778,31782,31801:31803,31813,31904,31915,33708,33935,34619,34672,34720:34723,34766,34835,37594,38854,39235,39942,40030,40142,40520,40815,40907,41201,41578,41781,41954,42292,43110,43565,43801,43998,44180,44241,44295,44401,44561,44635,44798,44861,44946,45022,45137:45148,45166,45179,45707,47114,47141:47157,47194,47314,47349,47386,47489,47496,47534:47543,54460,54601,54637:54652";
        Buf = NewStrBufPlain(teststring, -1);
	NextTokenizerIterateBuf(Buf, 8);
	FreeStrBuf(&Buf);
}


/*
Some samples from the original...
	CU_ASSERT_EQUAL(10, 10);
	CU_ASSERT_EQUAL(0, -0);
	CU_ASSERT_EQUAL(-12, -12);
	CU_ASSERT_NOT_EQUAL(10, 11);
	CU_ASSERT_NOT_EQUAL(0, -1);
	CU_ASSERT_NOT_EQUAL(-12, -11);
	CU_ASSERT_PTR_EQUAL((void*)0x100, (void*)0x100);
	CU_ASSERT_PTR_NOT_EQUAL((void*)0x100, (void*)0x101);
	CU_ASSERT_PTR_NULL(NULL);
	CU_ASSERT_PTR_NULL(0x0);
	CU_ASSERT_PTR_NOT_NULL((void*)0x23);
	CU_ASSERT_STRING_EQUAL(str1, str2);
	CU_ASSERT_STRING_NOT_EQUAL(str1, str2);
	CU_ASSERT_NSTRING_EQUAL(str1, str2, strlen(str1));
	CU_ASSERT_NSTRING_EQUAL(str1, str1, strlen(str1));
	CU_ASSERT_NSTRING_EQUAL(str1, str1, strlen(str1) + 1);
	CU_ASSERT_NSTRING_NOT_EQUAL(str1, str2, 3);
	CU_ASSERT_NSTRING_NOT_EQUAL(str1, str3, strlen(str1) + 1);
	CU_ASSERT_DOUBLE_EQUAL(10, 10.0001, 0.0001);
	CU_ASSERT_DOUBLE_EQUAL(10, 10.0001, -0.0001);
	CU_ASSERT_DOUBLE_EQUAL(-10, -10.0001, 0.0001);
	CU_ASSERT_DOUBLE_EQUAL(-10, -10.0001, -0.0001);
	CU_ASSERT_DOUBLE_NOT_EQUAL(10, 10.001, 0.0001);
	CU_ASSERT_DOUBLE_NOT_EQUAL(10, 10.001, -0.0001);
	CU_ASSERT_DOUBLE_NOT_EQUAL(-10, -10.001, 0.0001);
	CU_ASSERT_DOUBLE_NOT_EQUAL(-10, -10.001, -0.0001);
*/

static void AddTests(void)
{
	CU_pSuite pGroup = NULL;
	CU_pTest pTest = NULL;

	pGroup = CU_add_suite("Sucess", success_init, success_clean);
	pTest = CU_add_test(pGroup, "testSuccess1", testSuccess1);
	pTest = CU_add_test(pGroup, "testSuccess2", testSuccess2);
	pTest = CU_add_test(pGroup, "testSuccess3", testSuccess3);
	
	pGroup = CU_add_suite("failure", NULL, NULL);
	pTest = CU_add_test(pGroup, "testfailure1", testfailure1);
	pTest = CU_add_test(pGroup, "testfailure2", testfailure2);
	pTest = CU_add_test(pGroup, "testfailure3", testfailure3);

	pGroup = CU_add_suite("group_failure", group_failure_init, group_failure_clean);
	pTest = CU_add_test(pGroup, "testGroupFailure1", testGroupFailure1);
	pTest = CU_add_test(pGroup, "testGroupFailure2", testGroupFailure2);
}

static void AddStrBufSimlpeTests(void)
{
	CU_pSuite pGroup = NULL;
	CU_pTest pTest = NULL;

	pGroup = CU_add_suite("TestStringBufSimpleAppenders", NULL, NULL);
	pTest = CU_add_test(pGroup, "testCreateBuf", TestCreateBuf);

	pGroup = CU_add_suite("TestStringTokenizer", NULL, NULL);
	pTest = CU_add_test(pGroup, "testNextTokenizer_EndWithEmpty", TestNextTokenizer_EndWithEmpty);
	pTest = CU_add_test(pGroup, "testNextTokenizer_StartWithEmpty", TestNextTokenizer_StartWithEmpty);
	pTest = CU_add_test(pGroup, "testNextTokenizer_StartWithEmpty", TestNextTokenizer_StartWithEmpty);
	pTest = CU_add_test(pGroup, "testNextTokenizer_Empty", TestNextTokenizer_Empty);
	pTest = CU_add_test(pGroup, "testNextTokenizer_TwoEmpty", TestNextTokenizer_TwoEmpty);
	pTest = CU_add_test(pGroup, "testNextTokenizer_One", TestNextTokenizer_One);
	pTest = CU_add_test(pGroup, "testNextTokenizer_Sequence", TestNextTokenizer_Sequence);


/*
	pGroup = CU_add_suite("TestBooleanAssert", NULL, NULL);
	pTest = CU_add_test(pGroup, "testSuccessAssertTrue", testSuccessAssertTrue);
	pTest = CU_add_test(pGroup, "testSuccessAssertFalse", testSuccessAssertFalse);

	pGroup = CU_add_suite("TestEqualityAssert", NULL, NULL);
	pTest = CU_add_test(pGroup, "testSuccessAssertEqual", testSuccessAssertEqual);
	pTest = CU_add_test(pGroup, "testSuccessAssertNotEqual", testSuccessAssertNotEqual);

	pGroup = CU_add_suite("TestPointerAssert", NULL, NULL);
	pTest = CU_add_test(pGroup, "testSuccessAssertPtrEqual", testSuccessAssertPtrEqual);
	pTest = CU_add_test(pGroup, "testSuccessAssertPtrNotEqual", testSuccessAssertPtrNotEqual);

	pGroup = CU_add_suite("TestNullnessAssert", NULL, NULL);
	pTest = CU_add_test(pGroup, "testSuccessAssertPtrNull", testSuccessAssertPtrNull);
	pTest = CU_add_test(pGroup, "testSuccessAssertPtrNotNull", testSuccessAssertPtrNotNull);

	pGroup = CU_add_suite("TestStringAssert", NULL, NULL);
	pTest = CU_add_test(pGroup, "testSuccessAssertStringEqual", testSuccessAssertStringEqual);
	pTest = CU_add_test(pGroup, "testSuccessAssertStringNotEqual", testSuccessAssertStringNotEqual);

	pGroup = CU_add_suite("TestNStringAssert", NULL, NULL);
	pTest = CU_add_test(pGroup, "testSuccessAssertNStringEqual", testSuccessAssertNStringEqual);
	pTest = CU_add_test(pGroup, "testSuccessAssertNStringNotEqual", testSuccessAssertNStringNotEqual);

	pGroup = CU_add_suite("TestDoubleAssert", NULL, NULL);
	pTest = CU_add_test(pGroup, "testSuccessAssertDoubleEqual", testSuccessAssertDoubleEqual);
	pTest = CU_add_test(pGroup, "testSuccessAssertDoubleNotEqual", testSuccessAssertDoubleNotEqual);
*/
}


int main(int argc, char* argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);

	StartLibCitadel(8);
	if (argc > 1) {
		CU_BOOL Run = CU_FALSE ;

		CU_set_output_filename("TestAutomated");
		if (CU_initialize_registry()) {
			printf("\nInitialize of test Registry failed.");
		}

		if (!strcmp("--test", argv[1])) {
			Run = CU_TRUE ;
			AddTests();
		}
		else if (!strcmp("--atest", argv[1])) {
			Run = CU_TRUE ;
			AddStrBufSimlpeTests();
		}
		else if (!strcmp("--alltest", argv[1])) {
			Run = CU_TRUE ;
			AddTests();
//			AddAssertTests();
		}
		
		if (CU_TRUE == Run) {
			//CU_console_run_tests();
    printf("\nTests completed with return value %d.\n", CU_basic_run_tests());

			///CU_automated_run_tests();
		}

		CU_cleanup_registry();
	}

	return 0;
}