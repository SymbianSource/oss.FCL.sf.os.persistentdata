// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// EventLogger - deleting EventLogger test files from C:
// Please, ensure that t_logenvdestroy test is executed after the EventLogger component tests
// 
//

#include <e32test.h>
#include <bautils.h>

RTest TheTest(_L("t_logenvdestroy - deleting EventLogger test files from C:"));

_LIT(KCFileName1, "c:\\private\\10003a73\\CntModel.ini");
_LIT(KCFileName2, "c:\\private\\10003a73\\SQLite__Contacts.cdb");

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
static void Check(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		RDebug::Print(_L("*** Expected error: %d, got: %d\r\n"), aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////

void DoDeleteFile(RFs& aFs, const TDesC& aFilePath)
	{
	TInt err = BaflUtils::DeleteFile(aFs, aFilePath);
	if(err != KErrNone && err != KErrNotFound)
		{
		TheTest.Printf(_L("Error %d deleting \"%S\" file.\n"), err, &aFilePath);
		}
	}

void DoRun()
	{
    RFs fs;
	TInt err = fs.Connect();
	TEST2(err, KErrNone);

	TheTest.Start(_L("  @SYMTestCaseID: PDS-LOGENG-CT-4049 Deleting EventLogger test files from C:"));

	DoDeleteFile(fs, KCFileName1);
	DoDeleteFile(fs, KCFileName2);

	fs.Close();
	}

TInt E32Main()
    {
	TheTest.Title();

	CTrapCleanup* tc = CTrapCleanup::New();

	__UHEAP_MARK;

	DoRun();

	__UHEAP_MARKEND;

	TheTest.End();
	TheTest.Close();

	delete tc;

	User::Heap().Check();
	return KErrNone;
    }
