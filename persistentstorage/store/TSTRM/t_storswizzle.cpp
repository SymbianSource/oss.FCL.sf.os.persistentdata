// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// 
//
#include "S32STD.H"
#include "S32MEM.H"
#include <e32test.h>

RTest TheTest(_L("t_storswizzle"));

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		TheTest.Printf(_L("*** Expression evaluated to false.\r\n"));
		TheTest(EFalse, aLine);
		}
	}
void Check(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		TheTest.Printf(_L("*** Expected error: %d, got: %d.\r\n"), aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST(arg) ::Check((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////

class TRectangle
	{
public:
	TRectangle();
	TRectangle(TInt aWidth, TInt aHeight);

	void ExternalizeL(RWriteStream& aStream) const;
	void InternalizeL(RReadStream& aStream);
	
public:
	TInt	iWidth;
	TInt	iHeight;
	};

TBool operator==(const TRectangle& aLeft, const TRectangle& aRight)
	{
	return aLeft.iWidth == aRight.iWidth && aLeft.iHeight == aRight.iHeight;
	}

TRectangle::TRectangle() :
	iWidth(-1),
	iHeight(-1)
	{
	}

TRectangle::TRectangle(TInt aWidth, TInt aHeight) :
	iWidth(aWidth),
	iHeight(aHeight)
	{
	}

void TRectangle::ExternalizeL(RWriteStream& aStream) const
	{
	aStream << (TInt32)iWidth;
	aStream << (TInt32)iHeight;
	}

void TRectangle::InternalizeL(RReadStream& aStream)
	{
	TInt32 a;
	aStream >> a;
	iWidth = a;
	aStream >> a;
	iHeight = a;
	}

class TRectangleExternalizer : public MExternalizer<TRectangle>
	{
public:
	virtual void ExternalizeL(const TRectangle& aObject, RWriteStream& aStream) const;
	};

void TRectangleExternalizer::ExternalizeL(const TRectangle& aObject, RWriteStream& aStream) const
	{
	aStream << aObject;
	}

class TRectangleInternalizer : public MInternalizer<TRectangle>
	{
public:
	virtual void InternalizeL(TRectangle& aObject, RReadStream& aStream) const;
	};

void TRectangleInternalizer::InternalizeL(TRectangle& aObject, RReadStream& aStream) const
	{
	aStream >> aObject;
	}

void RectangleExternalizeL(const TAny* aPtr, RWriteStream& aStream)
	{
	TRectangle* rcPtr = (TRectangle*)aPtr;
	rcPtr->ExternalizeL(aStream);
	}

class TStreamRefExternalizer : public MExternalizer<TStreamRef>
	{
public:
	virtual void ExternalizeL(const TStreamRef& aObject, RWriteStream& aStream) const;
	};

void TStreamRefExternalizer::ExternalizeL(const TStreamRef& aObject, RWriteStream& aStream) const
	{
	TExternalizeFunction func = aObject.Function();
	(*func)(aObject.Ptr(), aStream);
	}

///////////////////////////////////////////////////////////////////////////////////////

/**
@SYMTestCaseID          PDS-STORE-CT-4060
@SYMTestCaseDesc        TSwizzleC<T> tests.
@SYMTestActions         TSwizzleC<T> functionality test. 
@SYMTestPriority        High
@SYMTestExpectedResults Test must not fail
*/
void TestSwizzleCL()
	{
	CBufStore* bufStore = CBufStore::NewLC(100);
	
	const TInt KWidth = 10;
	const TInt KHeight = 20;
	TRectangle r1(KWidth, KHeight);

	const TInt KWidth2 = 6;
	const TInt KHeight2 = 17;
	TRectangle r2(KWidth2, KHeight2);
	
	const TInt KWidth3 = 19;
	const TInt KHeight3 = 5;
	TRectangle r3(KWidth3, KHeight3);

	const TInt KWidth4 = 22;
	const TInt KHeight4 = 19;
	TRectangle r4(KWidth4, KHeight4);
	
	//Externalize r1 using RStoreWriteStream directly 
	RStoreWriteStream wstrm1;
	TStreamId strmId1 = wstrm1.CreateLC(*bufStore);
	TSwizzleC<TRectangle> swizzle1(&r1);
	TEST((const void*)swizzle1 == (const void*)&r1);
	wstrm1 << *swizzle1;
	wstrm1.CommitL();
	CleanupStack::PopAndDestroy(&wstrm1);

	//Externalize r2 using TRectangleExternalizer 
	RStoreWriteStream wstrm2;
	TStreamId strmId2 = wstrm2.CreateLC(*bufStore);
	TSwizzleC<TRectangle> swizzle2(&r2);
	TEST((const void*)swizzle2 == (const void*)&r2);
	TRectangleExternalizer rcExternalizer2;
	rcExternalizer2(*swizzle2, wstrm2);
	wstrm2.CommitL();
	CleanupStack::PopAndDestroy(&wstrm2);

	//Externalize r3 using TExternalizer<TRectangle> 
	RStoreWriteStream wstrm3;
	TStreamId strmId3 = wstrm3.CreateLC(*bufStore);
	TSwizzleC<TRectangle> swizzle3(&r3);
	TEST((const void*)swizzle3 == (const void*)&r3);
	TExternalizer<TRectangle> rcExternalizer3;
	rcExternalizer3(*swizzle3, wstrm3);
	wstrm3.CommitL();
	CleanupStack::PopAndDestroy(&wstrm3);
	
	//Externalize r4 using TSwizzleC::ExternalizeL()
	TStreamRefExternalizer rcExternalizer4;
	RStoreWriteStream wstrm4(rcExternalizer4);
	TStreamId strmId4 = wstrm4.CreateLC(*bufStore);
	TSwizzleC<TRectangle> swizzle4(&r4);
	TEST((const void*)swizzle4 == (const void*)&r4);
	swizzle4.ExternalizeL(wstrm4);
	wstrm4.CommitL();
	CleanupStack::PopAndDestroy(&wstrm4);
	
	//Internalize r1 using RStoreReadStream directly 
	TRectangle r1in;
	RStoreReadStream rstrm1;
	rstrm1.OpenLC(*bufStore, strmId1);
	rstrm1 >> r1in;
	CleanupStack::PopAndDestroy(&rstrm1);
	TEST(r1 == r1in);

	//Internalize r2 using TRectangleInternalizer
	TRectangle r2in;
	TSwizzle<TRectangle> swizzle2in(&r2in);
	RStoreReadStream rstrm2;
	rstrm2.OpenLC(*bufStore, strmId2);
	TRectangleInternalizer rcInternalizer2;
	rcInternalizer2(*swizzle2in, rstrm2);
	CleanupStack::PopAndDestroy(&rstrm2);
	TEST(r2 == r2in);
	TEST(*swizzle2 == *swizzle2in);

	//Internalize r3 using TInternalizer<TRectangle>
	TRectangle r3in;
	TSwizzle<TRectangle> swizzle3in(&r3in);
	RStoreReadStream rstrm3;
	rstrm3.OpenLC(*bufStore, strmId3);
	TInternalizer<TRectangle> rcInternalizer3;
	rcInternalizer3(*swizzle3in, rstrm3);
	CleanupStack::PopAndDestroy(&rstrm3);
	TEST(r3 == r3in);
	TEST(*swizzle3 == *swizzle3in);

	//Internalize r4
	TSwizzle<TRectangle> swizzle4in(strmId4);
	TEST(swizzle4in.IsId());
	RStoreReadStream rstrm4;
	rstrm4.OpenLC(*bufStore, swizzle4in.AsId());
	TRectangle r4in;
	rstrm4 >> r4in;
	CleanupStack::PopAndDestroy(&rstrm4);
	TEST(r4 == r4in);
	
	CleanupStack::PopAndDestroy(bufStore);

	//TSwizzleC copy constructor test
	TSwizzleC<TRectangle> swizzle1copy(swizzle1);
	TEST(swizzle1->iWidth == swizzle1copy->iWidth);
	TEST(swizzle1->iHeight == swizzle1copy->iHeight);
	TEST(swizzle1.AsPtr()->iHeight == swizzle1copy.AsPtr()->iHeight);

	//TSwizzleC assignment operator test
	TSwizzleC<TRectangle> swizzle1assign;
	swizzle1assign = &r1in;
	TEST(swizzle1->iWidth == swizzle1assign->iWidth);
	TEST(swizzle1->iHeight == swizzle1assign->iHeight);
	TEST(swizzle1.AsPtr()->iHeight == swizzle1assign.AsPtr()->iHeight);
	}

/**
@SYMTestCaseID          PDS-STORE-CT-4061
@SYMTestCaseDesc        TSwizzleC<TAny> tests.
@SYMTestActions         TSwizzleC<TAny> functionality test. 
@SYMTestPriority        High
@SYMTestExpectedResults Test must not fail
*/
void TestSwizzleCAny()
	{
	const TInt KWidth = 10;
	const TInt KHeight = 20;
	TRectangle r1(KWidth, KHeight);
	
	TSwizzleC<TAny> swizzle1(&r1);
	TSwizzleC<TAny> swizzle2(&r1);
	TSwizzleC<TAny> swizzle3;
	swizzle3 = &r1;
	TEST((const void*)swizzle3 == (const void*)&r1);
	
	TSwizzleCBase b1 = swizzle1; 
	TSwizzleCBase b2 = swizzle2;
	TBool rc = b1 == b2;
	TEST(rc);
	rc = b1 != b2;
	TEST(!rc);
	
	TSwizzleC<TAny> swizzle4(b1);
	TEST(swizzle4.AsPtr() == swizzle1.AsPtr());

	const void* p1 = swizzle1.AsPtr();
	const void* p2 = swizzle2.AsPtr();
	const void* p3 = swizzle3.AsPtr();
	
	TEST(((const TRectangle*)p1)->iWidth == ((const TRectangle*)p2)->iWidth);
	TEST(((const TRectangle*)p1)->iHeight == ((const TRectangle*)p2)->iHeight);
	TEST(((const TRectangle*)p3)->iWidth == ((const TRectangle*)p2)->iWidth);
	TEST(((const TRectangle*)p3)->iHeight == ((const TRectangle*)p2)->iHeight);
	}

/**
@SYMTestCaseID          PDS-STORE-CT-4062
@SYMTestCaseDesc        TSwizzle<TAny> tests.
@SYMTestActions         TSwizzle<TAny> functionality test. 
@SYMTestPriority        High
@SYMTestExpectedResults Test must not fail
*/
void TestSwizzleAny()
	{
	const TInt KWidth = 10;
	const TInt KHeight = 20;
	TRectangle r1(KWidth, KHeight);
		
	TSwizzle<TAny> swizzle1(&r1);
	TSwizzle<TAny> swizzle2(&r1);
	TSwizzle<TAny> swizzle3;
	swizzle3 = &r1;
	TEST((void*)swizzle3 == (void*)&r1);
	
	TSwizzleBase b1 = swizzle1; 
	TSwizzleBase b2 = swizzle2;
	TBool rc = b1 == b2;
	TEST(rc);
	rc = b1 != b2;
	TEST(!rc);
	
	TSwizzle<TAny> swizzle4(b1);
	TEST(swizzle4.AsPtr() == swizzle1.AsPtr());
	
	void* p1 = swizzle1.AsPtr();
	void* p2 = swizzle2.AsPtr();
	void* p3 = swizzle3.AsPtr();
	
	TEST(((TRectangle*)p1)->iWidth == ((TRectangle*)p2)->iWidth);
	TEST(((TRectangle*)p1)->iHeight == ((TRectangle*)p2)->iHeight);
	TEST(((TRectangle*)p3)->iWidth == ((TRectangle*)p2)->iWidth);
	TEST(((TRectangle*)p3)->iHeight == ((TRectangle*)p2)->iHeight);
	
	((TRectangle*)p3)->iWidth = 5;
	((TRectangle*)p3)->iHeight = 3;
	
	TEST2(r1.iWidth, 5);
	TEST2(r1.iHeight, 3);
	
	TSwizzle<TRectangle> swizzle5;
	swizzle5 = &r1;
	TEST2(swizzle5->iWidth, 5);
	TEST2(swizzle5->iHeight, 3);
	}

void DoTestsL()
	{
	TheTest.Start(_L("@SYMTestCaseID:PDS-STORE-CT-4060: TSwizzleC<T> test"));
	TestSwizzleCL();
	
	TheTest.Next(_L("@SYMTestCaseID:PDS-STORE-CT-4061: TSwizzleC<TAny> test"));
	TestSwizzleCAny();
	
	TheTest.Next(_L("@SYMTestCaseID:PDS-STORE-CT-4062: TSwizzle<TAny> test"));
	TestSwizzleAny();
	}

TInt E32Main()
	{
	TheTest.Title();

	CTrapCleanup* tc = CTrapCleanup::New();
	TheTest(tc != NULL);

	__UHEAP_MARK;

	TRAPD(err, DoTestsL());
	TEST2(err, KErrNone);

	__UHEAP_MARKEND;

	TheTest.End();
	TheTest.Close();

	delete tc;

	User::Heap().Check();
	return KErrNone;
	}
