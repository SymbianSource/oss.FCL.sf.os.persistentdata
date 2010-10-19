// Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <e32test.h>
#include <bautils.h>
#include <sqldb.h>
#include <hal.h>
#include "t_sqlcmdlineutil.h"

///////////////////////////////////////////////////////////////////////////////////////

RTest			TheTest(_L("t_sqlperformance2 test"));
RSqlDatabase 	TheDb;
TFileName		TheDbFileName;
RFs				TheFs;

TBuf<200> 		TheTestTitle;
TCmdLineParams 	TheCmdLineParams;
TBuf8<200> 		TheSqlConfigString;

TBuf<250> 		TheLogLine;
TBuf8<250> 		TheLogLine8;
RFile 			TheLogFile; 

_LIT(KUtf8,  "UTF8 ");
_LIT(KUtf16, "UTF16");

TInt TheBlobSize = 1024 * 256;
TInt TheDbSize1, TheDbSize2;
TUint32 TheStartTicks, TheEndTicks;

///////////////////////////////////////////////////////////////////////////////////////

void TestEnvDestroy()
	{
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		(void)TheLogFile.Flush();
		TheLogFile.Close();
		}
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	TheFs.Close();
	}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check1(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		TestEnvDestroy();
		TheTest.Printf(_L("*** Line %d\r\n"), aLine);
		TheTest(EFalse, aLine);
		}
	}
void Check2(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		TestEnvDestroy();
		TheTest.Printf(_L("*** Line %d, Expected error: %d, got: %d\r\n"), aLine, aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST(arg) ::Check1((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check2(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////

void TestEnvInit()
    {
    TInt err = TheFs.Connect();
    TEST2(err, KErrNone);
    //
    TInt driveNumber = -1;
	err = RFs::CharToDrive(TheCmdLineParams.iDriveName[0], driveNumber);
	TEST2(err, KErrNone);
	TDriveNumber driveNo = static_cast <TDriveNumber> (driveNumber);
	TDriveInfo driveInfo;
	err = TheFs.Drive(driveInfo, driveNo);
	TEST2(err, KErrNone);
    //Create the test directory
	err = TheFs.MkDirAll(TheDbFileName);
	TEST(err == KErrNone || err == KErrAlreadyExists);
    //Print drive info and the database name 
	_LIT(KType1, "Not present");
	_LIT(KType2, "Unknown");
	_LIT(KType3, "Floppy");
	_LIT(KType4, "Hard disk");
	_LIT(KType5, "CD ROM");
	_LIT(KType6, "RAM disk");
	_LIT(KType7, "Flash");
	_LIT(KType8, "ROM drive");
	_LIT(KType9, "Remote drive");
	_LIT(KType10,"NAND flash");
	_LIT(KType11,"Rotating media");
	TPtrC KMediaTypeNames[] = {KType1(), KType2(), KType3(), KType4(), KType5(), KType6(), KType7(), KType8(), KType9(), KType10(), KType11()};
	TheTest.Printf(_L("Drive %C: %S. File: \"%S\"\r\n"), 'A' + driveNo, &KMediaTypeNames[driveInfo.iType], &TheDbFileName);
	
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		err = TheLogFile.Replace(TheFs, TheCmdLineParams.iLogFileName, EFileRead | EFileWrite);
		TEST2(err, KErrNone);
		LogConfig(TheLogFile, TheCmdLineParams);
		}
    }
	
//Prints the test case title and execution time in microseconds
void PrintWriteStats()
	{
	static TInt freq = 0;
	if(freq == 0)
		{
		TEST2(HAL::Get(HAL::EFastCounterFrequency, freq), KErrNone);
		}
	TInt64 diffTicks = (TInt64)TheEndTicks - (TInt64)TheStartTicks;
	if(diffTicks < 0)
		{
		diffTicks = KMaxTUint32 + diffTicks + 1;
		}
	const TInt KMicroSecIn1Sec = 1000000;
	TInt32 us = (diffTicks * KMicroSecIn1Sec) / freq;
	TheTest.Printf(_L("%S, blob: %d Kb, db size before: %d Kb, db size after: %d Kb, %d us\r\n"), 
		&TheTestTitle, TheBlobSize / 1024, TheDbSize1 / 1024, TheDbSize2 / 1024, us);
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		TheLogLine.Format(_L("%S, blob: %d Kb, db size before: %d Kb, db size after: %d Kb¬%d¬us\r\n"), 
			&TheTestTitle, TheBlobSize / 1024, TheDbSize1 / 1024, TheDbSize2 / 1024, us);
		TheLogLine8.Copy(TheLogLine);
		(void)TheLogFile.Write(TheLogLine8);
		}
	}

//Prints the test case title and execution time in microseconds
void PrintReadStats()
	{
	static TInt freq = 0;
	if(freq == 0)
		{
		TEST2(HAL::Get(HAL::EFastCounterFrequency, freq), KErrNone);
		}
	TInt64 diffTicks = (TInt64)TheEndTicks - (TInt64)TheStartTicks;
	if(diffTicks < 0)
		{
		diffTicks = KMaxTUint32 + diffTicks + 1;
		}
	const TInt KMicroSecIn1Sec = 1000000;
	TInt32 us = (diffTicks * KMicroSecIn1Sec) / freq;
	TheTest.Printf(_L("%S, blob: %d Kb, %d us\r\n"), &TheTestTitle, TheBlobSize / 1024, us);
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		TheLogLine.Format(_L("%S, blob: %d Kb¬%d¬us\r\n"), &TheTestTitle, TheBlobSize / 1024, us);
		TheLogLine8.Copy(TheLogLine);
		(void)TheLogFile.Write(TheLogLine8);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////
	
void CreateTestDb()
	{
	(void)RSqlDatabase::Delete(TheDbFileName);
	TInt err = TheDb.Create(TheDbFileName, &TheSqlConfigString);
	TEST2(err, KErrNone);
	err = TheDb.Exec(_L8("CREATE TABLE A(B BLOB)"));
	TEST2(err, 1);
	}

void DoWriteBlobIncrL(const TDesC8& aData)
	{
	RSqlBlobWriteStream strm;
	CleanupClosePushL(strm);
	
	strm.OpenL(TheDb, _L("A"), _L("B"));	
	strm.WriteL(aData);
	
	strm.CommitL();
	
	CleanupStack::PopAndDestroy(&strm);
	}
	
void InsertZeroBlob()
	{
	TBuf<100> sql;
	sql.Format(_L("INSERT INTO A VALUES(zeroblob(%d))"), TheBlobSize);
	
	TheStartTicks = User::FastCounter();
	TInt err = TheDb.Exec(sql);
	TheEndTicks = User::FastCounter();
	
	TEST2(err, 1);
	}
	
void InsertRealBlob()
	{
	HBufC8* data = HBufC8::New(TheBlobSize);
	TEST(data != NULL);
	TPtr8 dataptr = data->Des();
	dataptr.SetLength(TheBlobSize);
	dataptr.Fill(TChar('A'));
		
	RSqlStatement stmt;
	TInt err = stmt.Prepare(TheDb, _L8("INSERT INTO A VALUES(:Prm)"));
	TEST2(err, KErrNone);

	RSqlParamWriteStream strm;
	err = strm.BindBinary(stmt, 0);
	TEST2(err, KErrNone);
	
	TRAP(err, strm.WriteL(dataptr));
	TEST2(err, KErrNone);
	TRAP(err, strm.CommitL());
	TEST2(err, KErrNone);
	strm.Close();

	err = stmt.Exec();	
	TEST2(err, 1);			
	stmt.Close();	
	
	delete data;
	}
	
void InsertBlobIncr()
	{
	HBufC8* data = HBufC8::New(TheBlobSize);
	TEST(data != NULL);
	TPtr8 dataptr = data->Des();
	dataptr.SetLength(TheBlobSize);
	dataptr.Fill(TChar('B'));
	
	TBuf<100> sql;
	sql.Format(_L("INSERT INTO A VALUES(zeroblob(%d))"), TheBlobSize);

	TheStartTicks = User::FastCounter();
	
	TInt err = TheDb.Exec(_L8("BEGIN"));
	TEST(err >= 0);
	
	err = TheDb.Exec(sql);
	TEST2(err, 1);
	
	TRAP(err, DoWriteBlobIncrL(dataptr));
	TEST2(err, KErrNone);
	
	err = TheDb.Exec(_L8("COMMIT"));
	TEST(err >= 0);
		
	TheEndTicks = User::FastCounter();
	
	delete data;
	}
	
void InsertBlobExec()
	{
	HBufC8* buf = HBufC8::New(TheBlobSize * 2 + 100);
	TEST(buf != NULL);
	TPtr8 sql = buf->Des();
	_LIT8(KStr, "INSERT INTO A VALUES(x'");
	sql.SetLength(TheBlobSize * 2 + KStr().Length());
	sql.Fill(TChar('A'));
	sql.Replace(0, KStr().Length(), KStr);
	sql.Append(_L8("')"));

	TheStartTicks = User::FastCounter();
	
	TInt err = TheDb.Exec(sql);
	TEST2(err, 1);

	TheEndTicks = User::FastCounter();
	
	delete buf;	
	}

void InsertBlobBindStreamPrm()
	{
	HBufC8* data = HBufC8::New(TheBlobSize);
	TEST(data != NULL);
	TPtr8 dataptr = data->Des();
	dataptr.SetLength(TheBlobSize);
	dataptr.Fill(TChar('A'));
	
	TheStartTicks = User::FastCounter();
				
	RSqlStatement stmt;
	TInt err = stmt.Prepare(TheDb, _L8("INSERT INTO A VALUES(:Prm)"));
	TEST2(err, KErrNone);

	RSqlParamWriteStream strm;
	err = strm.BindBinary(stmt, 0);
	TEST2(err, KErrNone);
	
	TRAP(err, strm.WriteL(dataptr));
	TEST2(err, KErrNone);
	
	TRAP(err, strm.CommitL());
	TEST2(err, KErrNone);
	
	err = stmt.Exec();	
	TEST2(err, 1);
	
	strm.Close();
	stmt.Close();	
	
	TheEndTicks = User::FastCounter();
					
	delete data;	
	}
	
void UpdateBlobIncr()
	{
	HBufC8* data = HBufC8::New(TheBlobSize);
	TEST(data != NULL);
	TPtr8 dataptr = data->Des();
	dataptr.SetLength(TheBlobSize);
	dataptr.Fill(TChar('A'));
	
	TheStartTicks = User::FastCounter();
	
	TInt err = TheDb.Exec(_L8("BEGIN"));
	TEST(err >= 0);
		
	TRAP(err, DoWriteBlobIncrL(dataptr));
	TEST2(err, KErrNone);
	
	err = TheDb.Exec(_L8("COMMIT"));
	TEST(err >= 0);
			
	TheEndTicks = User::FastCounter();
	
	delete data;
	}
	
void UpdateBlobExec()
	{
	HBufC8* buf = HBufC8::New(TheBlobSize * 2 + 100);
	TEST(buf != NULL);
	TPtr8 sql = buf->Des();
	_LIT8(KStr, "UPDATE A SET B=x'");
	sql.SetLength(TheBlobSize * 2 + KStr().Length());
	sql.Fill(TChar('B'));
	sql.Replace(0, KStr().Length(), KStr);
	sql.Append(_L8("'"));

	TheStartTicks = User::FastCounter();
	TInt err = TheDb.Exec(sql);	
	TEST2(err, 1);
	TheEndTicks = User::FastCounter();

	delete buf;	
	}

void UpdateBlobBindStreamPrm()
	{
	HBufC8* data = HBufC8::New(TheBlobSize);
	TEST(data != NULL);
	TPtr8 dataptr = data->Des();
	dataptr.SetLength(TheBlobSize);
	dataptr.Fill(TChar('B'));
	
	TheStartTicks = User::FastCounter();
				
	RSqlStatement stmt;
	TInt err = stmt.Prepare(TheDb, _L8("UPDATE A SET B=(:Prm)"));
	TEST2(err, KErrNone);

	RSqlParamWriteStream strm;
	err = strm.BindBinary(stmt, 0);
	TEST2(err, KErrNone);
	
	TRAP(err, strm.WriteL(dataptr));
	TEST2(err, KErrNone);
	
	TRAP(err, strm.CommitL());
	TEST2(err, KErrNone);
	
	err = stmt.Exec();	
	TEST2(err, 1);

	strm.Close();		
	stmt.Close();	

	TheEndTicks = User::FastCounter();
	
	delete data;	
	}

/**
@SYMTestCaseID			SYSLIB-SQL-UT-4084
@SYMTestCaseDesc		SQL, BLOB write, performance tests.
						Tests insertion and updates of BLOBs using the old
						APIs and the new RSqlBlobWriteStream APIs.
@SYMTestPriority		Medium
@SYMTestActions			Insertion and updates of blobs using the old and new APIs.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ5912
*/
void BlobWriteTest()
	{	
	//Insert a blob
	
	CreateTestDb();
	TheTestTitle.Copy(_L("INSERT zeroblob - RSqlDatabase::Exec()"));
	TheDbSize1 = TheDb.Size();
	InsertZeroBlob();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
				
	CreateTestDb();
	TheTestTitle.Copy(_L("INSERT blob - RSqlParamWriteStream"));
	TheDbSize1 = TheDb.Size();
	InsertBlobBindStreamPrm();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
	
	CreateTestDb();
	TheTestTitle.Copy(_L("INSERT blob - RSqlDatabase::Exec()"));
	TheDbSize1 = TheDb.Size();
	InsertBlobExec();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
	
	CreateTestDb();
	TheTestTitle.Copy(_L("INSERT blob - RSqlBlobWriteStream + transaction"));
	TheDbSize1 = TheDb.Size();
	InsertBlobIncr();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
						
	// Update a blob 
		
	CreateTestDb();
	TheTestTitle.Copy(_L("UPDATE zeroblob - RSqlParamWriteStream"));
	TheDbSize1 = TheDb.Size();
	InsertZeroBlob();
	UpdateBlobBindStreamPrm();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
		
	CreateTestDb();
	TheTestTitle.Copy(_L("UPDATE blob - RSqlParamWriteStream"));
	TheDbSize1 = TheDb.Size();
	InsertRealBlob();
	UpdateBlobBindStreamPrm();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
			
	CreateTestDb();
	TheTestTitle.Copy(_L("UPDATE zeroblob - RSqlDatabase::Exec()"));
	TheDbSize1 = TheDb.Size();
	InsertZeroBlob();
	UpdateBlobExec();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
		
	CreateTestDb();
	TheTestTitle.Copy(_L("UPDATE blob - RSqlDatabase::Exec()"));
	TheDbSize1 = TheDb.Size();
	InsertRealBlob();
	UpdateBlobExec();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
		
	CreateTestDb();
	TheTestTitle.Copy(_L("UPDATE zeroblob - RSqlBlobWriteStream + transaction"));
	TheDbSize1 = TheDb.Size();
	InsertZeroBlob();
	UpdateBlobIncr();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	PrintWriteStats();
		
	CreateTestDb();
	TheTestTitle.Copy(_L("UPDATE blob - RSqlBlobWriteStream + transaction"));
	TheDbSize1 = TheDb.Size();
	InsertRealBlob();
	UpdateBlobIncr();
	TheDbSize2 = TheDb.Size();
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);	
	PrintWriteStats();
	}

void DoReadBlobIncrL(TDes8& aDes, TInt aMaxLength)
	{
	TheStartTicks = User::FastCounter();
	
	RSqlBlobReadStream strm;
	CleanupClosePushL(strm);
	strm.OpenL(TheDb, _L("A"), _L("B"), 1);
	strm.ReadL(aDes, aMaxLength);
	CleanupStack::PopAndDestroy(&strm);

	TheEndTicks = User::FastCounter();
	}

void ReadBlobIncr()
	{
	HBufC8* data = HBufC8::New(TheBlobSize);
	TEST(data != NULL);
	TPtr8 dataptr = data->Des();
	
	TRAPD(err, DoReadBlobIncrL(dataptr, TheBlobSize));
	TEST2(err, KErrNone);
	TEST2(dataptr.Length(), TheBlobSize);
	
	delete data;
	}

void ReadBlobColDes()
	{
	HBufC8* data = HBufC8::New(TheBlobSize);
	TEST(data != NULL);
	TPtr8 dataptr = data->Des();
	
	TheStartTicks = User::FastCounter();
	
	RSqlStatement stmt;
	TInt err = stmt.Prepare(TheDb, _L8("SELECT B FROM A WHERE ROWID=1"));
	TEST2(err, KErrNone);

	err = stmt.Next();
	TEST2(err, KSqlAtRow);
	
	err = stmt.ColumnBinary(0, dataptr);
	TEST2(err, KErrNone);
	TEST2(dataptr.Length(), TheBlobSize);
	stmt.Close();

	TheEndTicks = User::FastCounter();
	
	delete data;
	}

void ReadBlobColPtr()
	{
	TheStartTicks = User::FastCounter();
	
	RSqlStatement stmt;
	TInt err = stmt.Prepare(TheDb, _L8("SELECT B FROM A WHERE ROWID=1"));
	TEST2(err, KErrNone);
	
	err = stmt.Next();
	TEST2(err, KSqlAtRow);
	
	TPtrC8 data;
	err = stmt.ColumnBinary(0, data);
	TEST2(err, KErrNone);
	TEST2(data.Length(), TheBlobSize);

	stmt.Close();

	TheEndTicks = User::FastCounter();
	}

void ReadBlobStreamCol()
	{
	HBufC8* data = HBufC8::New(TheBlobSize);
	TEST(data != NULL);
	TPtr8 dataptr = data->Des();
	
	TheStartTicks = User::FastCounter();
	
	RSqlStatement stmt;
	TInt err = stmt.Prepare(TheDb, _L8("SELECT B FROM A WHERE ROWID=1"));
	TEST2(err, KErrNone);

	err = stmt.Next();
	TEST2(err, KSqlAtRow);

	RSqlColumnReadStream strm;
	err = strm.ColumnBinary(stmt, 0);
	TEST2(err, KErrNone);
	TRAP(err, strm.ReadL(dataptr, TheBlobSize));
	TEST2(err, KErrNone);
	TEST2(dataptr.Length(), TheBlobSize);

	strm.Close();
	stmt.Close();
	
	TheEndTicks = User::FastCounter();
		
	delete data;
	}
	
/**
@SYMTestCaseID			SYSLIB-SQL-UT-4085
@SYMTestCaseDesc		SQL, BLOB read, performance tests.
						Tests retrieval of BLOBs using the old
						APIs and the new RSqlBlobReadStream APIs.
@SYMTestPriority		Medium
@SYMTestActions			Retrieval of blobs using the old and new APIs.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ5912
*/
void BlobReadTest()
	{
	// Insert a blob
	CreateTestDb();
	InsertBlobExec();
	TheDb.Close();
		
	// Read the blob
		
	TheTestTitle.Copy(_L("Read blob - RSqlBlobReadStream"));
	TInt err = TheDb.Open(TheDbFileName);
	TEST2(err, KErrNone);
	ReadBlobIncr();
	TheDb.Close();
	PrintReadStats();

	TheTestTitle.Copy(_L("Read blob - RSqlStatement::ColumnBinary(TInt, TDes8&)"));
	err = TheDb.Open(TheDbFileName);
	TEST2(err, KErrNone);
	ReadBlobColDes();
	TheDb.Close();
	PrintReadStats();
		
	TheTestTitle.Copy(_L("Read blob - RSqlStatement::ColumnBinary(TInt, TPtrC8&)"));
	err = TheDb.Open(TheDbFileName);
	TEST2(err, KErrNone);
	ReadBlobColPtr();
	TheDb.Close();
	PrintReadStats();
		
	TheTestTitle.Copy(_L("Read blob - RSqlColumnReadStream"));
	err = TheDb.Open(TheDbFileName);
	TEST2(err, KErrNone);
	ReadBlobStreamCol();
	TheDb.Close();
	PrintReadStats();
		
	(void)RSqlDatabase::Delete(TheDbFileName);
	}
	
/**
@SYMTestCaseID			SYSLIB-SQL-UT-4115
@SYMTestCaseDesc		SQL, sequential BLOB writes, performance tests.
						Tests sequentially writing 32Kb blocks to a 1.125Mb blob
						using the new RSqlBlobWriteStream APIs to examine
					    the write performance at different stages in the 
						sequence.
@SYMTestPriority		Medium
@SYMTestActions			Sequential writing of a blob using the new RSqlBlobWriteStream APIs.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ5912
*/
void SequentialWriteTestL()
	{
	const TInt KBufLen = 32768; // 32Kb
	HBufC8* buf = HBufC8::NewL(KBufLen);
	TPtr8 dataPtr =	buf->Des();
	dataPtr.SetLength(KBufLen);
	dataPtr.Fill('A', KBufLen);	
	
	TheTestTitle.Copy(_L("Sequential BLOB writes"));
	
	CreateTestDb();
	InsertZeroBlob(); // insert zeroblob of "TheBlobSize" size
		
	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
	
	RSqlBlobWriteStream strm;
	strm.OpenL(TheDb, _L("A"), _L("B"));	
								
	// Sequentially write 32Kb blocks of data to the 
	// blob until the 1Mb cache is full and writes to the disk begin.
	// 32 * 32Kb = 1Mb = soft heap limit
	const TInt KItCount = TheBlobSize / KBufLen - 1;
	for(TInt i = 1; i <= KItCount; ++i)
		{
		strm.WriteL(dataPtr);
		}	
	strm.CommitL();
	strm.Close();
	
	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();
	
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	
	PrintWriteStats();
		
	delete buf;
	}
	
/**
@SYMTestCaseID			SYSLIB-SQL-UT-4116
@SYMTestCaseDesc		SQL, transaction sequential BLOB writes, performance tests.
						Tests sequentially writing 32Kb blocks to a 1.125Mb blob
						within a transaction, using the new RSqlBlobWriteStream APIs,
						to examine the write performance at different stages in the 
						sequence.
@SYMTestPriority		Medium
@SYMTestActions			Sequential writing of a blob within a transactions, using the 
						new RSqlBlobWriteStream APIs.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ5912
*/
void TransSequentialWriteTestL()
	{
	const TInt KBufLen = 32768; // 32Kb
	HBufC8* buf = HBufC8::NewL(KBufLen);
	TPtr8 dataPtr =	buf->Des();
	dataPtr.SetLength(KBufLen);
	dataPtr.Fill('A', KBufLen);	

	TheTestTitle.Copy(_L("Sequential BLOB writes in transaction"));
	
	CreateTestDb();
	InsertZeroBlob(); // insert zeroblob of "TheBlobSize" size
		
	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
	
	RSqlBlobWriteStream strm;
	strm.OpenL(TheDb, _L("A"), _L("B"));
	
	TInt err = TheDb.Exec(_L8("BEGIN"));
	TEST(err >= 0);	
								
	// Sequentially write 32Kb blocks of data to the 
	// blob until the 1Mb cache is full and writes to the disk begin.
	// 32 * 32Kb = 1Mb = soft heap limit
	const TInt KItCount = TheBlobSize / KBufLen - 1;
	for(TInt i = 1; i <= KItCount; ++i)
		{
		strm.WriteL(dataPtr);
		}	
	strm.CommitL();
	err = TheDb.Exec(_L8("COMMIT"));
	TEST(err >= 0);
	strm.Close();

	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();
	
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);

	PrintWriteStats();
	
	delete buf;
	}

/**
@SYMTestCaseID			SYSLIB-SQL-UT-4117
@SYMTestCaseDesc		SQL, whole BLOB write, performance tests.
						Tests writing a 256Kb data block to a 256Kb blob to examine the
						relative performance of the TSqlBlob and RSqlBlobWriteStream APIs.
@SYMTestPriority		Medium
@SYMTestActions			Whole update of a blob using the new TSqlBlob and RSqlBlobWriteStream APIs.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ5912
*/
void WholeWriteTestL()
	{
	TInt bufLen = TheBlobSize; 
	HBufC8* buf = HBufC8::NewL(bufLen);
	TPtr8 dataPtr =	buf->Des();
	dataPtr.SetLength(bufLen);
	dataPtr.Fill('Z', bufLen);	

	TheTestTitle.Copy(_L("Whole BLOB write - TSqlBlob::Set()"));
	
	CreateTestDb();
	InsertRealBlob(); // insert blob of "TheBlobSize" size
			
	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
	
	TSqlBlob::SetL(TheDb, _L("A"), _L("B"), dataPtr);	
		
	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();

	PrintWriteStats();
	
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);		

	// to avoid caching issues, close and re-create the database for the next part
	
	TheTestTitle.Copy(_L("Whole BLOB write - RSqlBlobWriteStream::WriteL()"));
	
	CreateTestDb();
	InsertRealBlob(); // insert blob of "TheBlobSize" size

	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
	
	RSqlBlobWriteStream strm;
	CleanupClosePushL(strm);
	strm.OpenL(TheDb, _L("A"), _L("B"));	
	strm.WriteL(dataPtr);
	CleanupStack::PopAndDestroy(&strm);
		
	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();
	PrintWriteStats();
				
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
		
	delete buf;
	}
	
/**
@SYMTestCaseID			SYSLIB-SQL-UT-4118
@SYMTestCaseDesc		SQL, transaction whole BLOB write, performance tests.
						Tests writing a 256Kb data block to a 256Kb blob, within a transaction,
						to examine the relative performance of the TSqlBlob and RSqlBlobWriteStream APIs.
@SYMTestPriority		Medium
@SYMTestActions			Whole update of a blob, within a transaction, using the new TSqlBlob and 
						RSqlBlobWriteStream APIs.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ5912
*/
void TransWholeWriteTestL()
	{
	TInt bufLen = TheBlobSize;
	HBufC8* buf = HBufC8::NewL(bufLen);
	TPtr8 dataPtr =	buf->Des();
	dataPtr.SetLength(bufLen);
	dataPtr.Fill('Z', bufLen);	
	
	TheTestTitle.Copy(_L("Whole BLOB write - TSqlBlob::Set() in transaction"));
	
	CreateTestDb();
	InsertRealBlob(); // insert blob of "TheBlobSize" size
	
	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
		
	TInt err = TheDb.Exec(_L8("BEGIN"));
	TEST(err >= 0);		
	TSqlBlob::SetL(TheDb, _L("A"), _L("B"), dataPtr);			
	err = TheDb.Exec(_L8("COMMIT"));
	TEST(err >= 0);	

	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();

	PrintWriteStats();
	
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);

	// to avoid caching issues, close and re-create the database for the next part
	
	TheTestTitle.Copy(_L("Whole BLOB write - RSqlBlobWriteStream::WriteL() in transaction"));
	
	CreateTestDb();
	InsertRealBlob(); // insert blob of "TheBlobSize" size
	
	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
	
	err = TheDb.Exec(_L8("BEGIN"));
	TEST(err >= 0);	
	RSqlBlobWriteStream strm;
	CleanupClosePushL(strm);
	strm.OpenL(TheDb, _L("A"), _L("B"));	
	strm.WriteL(dataPtr);
	CleanupStack::PopAndDestroy(&strm);
	err = TheDb.Exec(_L8("COMMIT"));
	TEST(err >= 0);	

	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();

	PrintWriteStats();
				
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
		
	delete buf;
	}	

/**
@SYMTestCaseID			SYSLIB-SQL-UT-4119
@SYMTestCaseDesc		SQL, whole BLOB read, performance tests.
						Tests reading a 256Kb blob in one block to examine the
						relative performance of the TSqlBlob and RSqlBlobReadStream APIs.
@SYMTestPriority		Medium
@SYMTestActions			Whole retrieval of a blob using the new TSqlBlob and RSqlBlobReadStream APIs.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ5912
*/	
void WholeReadTestL()
	{
	TInt bufLen = TheBlobSize; 
	HBufC8* buf = HBufC8::NewL(bufLen);
	TPtr8 dataPtr =	buf->Des();
	dataPtr.SetLength(bufLen);
	dataPtr.Fill('A', bufLen);	
	
	TheTestTitle.Copy(_L("Whole BLOB read - TSqlBlob::GetLC()"));
	
	CreateTestDb();
	InsertRealBlob(); // insert blob of "TheBlobSize" size

	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
	
	HBufC8* readBuf = TSqlBlob::GetLC(TheDb, _L("A"), _L("B"));
	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();
	
	TEST(readBuf->Des().Compare(buf->Des()) == 0);
	CleanupStack::PopAndDestroy(readBuf);
	
	PrintReadStats();
	
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	
	// to avoid caching issues, close and re-create the database for the next part
	TheTestTitle.Copy(_L("Whole BLOB read - TSqlBlob::Get()"));
	
	CreateTestDb();
	InsertRealBlob(); // insert blob of "TheBlobSize" size
	
	HBufC8* preBuf = HBufC8::NewLC(bufLen);
	TPtr8 preBufPtr(preBuf->Des());
	
	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
	
	TInt err = TSqlBlob::Get(TheDb, _L("A"), _L("B"), preBufPtr);	
	TEST2(err, KErrNone);
	
	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();
	
	TEST(preBufPtr.Compare(buf->Des()) == 0);
	CleanupStack::PopAndDestroy(preBuf);
	
	PrintReadStats();
	
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	
	// to avoid caching issues, close and re-create the database for the next part
	TheTestTitle.Copy(_L("Whole BLOB read - RSqlBlobReadStream::ReadL()"));
	
	CreateTestDb();
	InsertRealBlob(); // insert blob of "TheBlobSize" size
	
	preBuf = HBufC8::NewLC(bufLen);	
	TPtr8 preBufP(preBuf->Des());
	
	TheDbSize1 = TheDb.Size();
	TheStartTicks = User::FastCounter();
	
	RSqlBlobReadStream strm;
	CleanupClosePushL(strm);
	strm.OpenL(TheDb, _L("A"), _L("B"));
	strm.ReadL(preBufP, bufLen);
	CleanupStack::PopAndDestroy(&strm);

	TheEndTicks = User::FastCounter();
	TheDbSize2 = TheDb.Size();
	
	TEST(preBufP.Compare(buf->Des()) == 0);
	CleanupStack::PopAndDestroy(preBuf);

	PrintReadStats();
	
	TheDb.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
		
	delete buf;
	}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void DoTests()
	{
	TheTestTitle.Format(_L("@SYMTestCaseID:SYSLIB-SQL-UT-4084 SQL, BLOB write, performance tests, encoding: \"%S\", page size: %d\r\n"), 
			TheCmdLineParams.iDbEncoding == TCmdLineParams::EDbUtf16 ? &KUtf16 : &KUtf8, TheCmdLineParams.iPageSize);
	TheTest.Start(TheTestTitle);
	BlobWriteTest();
	
	TheTestTitle.Format(_L("@SYMTestCaseID:SYSLIB-SQL-UT-4085 SQL, BLOB read, performance tests, encoding: \"%S\", page size: %d\r\n"), 
			TheCmdLineParams.iDbEncoding == TCmdLineParams::EDbUtf16 ? &KUtf16 : &KUtf8, TheCmdLineParams.iPageSize);
	TheTest.Next(TheTestTitle);
	BlobReadTest();

	TheBlobSize = 1024 * 1024 + 128 * 1024;//1.125Mb 

	TheTestTitle.Format(_L("@SYMTestCaseID:SYSLIB-SQL-UT-4115 SQL, sequential BLOB writes, performance tests, encoding: \"%S\", page size: %d\r\n"), 
			TheCmdLineParams.iDbEncoding == TCmdLineParams::EDbUtf16 ? &KUtf16 : &KUtf8, TheCmdLineParams.iPageSize);
	TheTest.Next(TheTestTitle);
	TRAPD(err, SequentialWriteTestL());
	TEST2(err, KErrNone);
	
	TheTestTitle.Format(_L("@SYMTestCaseID:SYSLIB-SQL-UT-4116 SQL, transaction sequential BLOB writes, performance tests, encoding: \"%S\", page size: %d\r\n"), 
			TheCmdLineParams.iDbEncoding == TCmdLineParams::EDbUtf16 ? &KUtf16 : &KUtf8, TheCmdLineParams.iPageSize);
	TheTest.Next(TheTestTitle);
	TRAP(err, TransSequentialWriteTestL());
	TEST2(err, KErrNone);
		
	TheBlobSize = 256 * 1024 ; // 256Kb
		
	TheTestTitle.Format(_L("@SYMTestCaseID:SYSLIB-SQL-UT-4117 SQL, whole BLOB write, performance tests, encoding: \"%S\", page size: %d\r\n"), 
			TheCmdLineParams.iDbEncoding == TCmdLineParams::EDbUtf16 ? &KUtf16 : &KUtf8, TheCmdLineParams.iPageSize);
	TheTest.Next(TheTestTitle);
	TRAP(err, WholeWriteTestL());
	TEST2(err, KErrNone);
	
	TheTestTitle.Format(_L("@SYMTestCaseID:SYSLIB-SQL-UT-4118 SQL, transaction whole BLOB write, performance tests, encoding: \"%S\", page size: %d\r\n"), 
			TheCmdLineParams.iDbEncoding == TCmdLineParams::EDbUtf16 ? &KUtf16 : &KUtf8, TheCmdLineParams.iPageSize);
	TheTest.Next(TheTestTitle);
	TRAP(err, TransWholeWriteTestL());
	TEST2(err, KErrNone);
	
	TheTestTitle.Format(_L("@SYMTestCaseID:SYSLIB-SQL-UT-4119 SQL, whole BLOB read, performance tests, encoding: \"%S\", page size: %d\r\n"), 
			TheCmdLineParams.iDbEncoding == TCmdLineParams::EDbUtf16 ? &KUtf16 : &KUtf8, TheCmdLineParams.iPageSize);
	TheTest.Next(TheTestTitle);
	TRAP(err, WholeReadTestL());
	TEST2(err, KErrNone);
	}

TInt E32Main()
	{
	TheTest.Title();

	CTrapCleanup* tc = CTrapCleanup::New();
	TheTest(tc != NULL);

	__UHEAP_MARK;

	GetCmdLineParamsAndSqlConfigString(TheTest, _L("t_sqlperformance2"), TheCmdLineParams, TheSqlConfigString);
	_LIT(KDbName, "c:\\test\\t_sqlperformance2.db");
	PrepareDbName(KDbName, TheCmdLineParams.iDriveName, TheDbFileName);

	TheTest.Printf(_L("==Databases: %S\r\n"), &TheDbFileName); 
	
	TestEnvInit();
	DoTests();
	TestEnvDestroy();

	__UHEAP_MARKEND;
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;

	User::Heap().Check();
	return KErrNone;
	}
	

