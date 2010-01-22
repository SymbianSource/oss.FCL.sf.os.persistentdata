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
// EventLogger - copying EventLogger test files from Z: to C:
// Please, ensure that t_logenvcreate test is executed before the other EventLogger component tests
// 
//

#include <e32test.h>
#include <bautils.h>

RTest TheTest(_L("t_logenvcreate - copying EventLogger test files to C:"));

_LIT(KContactsServerName, "!CNTSRV");

_LIT(KContactsServerPrivateDirC, "c:\\private\\10003a73\\");

_LIT(KZFileName1, "z:\\private\\101f401d\\CntModel.ini");
_LIT(KZFileName2, "z:\\private\\101f401d\\SQLite__Contacts.cdb");

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

static void KillProcess(const TDesC& aProcessName)
    {
    TFullName name;
    TheTest.Printf(_L(":: Find and kill \"%S\" process.\n"), &aProcessName);
    TBuf<64> pattern(aProcessName);
    TInt length = pattern.Length();
    pattern += _L("*");
    TFindProcess procFinder(pattern);

    while (procFinder.Next(name) == KErrNone)
        {
        if (name.Length() > length)
            {//If found name is a string containing aProcessName string.
            TChar c(name[length]);
            if (c.IsAlphaDigit() || c == TChar('_') || c == TChar('-'))
                {
                // If the found name is other valid application name
                // starting with aProcessName string.
                TheTest.Printf(_L(":: Process name: \"%S\".\n"), &name);
                continue;
                }
            }
        RProcess proc;
        if (proc.Open(name) == KErrNone)
            {
            proc.Kill(0);
            TheTest.Printf(_L(":: \"%S\" process killed.\n"), &name);
            }
        proc.Close();
        }
    }

void DoRun()
	{
    RFs fs;
	TInt err = fs.Connect();
	TEST2(err, KErrNone);

	TheTest.Start(_L(" @SYMTestCaseID: PDS-LOGENG-CT-4048 Copy EventLogger test files from Z: to C: "));

	err = BaflUtils::CopyFile(fs, KZFileName1, KCFileName1);
	TEST2(err, KErrNone);
	err = fs.SetAtt(KCFileName1, 0, KEntryAttReadOnly);
	TEST2(err, KErrNone);

	err = BaflUtils::CopyFile(fs, KZFileName2, KCFileName2);
	TEST2(err, KErrNone);
	err = fs.SetAtt(KCFileName2, 0, KEntryAttReadOnly);
	TEST2(err, KErrNone);
	fs.Close();
	}

void CreateContactsServerPrivateDir()
    {
    RFs fs;
	TInt err = fs.Connect();
	TEST2(err, KErrNone);

	TRAP(err, BaflUtils::EnsurePathExistsL(fs, KContactsServerPrivateDirC));
	TEST2(err, KErrNone);

	fs.Close();
	}

TInt E32Main()
    {
	TheTest.Title();

	CTrapCleanup* tc = CTrapCleanup::New();

	__UHEAP_MARK;

	KillProcess(KContactsServerName);
	
	CreateContactsServerPrivateDir();

	DoRun();

	__UHEAP_MARKEND;

	TheTest.End();
	TheTest.Close();

	delete tc;

	User::Heap().Check();
	return KErrNone;
    }
