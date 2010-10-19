// Copyright (c) 1998-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <s32cont.h>
#include <s32page.h>
#include <s32mem.h>
#include <e32test.h>
#include "U32STD.H"

LOCAL_D RTest TheTest(_L("t_storset"));

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check1(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		TheTest.Printf(_L("*** Expression  evaluated to false. Line %d\r\n"), aLine);
		TheTest(EFalse, aLine);
		}
	}
void Check2(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		TheTest.Printf(_L("*** Line %d, Expected error: %d, got: %d\r\n"), aLine, aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
void Check3(TUint aValue, TUint aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		TheTest.Printf(_L("*** Line %d, Expected value: %u, got: %u\r\n"), aLine, aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST(arg) ::Check1((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check2(aValue, aExpected, __LINE__)
#define TEST3(aValue, aExpected) ::Check2(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////

/**
@SYMTestCaseID          SYSLIB-STORE-CT-1121
@SYMTestCaseDesc	    TPagedSet class functionality test
@SYMTestPriority 	    High
@SYMTestActions  	    Tests insert/delete/contains without duplicates.
                        Tests for emptying the set
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
LOCAL_C void test1L()
	{
	TheTest.Printf(_L("Basic operations\r\n"));
	
	const TInt KEntryCount=200;
	TPagedSet<TInt32> set;
	set.Connect(CMemPagePool::NewLC());
	//IsIntact() and IsDirty() test
	TheTest.Printf(_L("IsIntact() and IsDirty() test\r\n"));
	TBool rc = set.IsIntact();
	TEST(rc);
	rc = set.IsDirty();
	TEST(!rc);
	set.MarkDirty();
	rc = set.IsDirty();
	TEST(rc);
	//IsBroken() test
	TheTest.Printf(_L("IsBroken() test\r\n"));
	rc = set.IsBroken();
	TEST(!rc);
	set.MarkBroken();
	rc = set.IsBroken();
	TEST(!rc);//Empty tree - cannot be marked as broken
	TInt yy = 10;
	set.InsertL(yy);
	set.MarkBroken();
	rc = set.IsBroken();
	TEST(rc);
	TheTest.Printf(_L("RepairL()test\r\n"));
	set.RepairL();
	rc = set.IsBroken();
	TEST(!rc);
	TheTest.Printf(_L("ClearL() test\r\n"));
	set.ClearL();
	rc = set.IsBroken();
	TEST(!rc);

	TheTest.Printf(_L("InsertL() and DeleteL() test\r\n"));
	TInt32 it=0;
//*	TEST(set.InsertL(it));
	set.InsertL(it);
	TEST2(set.Count(), 1);
//*	TEST(!set.InsertL(it));
	TEST2(set.Count(), 1);
	TEST(set.ContainsL(it));
//*	TEST(set.DeleteL(it));
	set.DeleteL(it);
	TEST2(set.Count(), 0);
//*	TEST(!set.DeleteL(it));
//*	TEST(set.Count()==0);
	TEST(!set.ContainsL(it));

//*	TheTest.Printf(_L("Duplicates"));
	TInt ii;
//*	for (ii=0;ii<KEntryCount;++ii)
//*		TEST(set.InsertL(it,EAllowDuplicates));
//*	TEST2(set.Count(), KEntryCount);
//*	TEST(set.ContainsL(it));
//*	TEST(!set.InsertL(it));
//*	for (ii=0;ii<KEntryCount;++ii)
//*		TEST(set.DeleteL(it));
//*	TEST(!set.ContainsL(it));
//*	TEST(!set.DeleteL(it));
//*	TEST2(set.Count(), 0);

	TheTest.Printf(_L("No duplicates\r\n"));
	for (ii=0;ii<KEntryCount;++ii)
		{
		it=ii;
//*		TEST(set.InsertL(it));
		set.InsertL(it);
		}
	for (ii=0;ii<KEntryCount;++ii)
		{
		it=ii;
//*		TEST(!set.InsertL(it));
		TEST(set.ContainsL(it));
		}
	TEST2(set.Count(), KEntryCount);

	TheTest.Printf(_L("Empty the set\r\n"));
	set.ClearL();
	TEST2(set.Count(), 0);
	for (ii=0;ii<KEntryCount;++ii)
		{
		it=ii;
		TEST(!set.ContainsL(it));
		}

	CleanupStack::PopAndDestroy();
	}
/**
@SYMTestCaseID          SYSLIB-STORE-CT-1122
@SYMTestCaseDesc	    TPagedSet class functionality test with large (10000) set of TUint32.
@SYMTestPriority 	    High
@SYMTestActions  	    Insert,delete,contains,iteration operations test
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
LOCAL_C void test2L()
	{
	const TInt KEntryCount=10000;

	TPagedSet<TUint32> set;
//*	set.Connect(CMemPagePool::NewLC(),TBtree::EQosFastest);
	set.Connect(CMemPagePool::NewLC());

	TheTest.Printf(_L("Add items\r\n"));
	TUint32 jj=0;
	TInt32 ii;
	for (ii=KEntryCount;ii>0;--ii)
		{
		jj=(jj+17)%KEntryCount;
//*		TEST(set.InsertL(jj));
		set.InsertL(jj);
		}
	TEST2(set.Count(), KEntryCount);

	TheTest.Printf(_L("Check contents\r\n"));
	for (ii=0;ii<KEntryCount;++ii)
		TEST(set.ContainsL(ii));

	TheTest.Printf(_L("Iterate over items\r\n"));
	TUint8 *checkMap=(TUint8*)User::AllocLC(KEntryCount);
	Mem::FillZ(checkMap,KEntryCount);
	TPagedSetIter<TUint32> iter(set);
	if (iter.ResetL())
		{
		do	
			{
			TUint32 data1 = iter.AtL();
			++checkMap[data1];
			TUint32 data2;
			iter.ExtractAtL(data2);
			TEST3(data1, data2);
			}while(iter.NextL());
		}
	for (ii=0;ii<KEntryCount;++ii)
		TEST2(checkMap[ii], 1);
	CleanupStack::PopAndDestroy();

	TheTest.Printf(_L("Delete items\r\n"));
	jj=0;
	for (ii=KEntryCount;ii>KEntryCount/2;--ii)
		{
		jj=(jj+17)%KEntryCount;
//*		TEST(set.DeleteL(jj));
		set.DeleteL(jj);
		}
	TEST2(set.Count(), KEntryCount/2);

	TheTest.Printf(_L("Check contents\r\n"));
	for (;ii>0;--ii)
		{
		jj=(jj+17)%KEntryCount;
		TEST(set.ContainsL(jj));
		}
	jj=0;
	for (ii=KEntryCount;ii>KEntryCount/2;--ii)
		{
		jj=(jj+17)%KEntryCount;
		TEST(!set.ContainsL(jj));
		}

	TheTest.Printf(_L("Delete items\r\n"));
	for (;ii>1;--ii)
		{
		jj=(jj+17)%KEntryCount;
//*		TEST(set.DeleteL(jj));
		set.DeleteL(jj);
		}
	TEST2(set.Count(), 1);

	TheTest.Printf(_L("Check contents\r\n"));
	jj=(jj+17)%KEntryCount;
	TPagedSetBiIter<TUint32> biter(set);
	TEST(biter.FirstL());
	TEST3(biter.AtL(), jj);
	TUint32 data; 
	biter.ExtractAtL(data);
	TEST3(data, jj);
	TEST(!biter.NextL());
	TEST(biter.LastL());
	TEST3(biter.AtL(), jj);
	TEST(!biter.PreviousL());
	TPagedSetRIter<TUint32> riter(set);
	TEST(riter.ResetL());
	TEST3(riter.AtL(), jj);
	riter.ExtractAtL(data);
	TEST3(data, jj);
	TEST(!riter.NextL());

//*	TEST(set.DeleteL(jj));
	set.DeleteL(jj);
	TEST(!iter.ResetL());
	TEST2(set.Count(), 0);

	CleanupStack::PopAndDestroy();
	}
/**
@SYMTestCaseID          SYSLIB-STORE-CT-1123
@SYMTestCaseDesc	    Stream set out test
@SYMTestPriority 	    High
@SYMTestActions  	    Build set and stream out
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
LOCAL_C void test3aL(RWriteStream& aStream,MPagePool *aPool,TInt aCount)
	{
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1123"));
	TPagedSet<TInt32> set;
//*	set.Connect(aPool,TBtree::EQosFastest);
	set.Connect(aPool);

	for (TInt ii=0;ii<aCount;ii++)
		{
		TInt32 it=ii;
//*		test(set.InsertL(it));
		set.InsertL(it);
		}
	aStream<<set.Token();
	}
/**
@SYMTestCaseID          SYSLIB-STORE-CT-1124
@SYMTestCaseDesc	    Stream in and set test
@SYMTestPriority 	    High
@SYMTestActions  	    Read a token from a stream,create and a pagedset.Tests for emptying the set.
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
LOCAL_C void test3bL(RReadStream& aStream,MPagePool *aPool,TInt aCount)
	{
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1124"));
	TPagedSetToken token;
	aStream>>token;
	TPagedSet<TInt32> set(token);
//*	set.Connect(aPool,TBtree::EQosFastest);
	set.Connect(aPool);

	TEST2(set.Count(), aCount);
	for (TInt ii=0;ii<aCount;ii++)
		{
		TInt32 it=ii;
//*		TEST(set.DeleteL(it));
		set.DeleteL(it);
		}
	TEST2(set.Count(), 0);
	}

/**
@SYMTestCaseID          SYSLIB-STORE-CT-1125
@SYMTestCaseDesc	    Streaming sets test
@SYMTestPriority 	    High
@SYMTestActions  	    Tests for token streaming operations on pagedsets.
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
LOCAL_C void test3L()
	{
	const TInt KEntryCount=1000;
	MPagePool *pool=CMemPagePool::NewLC();
	TUint8 stream[0x40];
	TheTest.Printf(_L("Build set and stream out\r\n"));
	RMemWriteStream out(stream,sizeof(stream));
	test3aL(out,pool,KEntryCount);
	TheTest.Printf(_L("Stream in and test set\r\n"));
	RMemReadStream in(stream,sizeof(stream));
	test3bL(in,pool,KEntryCount);
	CleanupStack::PopAndDestroy();
	}

class CPersistentStoreHelper: public CPersistentStore
	{
private:
	virtual MStreamBuf* DoReadL(TStreamId) const
		{
		return NULL;
		}
	virtual MStreamBuf* DoCreateL(TStreamId&)
		{
		return NULL;
		}
	};

void DoBiITerMultiSetTestL(TPagedSetBiIter<TInt32>& aIter, TInt aCount)
	{
	TInt count = 0;
	TBool rc = aIter.FirstL();
	TEST(rc);
	++count;
	while(aIter.NextL())
		{
		++count;
		}
	TEST2(count, aCount);
	
	count = 0;
	rc = aIter.LastL();
	TEST(rc);
	++count;
	while(aIter.PreviousL())
		{
		++count;
		}
	TEST2(count, aCount);
	}

void DoRIterMultiSetTestL(TPagedSetRIter<TInt32> aIter, TInt aCount)
	{
	TInt count = 0;
	TBool rc = aIter.ResetL();
	TEST(rc);
	++count;
	while(aIter.NextL())
		{
		++count;
		}
	TEST2(count, aCount);
	}

void DoRIterAnyMultiSetTestL(TPagedSetRIter<TAny> aIter, TInt aCount)
	{
	TInt count = 0;
	TBool rc = aIter.ResetL();
	TEST(rc);
	++count;
	while(aIter.NextL())
		{
		++count;
		}
	TEST2(count, aCount);
	}

void DoIterMultiSetTestL(TPagedSetIter<TInt32> aIter, TInt aCount)
	{
	TInt count = 0;
	TBool rc = aIter.ResetL();
	TEST(rc);
	++count;
	while(aIter.NextL())
		{
		++count;
		}
	TEST2(count, aCount);
	}

void DoIterAnyMultiSetTestL(TPagedSetIter<TAny> aIter, TInt aCount)
	{
	TInt count = 0;
	TBool rc = aIter.ResetL();
	TEST(rc);
	++count;
	while(aIter.NextL())
		{
		++count;
		}
	TEST2(count, aCount);
	}

/**
@SYMTestCaseID          PDS-STORE-CT-4015
@SYMTestCaseDesc	    Test untested APIs of TPagedMultiset and TPagedSetToken
@SYMTestPriority 	    High
@SYMTestActions  	    Test possibility of adding duplicates into TPagedMultiset. Calling empy constructor TPagedSetToken.
						Test RepairL();
@SYMTestExpectedResults Insterting duplicates should be possible and should not fail. After adding KEntryCount
						identical elements Count() should equal KEntryCount. Constructor should create valid object.
						RepairL function can't be runned now, because it has a problem inside that cause KERN-EXEC: 3.
@SYMDEF                 DEF135804
*/
LOCAL_C void test4L()
	{
	const TInt KEntryCount=200;

	TheTest.Printf(_L("Test untested APIs\r\n"));

	TInt32 it=0;
	TPagedMultiset<TInt32> set;
	set.Connect(CMemPagePool::NewLC());
	TheTest.Printf(_L("Duplicates\r\n"));
	TInt ii, err;
	for (ii=0;ii<KEntryCount;++ii)
		{
		TRAP(err, set.InsertL(it));
		TEST2(err, KErrNone);
		}
	TEST2(set.Count(), KEntryCount);
	TBool rc = set.IsEmpty();
	TEST(!rc);

	TPagedSetBiIter<TInt32> biIter(set);
	DoBiITerMultiSetTestL(biIter, set.Count());

	TPagedSetRIter<TInt32> rIter(set);
	DoRIterMultiSetTestL(rIter, set.Count());
	TPagedSetRIter<TAny> rIter2(set);
	DoRIterAnyMultiSetTestL(rIter2, set.Count());

	TPagedSetIter<TInt32> iter(set);
	DoIterMultiSetTestL(iter, set.Count());
	TPagedSetIter<TAny> iter2(set);
	DoIterAnyMultiSetTestL(iter2, set.Count());
	
	TPagedSetToken token = set.Token();
	TPagedMultiset<TInt32> multiSet2(token);
	TEST2(multiSet2.Count(), set.Count());
	TPagedMultiset<TAny> multiSet3(token, sizeof(TInt32));
	TEST2(multiSet3.Count(), set.Count());
	TPagedMultiset<TAny> multiSet4(sizeof(TInt32));
	TEST2(multiSet4.Count(), 0);
	TPagedSet<TAny> set5(token, sizeof(TInt32));
	TEST2(set5.Count(), set.Count());
	TPagedSet<TAny> set6(sizeof(TInt32));
	TEST2(set6.Count(), 0);
	
	set.MarkDirty();
	set.MarkCurrent();
	TRAP(err, set.ContainsL(it));
	TEST2(err, KErrNone);
	TRAP(err, set.InsertL(it));
	TEST2(err, KErrNone);
		
	for (ii=0;ii<KEntryCount;++ii)
		{
		TRAP(err, set.DeleteL(it));
		TEST2(err, KErrNone);
		}
		
	TRAP(err, set.ContainsL(it));
	TEST2(err, KErrNone);
	TRAP(err, set.DeleteL(it));
	TEST2(err, KErrNone);
	TEST2(set.Count(), 0);
	
	TheTest.Printf(_L("Calling MPagePool::Delete\r\n"));
	CMemPagePool* mpp = CMemPagePool::NewLC();
	const TPageAbandonFunction& nopFunc = mpp->AcquireL();
	TEST(&nopFunc != NULL);
	TAny* any = mpp->AllocL();
	TPageRef pref;
	pref = mpp->AssignL(any, EPageReclaimable);
	mpp->MPagePool::Delete(pref);
	CleanupStack::PopAndDestroy();
	
	TheTest.Printf(_L("CPersistentStore::DoSetRootL\r\n"));
	CPersistentStoreHelper* ps = new (ELeave) CPersistentStoreHelper();
	CleanupStack::PushL(ps);
	ps->SetRootL(KNullStreamId);
	TRAP(err, ps->ExtendL());
	TEST2(err, KErrNotSupported);
	TRAP(err, ps->DeleteL(TStreamId(1u)));
	TEST2(err, KErrNotSupported);
	CleanupStack::PopAndDestroy(ps);
	
	TheTest.Printf(_L("HDirectStoreBuf::DoSeekL calls\r\n"));
	HBufC8* buf = HBufC8::NewLC(1024);
	RDesWriteStream wts;
	
	TPtr8 ptr(buf->Des());
	wts.Open(ptr);
	TStreamId id(5);
	wts << id;
	wts.CommitL();
	wts.Close();
	buf->Des().Append(_L8("Ala ma kota a kot ma futro. Futro jest dobre by chronic przed zimnem."));
	RDesReadStream rts;
	ptr.Set(buf->Des());
	rts.Open(ptr);

	CEmbeddedStore* estor = CEmbeddedStore::FromLC(rts);
	RStoreReadStream rstream;
	rstream.OpenL(*estor, id);
	TStreamPos pos = rstream.Source()->SeekL(MStreamBuf::ERead, 5);
	TEST2(pos.Offset(), 5);
	rts.Close();
	rstream.Close();
	CleanupStack::PopAndDestroy(2);
	
	TheTest.Printf(_L("Calling TEmpty Constructor\r\n"));
	TPagedSetToken set3(TBtreeToken::EEmpty);
	TEST2( set3.Count(), 0);
	
	TheTest.Printf(_L("Set function\r\n"));
	set.Set(set3);
	const TPagedSetToken& pst = set.Token();
	TEST2(pst.Count(), set3.Count());
	
	CleanupStack::PopAndDestroy();
	}

/**
@SYMTestCaseID          PDS-STORE-CT-4065
@SYMTestCaseDesc        TStreamPos tests.
@SYMTestActions         Tests operations provided by TStreamPos class. 
@SYMTestPriority        High
@SYMTestExpectedResults Test must not fail
*/
void StreamPosTest()
	{
	TStreamPos pos1;
	TStreamPos pos2(5);
	pos1 = pos2;
	TEST(pos1 == pos2);
	
	pos1 = 5 + pos2;
	TEST(pos1 > pos2);
	TEST(pos2 < pos1);
	TEST(pos2 <= pos1);
	TEST(pos1 != pos2);
	pos1 = pos1 - 5;
	TEST(pos1 == pos2);
	
	pos2 += 0;
	TEST(pos1 == pos2);
	pos2 -= 0;
	TEST(pos1 == pos2);
	}

struct TTestEntry
	{
	inline TTestEntry() :
		iKey(-1),
		iData(-1)
		{
		}
	inline TTestEntry(TInt aKey, TInt aData) :
		iKey(aKey),
		iData(aData)
		{
		}
	TInt	iKey;
	TInt	iData;
	};

/**
@SYMTestCaseID          PDS-STORE-CT-4066
@SYMTestCaseDesc        TBtreeFix tests.
@SYMTestActions         Tests operations provided by TBtreeFix class. 
@SYMTestPriority        High
@SYMTestExpectedResults Test must not fail
*/
void BTreeFixTestL()
	{
	CMemPagePool* pool = CMemPagePool::NewLC();
		
	TBtreeToken token(TBtreeToken::EEmpty);
	TBool rc = token.IsEmpty();
	TEST(rc);
	rc = token.IsIntact();
	TEST(rc);
	
	TBtreeFix<TTestEntry, TInt> bentry(token, EBtreeSecure);
	TBtreeKey bkey(sizeof(TInt));
	bentry.Connect(pool, &bkey);
	
	TBtreePos bpos;
	rc = bentry.FindL(bpos, 1);
	TEST(!rc);
	rc = bentry.InsertL(bpos, TTestEntry(1, 101));
	TEST(rc);
	rc = bentry.FindL(bpos, 1);
	TEST(rc);
	TTestEntry entry1 = bentry.AtL(bpos);
	TEST(entry1.iKey == 1 && entry1.iData == 101);
	const void* key = bkey.Key(&entry1);
	TInt keyVal = *((const TInt*)key);
	TheTest.Printf(_L("keyVal=%d\r\n"), keyVal);
	
	rc = bentry.InsertL(bpos, TTestEntry(3, 103));
	TEST(rc);
	rc = bentry.InsertL(bpos, TTestEntry(2, 102));
	TEST(rc);
	
	rc = bentry.FindL(bpos, 2);
	TEST(rc);
	TTestEntry entry2;
	bentry.ExtractAtL(bpos, entry2);
	TEST(entry2.iKey == 2 && entry2.iData == 102);

	rc = bentry.FindL(bpos, 3);
	TEST(rc);
	TTestEntry entry3;
	bentry.ExtractAtL(bpos, entry3);
	TEST(entry3.iKey == 3 && entry3.iData == 103);

	//==============================================
	
	TBtreeMark bmark;
	if(bentry.ResetL(bmark))
		{
		do
			{
			TTestEntry entry = bentry.AtL(bmark);
			TheTest.Printf(_L("AtL(): entry.iKey=%d, entry.iData=%d\r\n"), entry.iKey, entry.iData);
			bentry.ExtractAtL(bmark, entry);
			TheTest.Printf(_L("ExtractAtL(): entry.iKey=%d, entry.iData=%d\r\n"), entry.iKey, entry.iData);
			}while(bentry.NextL(bmark));
		}

	rc = bentry.NextL(bmark);
	TEST(!rc);

	//==============================================

	rc = bentry.DeleteL(2);
	TEST(rc);
	rc = bentry.FindL(bpos, 2);
	TEST(!rc);
	rc = bentry.FindL(bpos, 3);
	TEST(rc);
	TRAPD(err, bentry.DeleteAtL(bpos));
	TEST(err == KErrNone);
	rc = bentry.FindL(bpos, 3);
	TEST(!rc);
	
	bentry.MarkDirty();
	rc = bentry.IsDirty();
	TEST(rc);
	bentry.MarkCurrent();
	rc = bentry.IsDirty();
	TEST(!rc);
	
	bentry.ClearL();
	CleanupStack::PopAndDestroy(pool);
	}

LOCAL_C void DoTestsL()
	{
	TheTest.Start(_L("@SYMTestCaseID:SYSLIB-STORE-CT-1121 Basic operations"));
	test1L();
	TheTest.Next(_L("@SYMTestCaseID:SYSLIB-STORE-CT-1122 Large set TUint32"));
	test2L();
	TheTest.Next(_L("@SYMTestCaseID:SYSLIB-STORE-CT-1125 Tokens and streaming"));
	test3L();
	TheTest.Next(_L("@SYMTestCaseID:PDS-STORE-CT-4015 Forgotten API"));
	test4L();
	TheTest.Next(_L("@SYMTestCaseID:PDS-STORE-CT-4065: TStreamPos test"));
	StreamPosTest();
	TheTest.Next(_L("@SYMTestCaseID:PDS-STORE-CT-4066: TBtreeFix test"));
	BTreeFixTestL();
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

    User::Heap().Check();
    
    TheTest.End();
    TheTest.Close();
    
    delete tc;

    return KErrNone;
	}

