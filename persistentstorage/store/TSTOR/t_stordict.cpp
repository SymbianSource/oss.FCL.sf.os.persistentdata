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

#if !defined(__E32TEST_H__)
#include <e32test.h>
#endif

#include <s32mem.h>

//
// CTestStreamDictionary
//

class CTestStreamDictionary : public CStreamDictionary
	{
public:
	struct TEntry {TUid uid;TStreamId id;};
public:
	static CTestStreamDictionary* NewL();
	CTestStreamDictionary();
//*	TUid Uid(TInt aInt) { return (*iEntryList)[aInt].iUid; }
//*	TStreamId StreamId(TInt aInt) { return (*iEntryList)[aInt].iStreamId; }
//*	TInt Count() { return iEntryList->Count(); }
	TUid Uid(TInt aInt) { return (*iCheat)[aInt].uid; }
	TStreamId StreamId(TInt aInt) { return (*iCheat)[aInt].id; }
	TInt Count() { return iCheat->Count(); }
private:
	CArrayFixSeg<TEntry>* iCheat;
	};

CTestStreamDictionary* CTestStreamDictionary::NewL()
	{
	CTestStreamDictionary* thing=new(ELeave) CTestStreamDictionary();
//*	CleanupStack::PushL(thing);
//*	thing->ConstructL();
//*	CleanupStack::Pop();
	return thing;
	}

CTestStreamDictionary::CTestStreamDictionary()
	: iCheat((CArrayFixSeg<TEntry>*)&iCheat-1) // modification to get to count etc
	{}


//
// Test code
//

const TInt KTestCleanupStack=0x40;
const TInt KTestExpandSize=0x20;

LOCAL_D RTest test(_L("t_stordict"));
LOCAL_D CTrapCleanup* TheTrapCleanup;

// some uid-stream pairs to use for testing
const TUid testUid1={1};
LOCAL_D TStreamId testStreamId1=TStreamId(1);
//
const TUid testUid2={57};
LOCAL_D TStreamId testStreamId2=TStreamId(57);
//
const TUid testUid3={99999};
LOCAL_D TStreamId testStreamId3=TStreamId(425);
//

/**
@SYMTestCaseID          SYSLIB-STORE-CT-1201
@SYMTestCaseDesc	    Tests for copy operations on dictionary files
@SYMTestPriority 	    High
@SYMTestActions  	    Attempt for copying two classes using memory based streams.
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
template <class T1,class T2>
void testCopyL(T1& aCopy,const T2& anOriginal)
	{
	test.Next(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1201 "));
	CBufSeg* buf=0;
	TRAPD(r,buf=CBufSeg::NewL(KTestExpandSize));
	if (r!=KErrNone)
		test.Panic(_L("Allocating buffer"));
//
// Write anOriginal out to the buffer.
//
	RBufWriteStream out;
	out.Append(*buf);
	TRAP(r,out<<anOriginal);
	test(r==KErrNone);
	TRAP(r,out.CommitL());
	if (r!=KErrNone)
		test.Panic(_L("Committing write stream"));
//
// Read anOriginal in from the buffer.
//
	RBufReadStream in(*buf);
	TRAP(r,in>>aCopy);
	test(r==KErrNone);
//
// See if it's consumed the lot.
//
	TUint8 b;
	test(in.Source()->ReadL(&b,1)==0);
//
	delete buf;
	}

/**
@SYMTestCaseID          SYSLIB-STORE-CT-1202
@SYMTestCaseDesc	    Tests if two dictionary files are equal
@SYMTestPriority 	    High
@SYMTestActions  	    Tests if count of entries,UID and streamID's are equal
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
void testIsEqual(CTestStreamDictionary* aCopy,CTestStreamDictionary* aOrig)
	{
	test.Next(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1202 "));
	TInt origCount=aOrig->Count();
	test(origCount==aCopy->Count());
	//
	for (TInt i=0 ; i<origCount ; i++)
		{
		test(aOrig->Uid(i)==aCopy->Uid(i));
		test(aOrig->StreamId(i)==aCopy->StreamId(i));
		}
	}

/**
@SYMTestCaseID          SYSLIB-STORE-CT-1203
@SYMTestCaseDesc	    Tests for simple operations on a dictionary file
@SYMTestPriority 	    High
@SYMTestActions  	    Tests for assign,re-assigning,removing entries from the file
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
LOCAL_C void simpleTestsL()
	{
	CTestStreamDictionary* dic=CTestStreamDictionary::NewL();
	// attempt finding and removing with an empty dictionary
	test.Next(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1203 Manipulating an empty dictionary "));
	test(dic->Count()==0);
	test(dic->At(testUid1)==KNullStreamId);
	dic->Remove(testUid1);
	test(dic->Count()==0);
	test(dic->IsNull());
	//
	// assign an entry
	test.Next(_L("Assigning entries and manipulating them"));
	TRAPD(ret,dic->AssignL(testUid1,testStreamId1));
	test(ret==KErrNone);
	test(dic->Count()==1);
	test(!dic->IsNull());
	test(dic->At(testUid1)==testStreamId1);
	//
	// assign another entry
	TRAP(ret,dic->AssignL(testUid2,testStreamId2));
	test(ret==KErrNone);
	test(dic->Count()==2);
	test(dic->At(testUid2)==testStreamId2);
	//
	// re-assign uid1
	TRAP(ret,dic->AssignL(testUid1,testStreamId3));
	test(ret==KErrNone);
	test(dic->Count()==2);
	test(dic->At(testUid1)==testStreamId3);
	//
	// test finding and removing a non-existant entry from a non-empty dictionary
	test(dic->At(testUid3)==KNullStreamId);
	dic->Remove(testUid3);
	test(dic->Count()==2);
	//
	// test removing an entry
	dic->Remove(testUid1);
	test(dic->Count()==1);
	test(dic->At(testUid1)==KNullStreamId);
	test(dic->At(testUid2)==testStreamId2);
	test(!dic->IsNull());
	//
	// test removing the other entry
	dic->Remove(testUid2);
	test(dic->Count()==0);
	test(dic->IsNull());
	test(dic->At(testUid1)==KNullStreamId);
	test(dic->At(testUid2)==KNullStreamId);
	//
	delete dic;
	}

/**
@SYMTestCaseID          SYSLIB-STORE-CT-1204
@SYMTestCaseDesc	    Streaming dictionary files tests
@SYMTestPriority 	    High
@SYMTestActions  	    Tests for copying an empty dictionary and dictionary containing different sets of entries
                        Tests for equality of two dictionary files and test the copied file.
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
LOCAL_C void streamingTestsL()
	{
	CTestStreamDictionary* orig=CTestStreamDictionary::NewL();
	CTestStreamDictionary* copy=CTestStreamDictionary::NewL();
	//
	// copy an empty dictionary
	test.Next(_L(" @SYMTestCaseID:SYSLIB-STORE-CT-1204 Streaming an empty dictionary "));
	test(orig->IsNull());
	testCopyL(*copy,*orig);
	test(copy->IsNull());
	//
	// copy a dictionary containing a range of entries
	test.Next(_L("Streaming a dictionary containing entries"));
	TRAPD(ret,orig->AssignL(testUid1,testStreamId1));
	TRAP(ret,orig->AssignL(testUid2,testStreamId2));
	TRAP(ret,orig->AssignL(testUid3,testStreamId3));
	testCopyL(*copy,*orig);
	testIsEqual(copy,orig);
	test(!copy->IsNull());
	//
	delete orig;
	delete copy;
	}


//
// Initialise the cleanup stack.
//
LOCAL_C void setupCleanup()
    {
	TheTrapCleanup=CTrapCleanup::New();
	TRAPD(r,\
		{\
		for (TInt i=KTestCleanupStack;i>0;i--)\
			CleanupStack::PushL((TAny*)1);\
		test(r==KErrNone);\
		CleanupStack::Pop(KTestCleanupStack);\
		});
	}


GLDEF_C TInt E32Main()
	{
	setupCleanup();
	//
	test.Title();
	test.Start(_L("Testing CStreamDictionary..."));
	//
	// run the testcode (inside an alloc heaven harness)
	__UHEAP_MARK;

	TRAPD(r,simpleTestsL());
		test(r==KErrNone);

	TRAP(r,streamingTestsL());
		test(r==KErrNone);

	__UHEAP_MARKEND;

 	test.End();
	test.Close();

	delete TheTrapCleanup;
	return KErrNone;
	}

