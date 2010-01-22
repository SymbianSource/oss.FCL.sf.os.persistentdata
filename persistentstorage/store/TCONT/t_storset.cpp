// Copyright (c) 1998-2009 Nokia Corporation and/or its subsidiary(-ies).
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

const TInt KTestCleanupStack=0x20;

LOCAL_D CTrapCleanup* TheTrapCleanup;
LOCAL_D RTest test(_L("t_storset"));

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
	const TInt KEntryCount=200;

	TPagedSet<TInt32> set;
	set.Connect(CMemPagePool::NewLC());
	test.Start(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1121 Insertion & Deletion "));

	TInt32 it=0;
//*	test(set.InsertL(it));
	set.InsertL(it);
	test(set.Count()==1);
//*	test(!set.InsertL(it));
	test(set.Count()==1);
	test(set.ContainsL(it));
//*	test(set.DeleteL(it));
	set.DeleteL(it);
	test(set.Count()==0);
//*	test(!set.DeleteL(it));
//*	test(set.Count()==0);
	test(!set.ContainsL(it));

//*	test.Next(_L("Duplicates"));
	TInt ii;
//*	for (ii=0;ii<KEntryCount;++ii)
//*		test(set.InsertL(it,EAllowDuplicates));
//*	test(set.Count()==KEntryCount);
//*	test(set.ContainsL(it));
//*	test(!set.InsertL(it));
//*	for (ii=0;ii<KEntryCount;++ii)
//*		test(set.DeleteL(it));
//*	test(!set.ContainsL(it));
//*	test(!set.DeleteL(it));
//*	test(set.Count()==0);

	test.Next(_L("No duplicates"));
	for (ii=0;ii<KEntryCount;++ii)
		{
		it=ii;
//*		test(set.InsertL(it));
		set.InsertL(it);
		}
	for (ii=0;ii<KEntryCount;++ii)
		{
		it=ii;
//*		test(!set.InsertL(it));
		test(set.ContainsL(it));
		}
	test(set.Count()==KEntryCount);

	test.Next(_L("Empty the set"));
	set.ClearL();
	test(set.Count()==0);
	for (ii=0;ii<KEntryCount;++ii)
		{
		it=ii;
		test(!set.ContainsL(it));
		}

	test.End();
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

	test.Start(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1122 Add items "));
	TUint32 jj=0;
	TInt32 ii;
	for (ii=KEntryCount;ii>0;--ii)
		{
		jj=(jj+17)%KEntryCount;
//*		test(set.InsertL(jj));
		set.InsertL(jj);
		}
	test(set.Count()==KEntryCount);

	test.Next(_L("Check contents"));
	for (ii=0;ii<KEntryCount;++ii)
		test(set.ContainsL(ii));

	test.Next(_L("Iterate over items"));
	TUint8 *checkMap=(TUint8*)User::AllocLC(KEntryCount);
	Mem::FillZ(checkMap,KEntryCount);
	TPagedSetIter<TUint32> iter(set);
	if (iter.ResetL())
		do	++checkMap[iter.AtL()]; while (iter.NextL());
	for (ii=0;ii<KEntryCount;++ii)
		test(checkMap[ii]==1);
	CleanupStack::PopAndDestroy();

	test.Next(_L("Delete items"));
	jj=0;
	for (ii=KEntryCount;ii>KEntryCount/2;--ii)
		{
		jj=(jj+17)%KEntryCount;
//*		test(set.DeleteL(jj));
		set.DeleteL(jj);
		}
	test(set.Count()==KEntryCount/2);

	test.Next(_L("Check contents"));
	for (;ii>0;--ii)
		{
		jj=(jj+17)%KEntryCount;
		test(set.ContainsL(jj));
		}
	jj=0;
	for (ii=KEntryCount;ii>KEntryCount/2;--ii)
		{
		jj=(jj+17)%KEntryCount;
		test(!set.ContainsL(jj));
		}

	test.Next(_L("Delete items"));
	for (;ii>1;--ii)
		{
		jj=(jj+17)%KEntryCount;
//*		test(set.DeleteL(jj));
		set.DeleteL(jj);
		}
	test(set.Count()==1);

	test.Next(_L("Check contents"));
	jj=(jj+17)%KEntryCount;
	TPagedSetBiIter<TUint32> biter(set);
	test(biter.FirstL());
	test(biter.AtL()==jj);
	test(!biter.NextL());
	test(biter.LastL());
	test(biter.AtL()==jj);
	test(!biter.PreviousL());
	TPagedSetRIter<TUint32> riter(set);
	test(riter.ResetL());
	test(riter.AtL()==jj);
	test(!riter.NextL());

//*	test(set.DeleteL(jj));
	set.DeleteL(jj);
	test(!iter.ResetL());
	test(set.Count()==0);

	test.End();

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
	test.Next(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1123 "));
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
	test.Next(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1124 "));
	TPagedSetToken token;
	aStream>>token;
	TPagedSet<TInt32> set(token);
//*	set.Connect(aPool,TBtree::EQosFastest);
	set.Connect(aPool);

	test(set.Count()==aCount);
	for (TInt ii=0;ii<aCount;ii++)
		{
		TInt32 it=ii;
//*		test(set.DeleteL(it));
		set.DeleteL(it);
		}
	test(set.Count()==0);
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
	test.Start(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1125 Build set and stream out "));
	RMemWriteStream out(stream,sizeof(stream));
	test3aL(out,pool,KEntryCount);
	test.Next(_L("Stream in and test set "));
	RMemReadStream in(stream,sizeof(stream));
	test3bL(in,pool,KEntryCount);
	test.End();
	CleanupStack::PopAndDestroy();
	}

class CPersistentStoreHelper: public CPersistentStore
	{
	virtual MStreamBuf* DoReadL(TStreamId /*anId*/) const
		{
		return NULL;
		}
	virtual MStreamBuf* DoCreateL(TStreamId& /*anId*/)
		{
		return NULL;
		}
	void Help()
		{
		//just do nothing
		return;
		}
	};

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

	test.Start(_L(" @SYMTestCaseID:PDS-STORE-CT-4015 Test untested APIs "));

	TInt32 it=0;
	TPagedMultiset<TInt32> set;
	set.Connect(CMemPagePool::NewLC());
	test.Next(_L("Duplicates"));
	TInt ii, err;
	for (ii=0;ii<KEntryCount;++ii)
		{
		TRAP(err, set.InsertL(it));
		test(err==KErrNone);
		}
	test(set.Count()==KEntryCount);
	TRAP(err, set.ContainsL(it));
	test(err==KErrNone);
	TRAP(err, set.InsertL(it));
	test(err==KErrNone);
		
	for (ii=0;ii<KEntryCount;++ii)
		{
		TRAP(err, set.DeleteL(it));
		test(err==KErrNone);
		}
		
	TRAP(err, set.ContainsL(it));
	test(err==KErrNone);
	TRAP(err, set.DeleteL(it));
	test(err==KErrNone);
	test(set.Count()==0);
	
	test.Next(_L("Calling MPagePool::Delete"));
	CMemPagePool* mpp = CMemPagePool::NewLC();
	const TPageAbandonFunction& nopFunc = mpp->AcquireL();
	test(&nopFunc != NULL);
	TAny* any = mpp->AllocL();
	TPageRef pref;
	pref = mpp->AssignL(any, EPageReclaimable);
	mpp->MPagePool::Delete(pref);
	CleanupStack::PopAndDestroy();
	
	test.Next(_L("CPersistentStore DoSetRootL"));
	CPersistentStoreHelper* ps = new (ELeave) CPersistentStoreHelper();
	CleanupStack::PushL(ps);
	ps->SetRootL(KNullStreamId);
	CleanupStack::PopAndDestroy();
	
	test.Next(_L("HDirectStoreBuf::DoSeekL calls"));
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
	test(pos.Offset() == 5);
	rts.Close();
	rstream.Close();
	CleanupStack::PopAndDestroy(2);
	
	test.Next(_L("Calling TEmpty Constructor"));
	TPagedSetToken set2(TBtreeToken::EEmpty);
	test( set2.Count() == 0);
	
	test.Next(_L("Set function"));
	set.Set(set2);
	const TPagedSetToken& pst = set.Token();
	test(pst.Count() == set2.Count());
	
	test.End();
	CleanupStack::PopAndDestroy();
	}

LOCAL_C void doMainL()
	{
	test.Start(_L("Basic operations"));
	test1L();
	test.Next(_L("Large set TUint32"));
	test2L();
	test.Next(_L("Tokens and streaming"));
	test3L();
	test.Next(_L("Forgotten API"));
	test4L();
	test.End();
	}

LOCAL_C void setupCleanup()
//
// Initialise the cleanup stack.
//
    {
	TheTrapCleanup=CTrapCleanup::New();
	test(TheTrapCleanup!=NULL);
	TRAPD(r,\
		{\
		for (TInt i=KTestCleanupStack;i>0;i--)\
			CleanupStack::PushL((TAny*)1);\
		test(r==KErrNone);\
		CleanupStack::Pop(KTestCleanupStack);\
		});
	test(r==KErrNone);
	}

GLDEF_C TInt E32Main()
	{
	test.Title();
	setupCleanup();
	__UHEAP_MARK;
//
	TRAPD(r,doMainL());
	test(r==KErrNone);
//
	__UHEAP_MARKEND;
	delete TheTrapCleanup;
	test.Close();
	return 0;
	}

