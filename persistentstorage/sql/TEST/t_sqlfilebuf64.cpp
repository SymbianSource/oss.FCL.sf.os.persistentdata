// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifdef _SQLPROFILER

#include <bautils.h>
#include "FileBuf64.h"

///////////////////////////////////////////////////////////////////////////////////////

TBool TheOsCallTimeDetailedProfileEnabled = ETrue;//Needed because the RFileBuf64 source is included directly into this test
												  //nd the sql profiler is enabled (_SQLPROFILER is defined in the MMP file)

static RTest TheTest(_L("t_sqlfilebuf64 test"));
static RFs   TheFs;

_LIT(KTestDir, "c:\\test\\");
_LIT(KTestFile, "c:\\test\\t_sqlfilebuf64.bin");
_LIT(KTestFile2, "\\test\\t_sqlfilebuf64_2.bin");

static TBuf8<1024> TheBuf;
static TFileName TheDbName;

static TInt TheProcessHandleCount = 0;
static TInt TheThreadHandleCount = 0;
static TInt TheAllocatedCellsCount = 0;

#ifdef _DEBUG
static const TInt KBurstRate = 100;
#endif

enum TOomTestType
	{
	EOomCreateTest,	
	EOomOpenTest,	
	EOomTempTest
	};

///////////////////////////////////////////////////////////////////////////////////////

void DeleteTestFiles()
	{
	if(TheDbName.Length() > 0)
		{
		(void)TheFs.Delete(TheDbName);
		}
	(void)TheFs.Delete(KTestFile);
	}

void TestEnvDestroy()
	{
	DeleteTestFiles();
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
		RDebug::Print(_L("*** Line %d\r\n"), aLine);
		TheTest(EFalse, aLine);
		}
	}
void Check2(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		TestEnvDestroy();
		RDebug::Print(_L("*** Line %d, Expected result: %d, got: %d\r\n"), aLine, aExpected, aValue);
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

	err = TheFs.MkDir(KTestDir);
	TEST(err == KErrNone || err == KErrAlreadyExists);
	}

///////////////////////////////////////////////////////////////////////////////////////

static void MarkHandles()
	{
	RThread().HandleCount(TheProcessHandleCount, TheThreadHandleCount);
	}

static void MarkAllocatedCells()
	{
	TheAllocatedCellsCount = User::CountAllocCells();
	}

static void CheckAllocatedCells()
	{
	TInt allocatedCellsCount = User::CountAllocCells();
	TEST2(allocatedCellsCount, TheAllocatedCellsCount);
	}

static void CheckHandles()
	{
	TInt endProcessHandleCount;
	TInt endThreadHandleCount;
	
	RThread().HandleCount(endProcessHandleCount, endThreadHandleCount);

	TEST2(TheProcessHandleCount, endProcessHandleCount);
	TEST2(TheThreadHandleCount, endThreadHandleCount);
	}

static void VerifyFileContent(const TDesC8& aPattern)
	{
	TheBuf.Zero();
	
	RFile64 file;
	TInt err = file.Open(TheFs, KTestFile, EFileShareReadersOrWriters);
	TEST2(err, KErrNone);

	TInt64 fsize;
	err = file.Size(fsize);
	TEST2(err, KErrNone);
	TEST2((TInt)fsize, aPattern.Length());
	
	err = file.Read(TheBuf, aPattern.Length());
	TEST2(err, KErrNone);
	
	file.Close();
	
	err = TheBuf.Compare(aPattern);
	TEST2(err, 0);
	}

static void VerifyFileContent(const TDesC8& aPattern, TInt64 aFilePos)
	{
	__ASSERT_DEBUG(aFilePos >= 0, User::Invariant());
	
	TheBuf.Zero();
	
	RFile64 file;
	TInt err = file.Open(TheFs, KTestFile, EFileShareReadersOrWriters);
	TEST2(err, KErrNone);

	err = file.Read(aFilePos, TheBuf, aPattern.Length());
	TEST2(err, KErrNone);
	
	file.Close();
	
	err = TheBuf.Compare(aPattern);
	TEST2(err, 0);
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4132
@SYMTestCaseDesc		RFileBuf64 write test 1.
						The test performs file write operations using RFileBuf64 class.
						The write positions are inside the buffer or right at the end of the buffer.
						The purpose of the test: to verify the logic of RFileBuf64::Write().
@SYMTestActions			RFileBuf64 write test 1.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void WriteTest1()
	{
	RFileBuf64 fbuf(1024);
	TInt err = fbuf.Create(TheFs, KTestFile, EFileWrite);
	TEST2(err, KErrNone); 
    fbuf.ProfilerReset();
    
    //Zero write request
	err = fbuf.Write(0, _L8(""));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);

	//First write operation. After the operation the file buffer must countain 10 bytes.
	err = fbuf.Write(0, _L8("A123456789"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 1);

	//Second write operation. The offset is at the middle of the buffer.  Data length: 10;
	err = fbuf.Write(5, _L8("ZZZZZEEEEE"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);

	//Third write operation. The offset is at the end of the buffer.  Data length: 5;
	err = fbuf.Write(15, _L8("CCCCC"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);

	err = fbuf.Flush();
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 1);
	TEST2(fbuf.iFileFlushCount, 1);
	TEST2(fbuf.iFileWriteAmount, 20);
	TEST2(fbuf.iFileSizeCount, 1);

	fbuf.Close();
	
	VerifyFileContent(_L8("A1234ZZZZZEEEEECCCCC"));

	(void)TheFs.Delete(KTestFile);
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4133
@SYMTestCaseDesc		RFileBuf64 write test 2.
						The test performs file write operations using RFileBuf64 class.
						The write positions are beyound the end of the file but within the buffer capacity.
						The purpose of the test: to verify the logic of RFileBuf64::Write().
@SYMTestActions			RFileBuf64 write test 2.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void WriteTest2()
	{
	RFileBuf64 fbuf(1024);
	TInt err = fbuf.Create(TheFs, KTestFile, EFileWrite);
	TEST2(err, KErrNone); 
    fbuf.ProfilerReset();

	//First write operation. After the operation the file buffer must countain 10 bytes.
	err = fbuf.Write(0, _L8("A123456789"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 1);

	//Second write operation. After the operation the file buffer must countain 10 + 10 zeros + 10 bytes.
	err = fbuf.Write(20, _L8("FFGGHHJJKK"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 1);

	err = fbuf.Flush();
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 1);
	TEST2(fbuf.iFileFlushCount, 1);
	TEST2(fbuf.iFileWriteAmount, 30);
	TEST2(fbuf.iFileSizeCount, 1);

	fbuf.Close();
	
	TBuf8<30> pattern;
	pattern.Append(_L8("A123456789"));
	pattern.AppendFill(TChar(0), 10);
	pattern.Append(_L8("FFGGHHJJKK"));
	VerifyFileContent(pattern);
	
	(void)TheFs.Delete(KTestFile);
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4134
@SYMTestCaseDesc		RFileBuf64 write test 3.
						The test performs file write operations using RFileBuf64 class.
						The write position is before the start of the buffer but there is room for move.
						The purpose of the test: to verify the logic of RFileBuf64::Write().
@SYMTestActions			RFileBuf64 write test 3.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void WriteTest3()
	{
	//Iteration 1: The file length is 0, the first operation is "write beyond the end"
	//Iteration 2: The file length is 30, the first write operation is within the file.
	for(TInt i=0;i<2;++i)
		{
		RFileBuf64 fbuf(1024);
		TInt err = i == 0 ? fbuf.Create(TheFs, KTestFile, EFileWrite) : fbuf.Open(TheFs, KTestFile, EFileWrite);
		TEST2(err, KErrNone); 
	    fbuf.ProfilerReset();

		//First write operation. The offset is not 0.  Data length: 10;
		err = fbuf.Write(20, _L8("A123456789"));
		TEST2(err, KErrNone); 
		TEST2(fbuf.iFileWriteCount, 0);
		TEST2(fbuf.iFileWriteAmount, 0);
		TEST2(fbuf.iFileSizeCount, 1);
		
		//Second write operation. The offset is 0.  Data length: 20;
		err = fbuf.Write(0, _L8("AASSDDFFRR**********"));
		TEST2(err, KErrNone); 
		TEST2(fbuf.iFileWriteCount, 0);
		TEST2(fbuf.iFileWriteAmount, 0);
		TEST2(fbuf.iFileSizeCount, 1);

		err = fbuf.Flush();
		TEST2(err, KErrNone); 
		TEST2(fbuf.iFileWriteCount, 1);
		TEST2(fbuf.iFileFlushCount, 1);
		TEST2(fbuf.iFileWriteAmount, 30);
		TEST2(fbuf.iFileSizeCount, 1);

		fbuf.Close();

		VerifyFileContent(_L8("AASSDDFFRR**********A123456789"));
		}
	(void)TheFs.Delete(KTestFile);
	}	

/**
@SYMTestCaseID			PDS-SQL-UT-4135
@SYMTestCaseDesc		RFileBuf64 write test 4.
						The test performs file write operations using RFileBuf64 class and verifies that
						that the pending write data will be stored in the file when the buffer is closed.
						The purpose of the test: to verify the logic of RFileBuf64::Write().
@SYMTestActions			RFileBuf64 write test 4.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void WriteTest4()
	{
	RFileBuf64 fbuf(1024);
	TInt err = fbuf.Create(TheFs, KTestFile, EFileWrite);
	TEST2(err, KErrNone); 
    fbuf.ProfilerReset();

	// Data length: 10;
	err = fbuf.Write(0, _L8("A123456789"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 1);

	// Data length: 0;
	err = fbuf.Write(10, _L8(""));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 1);

	fbuf.Close();
	
	VerifyFileContent(_L8("A123456789"));
	
	(void)TheFs.Delete(KTestFile);
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4136
@SYMTestCaseDesc		RFileBuf64 write test 5.
						The test performs file write operations using RFileBuf64 class.
						The data is written before the start of the file buffer and is too big to fit in the buffer.
						The purpose of the test: to verify the logic of RFileBuf64::Write().
@SYMTestActions			RFileBuf64 write test 5.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void WriteTest5()
	{
	RFileBuf64 fbuf(20);
	TInt err = fbuf.Create(TheFs, KTestFile, EFileWrite);
	TEST2(err, KErrNone); 
    fbuf.ProfilerReset();

	//First write operation. The offset is not 0.  Data length: 10;
	err = fbuf.Write(10, _L8("A123456789"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 1);

	//Second write operation. The offset is 0.  Data length: 12;
	err = fbuf.Write(0, _L8("ZZXXCCVVBBNN"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 1);
	TEST2(fbuf.iFileWriteAmount, 20);
	TEST2(fbuf.iFileSizeCount, 1);

	err = fbuf.Flush();
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 2);
	TEST2(fbuf.iFileFlushCount, 1);
	TEST2(fbuf.iFileWriteAmount, 20 + 12);
	TEST2(fbuf.iFileSizeCount, 1);

	fbuf.Close();

	VerifyFileContent(_L8("ZZXXCCVVBBNN23456789"));
	
	(void)TheFs.Delete(KTestFile);
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4137
@SYMTestCaseDesc		RFileBuf64 write test 6.
						The test performs file write operations using RFileBuf64 class.
						The data is written before the start of the file buffer and is too big to fit in the buffer.
						The purpose of the test: to verify the logic of RFileBuf64::Write().
@SYMTestActions			RFileBuf64 write test 6.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void WriteTest6()
	{
	RFileBuf64 fbuf(20);
	TInt err = fbuf.Create(TheFs, KTestFile, EFileWrite);
	TEST2(err, KErrNone); 

	err = fbuf.Write(0, _L8("A123456789B123456789C123456789"));
	TEST2(err, KErrNone); 
	err = fbuf.Flush();
	TEST2(err, KErrNone); 
	fbuf.Close();
	VerifyFileContent(_L8("A123456789B123456789C123456789"));

	err = fbuf.Open(TheFs, KTestFile, EFileWrite);
	TEST2(err, KErrNone); 
    fbuf.ProfilerReset();

	//First write operation. The offset is not 0. Data length: 10;
	err = fbuf.Write(15, _L8("OOOOOOOOOO"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 1);

	//Second write operation. The offset is 0. Data length: 15;
	err = fbuf.Write(0, _L8("TTTTTTTTTTTTTTT"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 1);
	TEST2(fbuf.iFileWriteAmount, 10);
	TEST2(fbuf.iFileSizeCount, 1);

	err = fbuf.Flush();
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileWriteCount, 2);
	TEST2(fbuf.iFileFlushCount, 1);
	TEST2(fbuf.iFileWriteAmount, 15 + 10);
	TEST2(fbuf.iFileSizeCount, 1);

	fbuf.Close();

	VerifyFileContent(_L8("TTTTTTTTTTTTTTTOOOOOOOOOO56789"));
	
	(void)TheFs.Delete(KTestFile);
	}

void PrepareReadTest()
	{
	RFile64 file;
	TInt err = file.Create(TheFs, KTestFile, EFileWrite);
	TEST2(err, KErrNone); 
	err = file.Write(_L8("A123456789ZZZZZZZZZZB-B-B-B-B-Y*Y*Y*Y*Y*"));
	TEST2(err, KErrNone); 
	err = file.Flush();
	TEST2(err, KErrNone); 
	file.Close();
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4138
@SYMTestCaseDesc		RFileBuf64 read test 1.
						The test performs file read operations using RFileBuf64 class.
						Tested "read" operations:
							- Zero max length request;
							- Too big read request;
							- Read beyond the end of the file;
						The purpose of the test: to verify the logic of RFileBuf64::Read().
@SYMTestActions			RFileBuf64 read test 1.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void ReadTest1()
	{
	const TInt KBufMaxSize = 20;// This is half the file size
	RFileBuf64 fbuf(KBufMaxSize);
	TInt err = fbuf.Open(TheFs, KTestFile, EFileWrite | EFileRead | EFileShareReadersOrWriters);
	TEST2(err, KErrNone); 
    fbuf.ProfilerReset();
    
	//Zero max length request
	HBufC8* buf1 = HBufC8::New(0);
	TEST(buf1 != NULL);
	TPtr8 ptr1 = buf1->Des();
	err = fbuf.Read(0, ptr1);
	TEST2(err, KErrNone); 
	delete buf1;
	TEST2(fbuf.iFileReadCount, 0);
	TEST2(fbuf.iFileReadAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);
	
	//Too big request
	TBuf8<KBufMaxSize * 2> buf2;
	err = fbuf.Read(0, buf2);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, (KBufMaxSize * 2));
	TEST2(fbuf.iFileSizeCount, 1);
	VerifyFileContent(buf2);
	
	//Read beyond the end of the file
	err = fbuf.Read(2000, buf2);
	TEST2(err, KErrNone); 
	TEST2(buf2.Length(), 0); 

	fbuf.Close();
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4139
@SYMTestCaseDesc		RFileBuf64 read test 2.
						The test performs file read operations using RFileBuf64 class.
						Tested operations:
							- Non-buffered reads;
							- Buffered reads;
						The purpose of the test: to verify the logic of RFileBuf64::Read().
@SYMTestActions			RFileBuf64 read test 2.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void ReadTest2()
	{
	RFileBuf64 fbuf(1024);
	TInt err = fbuf.Open(TheFs, KTestFile, EFileWrite | EFileRead | EFileShareReadersOrWriters);
	TEST2(err, KErrNone); 
    fbuf.ProfilerReset();
    
    //1. Read bytes [0..20]
    TBuf8<20> buf1;
	err = fbuf.Read(0, buf1);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, 20);
	TEST2(fbuf.iFileSizeCount, 1);
    fbuf.ProfilerReset();
	VerifyFileContent(buf1, 0);
    
    //2. Read again, bytes [10..20]. They are not buffered.
    TBuf8<10> buf2;
	err = fbuf.Read(10, buf2);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, 10);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	VerifyFileContent(buf2, 10);

    //3. Read again, bytes [20..30]. They are not buffered. But the file buffer will be populated, 
    //   because the file read position matches the guessed file read position from step 2.
	err = fbuf.Read(20, buf2);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, (10 * 2));
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	VerifyFileContent(buf2, 20);

	//4. Read again, bytes [25..35]. This is a buffered read operation.
	err = fbuf.Read(25, buf2);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 0);
	TEST2(fbuf.iFileReadAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	VerifyFileContent(buf2, 25);
	
	//5. Read again, bytes [15..25]. This is a non buffered read operation.
	err = fbuf.Read(15, buf2);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, 10);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	VerifyFileContent(buf2, 15);

    //6. Read again, bytes [25..35]. This is a buffered read operation. The buffer from step 3 is still there.
	err = fbuf.Read(25, buf2);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 0);
	TEST2(fbuf.iFileReadAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	VerifyFileContent(buf2, 25);
	
    //7. Read again, bytes [35..45] - beyond the end of the file. 
    //   This is a buffered read operation. The buffer from step 3 is still there.
	err = fbuf.Read(35, buf2);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 0);
	TEST2(fbuf.iFileReadAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);
	TEST2(buf2.Size(), 5);
    fbuf.ProfilerReset();
	VerifyFileContent(buf2, 35);
	
	fbuf.Close();
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4140
@SYMTestCaseDesc		RFileBuf64 read test 3.
						The test performs file read operations using RFileBuf64 class.
						Tested operations:
							- Non-buffered reads;
							- Buffered reads;
							- Part- buffered reads;
						The purpose of the test: to verify the logic of RFileBuf64::Read().
@SYMTestActions			RFileBuf64 read test 3.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void ReadTest3()
	{
	RFileBuf64 fbuf(1024);
	TInt err = fbuf.Open(TheFs, KTestFile, EFileWrite | EFileRead | EFileShareReadersOrWriters);
	TEST2(err, KErrNone); 
    fbuf.ProfilerReset();
    
    //1. Read bytes [0..10]. Non buffered.
    TBuf8<10> buf1;
	err = fbuf.Read(0, buf1);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, 10);
	TEST2(fbuf.iFileSizeCount, 1);
    fbuf.ProfilerReset();
	VerifyFileContent(buf1, 0);

    //2. Read bytes [10..20]. Non buffered. But the file buffer is populated, bytes [10..40].
	err = fbuf.Read(10, buf1);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, 30);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	VerifyFileContent(buf1, 10);
	
    //3. Read bytes [25..35]. Buffered. Because the previous operation [2] performed a read-ahead operation.
	err = fbuf.Read(25, buf1);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 0);
	TEST2(fbuf.iFileReadAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	VerifyFileContent(buf1, 25);

    //4. Write bytes [20..30]. Buffered. Read buffer is gone, the file buffer contains the [20..30] file area.
	err = fbuf.Write(20, _L8("IIIIIQQQQQ"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 0);
	TEST2(fbuf.iFileReadAmount, 0);
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();

    //5. Read bytes [25..35]. Part-buffered. Part of pending writes picked up. Then the buffer is flushed.
	err = fbuf.Read(25, buf1);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, 5);
	TEST2(fbuf.iFileWriteCount, 1);
	TEST2(fbuf.iFileWriteAmount, 10);
	TEST2(fbuf.iFileSizeCount, 0);
	
    fbuf.ProfilerReset();
    err = fbuf.Flush();
    TEST2(err, KErrNone);
    
    //All cached data should have been written to the file before the Flush() call.
	TEST2(fbuf.iFileReadCount, 0);
	TEST2(fbuf.iFileReadAmount, 0);
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);
	TEST2(fbuf.iFileFlushCount, 1);
    
    fbuf.ProfilerReset();
	VerifyFileContent(buf1, 25);

    //6. The buffer is empty after the last flush. Write bytes [0..10]. The file buffer contains the [0..10] file area.
	err = fbuf.Write(0, _L8("PPOOIIUUYY"));
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 0);
	TEST2(fbuf.iFileReadAmount, 0);
	TEST2(fbuf.iFileWriteCount, 0);
	TEST2(fbuf.iFileWriteAmount, 0);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	
    //7. Read bytes [5..15]. Part buffered. Pending writes picked up. The content is written to the file.
	err = fbuf.Read(5, buf1);
	TEST2(err, KErrNone); 
	TEST2(fbuf.iFileReadCount, 1);
	TEST2(fbuf.iFileReadAmount, 5);
	TEST2(fbuf.iFileWriteCount, 1);
	TEST2(fbuf.iFileWriteAmount, 10);
	TEST2(fbuf.iFileSizeCount, 0);
    fbuf.ProfilerReset();
	VerifyFileContent(buf1, 5);
	
	fbuf.Close();
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4141
@SYMTestCaseDesc		RFileBuf64::SetReadAheadSize() test.
						The test iterates over all existing drives.
						For each R/W drive a test file is created using RFileBuf64 class.
						Then the test collects information regarding the block size, cluster size and 
						read buffer size and calls RFileBuf64::SetReadAheadSize() with these parameters
						to check how the read-ahead buffer size will be recalculated.
@SYMTestActions			RFileBuf64::SetReadAheadSize() test.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void SetReadAheadSizeTest()
	{
	TheTest.Printf(_L("==================\r\n"));
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
	
	for(TInt drive=EDriveA;drive<=EDriveZ;++drive)
		{
		TDriveInfo driveInfo;
		TInt err = TheFs.Drive(driveInfo, drive);
		if(err == KErrNone)
			{
			TVolumeInfo vinfo;
			err = TheFs.Volume(vinfo, drive);
			if(err == KErrNone)
				{
				TVolumeIOParamInfo vparam;
				err = TheFs.VolumeIOParam(drive, vparam);
				TEST2(err, KErrNone);
				TBuf8<128> vinfoex8;
				err = TheFs.QueryVolumeInfoExt(drive, EFileSystemSubType, vinfoex8);
				TEST2(err, KErrNone);
				TPtrC vinfoex((const TUint16*)(vinfoex8.Ptr() + 8), vinfoex8[0]);
				TPtrC KMediaTypeNames[] = {KType1(), KType2(), KType3(), KType4(), KType5(), KType6(), KType7(), KType8(), KType9(), KType10(), KType11()};
				TheTest.Printf(_L("Drive: %C:, Type: %16.16S, File System: %8.8S, Size: %d Mb.\r\n"), 'A' + drive, &KMediaTypeNames[driveInfo.iType], &vinfoex, (TInt)(vinfo.iSize / (1024 * 1024)));
				TheTest.Printf(_L("            Size: %ld bytes.\r\n"), vinfo.iSize);
				TheTest.Printf(_L("       Block size=%d, Cluster size=%d, Read buffer size=%d.\r\n"), vparam.iBlockSize, vparam.iClusterSize, vparam.iRecReadBufSize);
				if(driveInfo.iType == EMediaRam || driveInfo.iType == EMediaHardDisk || driveInfo.iType == EMediaFlash || driveInfo.iType == EMediaNANDFlash)
				  	{
					TDriveUnit drvUnit(drive);
					TDriveName drvName = drvUnit.Name();
					TParse parse;
					parse.Set(KTestFile2, &drvName, NULL);
					TheDbName.Copy(parse.FullName());
					TRAP(err, BaflUtils::EnsurePathExistsL(TheFs, TheDbName));
					TEST(err == KErrNone || err == KErrAlreadyExists);
					(void)TheFs.Delete(TheDbName);
					RFileBuf64 fbuf64(8 * 1024);
					err = fbuf64.Create(TheFs, TheDbName, EFileRead | EFileWrite);
					TEST2(err, KErrNone);
					TInt readAhead = fbuf64.SetReadAheadSize(vparam.iBlockSize, vparam.iRecReadBufSize);
					TheTest.Printf(_L("       Read-ahead size=%d.\r\n"), readAhead);
					fbuf64.Close();
					(void)TheFs.Delete(TheDbName);
					}
				}
			else
				{
				TheTest.Printf(_L("Drive %C. RFs::Volume() has failed with err=%d.\r\n"), 'A' + drive, err);	
				}
			}
		else
			{
			TheTest.Printf(_L("Drive %C. RFs::Drive() has failed with err=%d.\r\n"), 'A' + drive, err);	
			}
		}
	TheTest.Printf(_L("==================\r\n"));
	//
	RFileBuf64 fbuf64(8 * 1024);//buffer capacity = 8Kb
	
	//"ReadRecBufSize" defined and is power of two, the "BlockSize" is also defined and is power of two
	TInt err2 = fbuf64.Create(TheFs, TheDbName, EFileRead | EFileWrite);
	TEST2(err2, KErrNone);
	TInt blockSize = 4096; TInt readRecBufSize = 2048;
	TInt readAhead2 = fbuf64.SetReadAheadSize(blockSize, readRecBufSize);
	TEST2(readAhead2, readRecBufSize);
	fbuf64.Close();
	
	//"ReadRecBufSize" defined and is power of two but is less than the default read-ahead value
	err2 = fbuf64.Open(TheFs, TheDbName, EFileRead | EFileWrite);
	TEST2(err2, KErrNone);
	blockSize = 0; readRecBufSize = 128;
	readAhead2 = fbuf64.SetReadAheadSize(blockSize, readRecBufSize);
	TEST2(readAhead2, RFileBuf64::KDefaultReadAheadSize);
	fbuf64.Close();
	
	//"ReadRecBufSize" defined and is power of two but is bigger than the buffer capacity
	err2 = fbuf64.Open(TheFs, TheDbName, EFileRead | EFileWrite);
	TEST2(err2, KErrNone);
	blockSize = -10; readRecBufSize = fbuf64.iCapacity * 2;
	readAhead2 = fbuf64.SetReadAheadSize(blockSize, readRecBufSize);
	TEST2(readAhead2, fbuf64.iCapacity);
	fbuf64.Close();
	
	//"ReadRecBufSize" defined but is not power of two, "BlockSize" defined but is less than the default read-ahead value
	err2 = fbuf64.Open(TheFs, TheDbName, EFileRead | EFileWrite);
	TEST2(err2, KErrNone);
	blockSize = 512; readRecBufSize = 4000;
	readAhead2 = fbuf64.SetReadAheadSize(blockSize, readRecBufSize);
	TEST2(readAhead2, RFileBuf64::KDefaultReadAheadSize);
	fbuf64.Close();
	
	//"ReadRecBufSize" defined but is not power of two, "BlockSize" defined and is bigger than the default read-ahead value
	err2 = fbuf64.Open(TheFs, TheDbName, EFileRead | EFileWrite);
	TEST2(err2, KErrNone);
	blockSize = 4096; readRecBufSize = 4000;
	readAhead2 = fbuf64.SetReadAheadSize(blockSize, readRecBufSize);
	TEST2(readAhead2, blockSize);
	fbuf64.Close();

	//"ReadRecBufSize" defined but is not power of two, "BlockSize" defined and is bigger than the buffer capacity
	err2 = fbuf64.Open(TheFs, TheDbName, EFileRead | EFileWrite);
	TEST2(err2, KErrNone);
	blockSize = fbuf64.iCapacity * 2; readRecBufSize = 1;
	readAhead2 = fbuf64.SetReadAheadSize(blockSize, readRecBufSize);
	TEST2(readAhead2, fbuf64.iCapacity);
	fbuf64.Close();
	
	//"ReadRecBufSize" negative, "BlockSize" defined but is not power of two
	err2 = fbuf64.Open(TheFs, TheDbName, EFileRead | EFileWrite);
	TEST2(err2, KErrNone);
	blockSize = 1000; readRecBufSize = -2;
	readAhead2 = fbuf64.SetReadAheadSize(blockSize, readRecBufSize);
	TEST2(readAhead2, RFileBuf64::KDefaultReadAheadSize);
	fbuf64.Close();
	
	//"ReadRecBufSize" negative, "BlockSize" negative
	err2 = fbuf64.Open(TheFs, TheDbName, EFileRead | EFileWrite);
	TEST2(err2, KErrNone);
	blockSize = -1; readRecBufSize = -2;
	readAhead2 = fbuf64.SetReadAheadSize(blockSize, readRecBufSize);
	TEST2(readAhead2, RFileBuf64::KDefaultReadAheadSize);
	fbuf64.Close();
	//
	(void)TheFs.Delete(TheDbName);
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4142
@SYMTestCaseDesc		RFileBuf64 OOM test.
						The test calls RFileBuf64:Create(), RFileBuf64:Open() and RFileBuf64:Temp() in an OOM
						simulation loop and verifies that no memory is leaked.
@SYMTestActions			RFileBuf64 OOM test.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		High
@SYMREQ					REQ12106
                        REQ12109
*/
void OomTest(TOomTestType aOomTestType)
	{
	(void)TheFs.Delete(KTestFile);
	
	if(aOomTestType == EOomOpenTest)
		{
		RFile64 file;
		TInt err2 = file.Create(TheFs, KTestFile, EFileWrite | EFileRead);	
		file.Close();
		TEST2(err2, KErrNone);
		}
	
	TFileName tmpFileName;
	TInt err = KErrNoMemory;
	TInt failingAllocationNo = 0;
	RFileBuf64 fbuf(1024);
	TheTest.Printf(_L("Iteration:\r\n"));
	while(err == KErrNoMemory)
		{
		TheTest.Printf(_L(" %d"), ++failingAllocationNo);
		
		MarkHandles();
		MarkAllocatedCells();
		
		__UHEAP_MARK;
		__UHEAP_SETBURSTFAIL(RAllocator::EBurstFailNext, failingAllocationNo, KBurstRate);
		
		switch(aOomTestType)
			{
			case EOomCreateTest:
				err = fbuf.Create(TheFs, KTestFile, EFileWrite | EFileRead);
				break;
			case EOomOpenTest:
				err = fbuf.Open(TheFs, KTestFile, EFileWrite | EFileRead);
				break;
			case EOomTempTest:
				{
				err = fbuf.Temp(TheFs, KTestDir, tmpFileName, EFileWrite | EFileRead);
				}
				break;
			default:
				TEST(0);
				break;
			}
		fbuf.Close();
		
		__UHEAP_RESET;
		__UHEAP_MARKEND;

		CheckAllocatedCells();
		CheckHandles();
		
		TEntry entry;
		if(err != KErrNoMemory)
			{
			TEST2(err, KErrNone);	
			}
		else if(aOomTestType == EOomCreateTest)
			{
			TInt err2 = TheFs.Entry(KTestFile, entry);
			TEST2(err2, KErrNotFound);
			}
		else if(aOomTestType == EOomTempTest)
			{
			if(tmpFileName.Size() > 0)
				{
				TInt err2 = TheFs.Entry(tmpFileName, entry);
				TEST2(err2, KErrNotFound);
				}
			}
		}
	TEST2(err, KErrNone);
	TheTest.Printf(_L("\r\n=== OOM Test succeeded at heap failure rate of %d ===\r\n"), failingAllocationNo);
	
	if(aOomTestType == EOomTempTest)
		{
		(void)TheFs.Delete(tmpFileName);
		}
	(void)TheFs.Delete(KTestFile);
	}

void DoTests()
	{
	TheTest.Start(_L(" @SYMTestCaseID:PDS-SQL-UT-4132 RFileBuf64 write test 1"));
	WriteTest1();
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4133 RFileBuf64 write test 2"));
	WriteTest2();
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4134 RFileBuf64 write test 3"));
	WriteTest3();
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4135 RFileBuf64 write test 4"));
	WriteTest4();
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4136 RFileBuf64 write test 5"));
	WriteTest5();
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4137 RFileBuf64 write test 6"));
	WriteTest6();
	TheTest.Next( _L("RFileBuf64 read test - preparation"));
	PrepareReadTest();
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4138 RFileBuf64 read test 1"));
	ReadTest1();
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4139 RFileBuf64 read test 2"));
	ReadTest2();
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4140 RFileBuf64 read test 3"));
	ReadTest3();

	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4141 RFileBuf64::SetReadAheadSize() test"));
	SetReadAheadSizeTest();
	
	(void)TheFs.Delete(KTestFile);
	
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4142 RFileBuf64::Create() OOM test"));
	OomTest(EOomCreateTest);
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4142 RFileBuf64::Open() OOM test"));
	OomTest(EOomOpenTest);
	TheTest.Next( _L(" @SYMTestCaseID:PDS-SQL-UT-4142 RFileBuf64::Temp() OOM test"));
	OomTest(EOomTempTest);
	}

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();
	TheTest(tc != NULL);
	
	__UHEAP_MARK;
	
	TestEnvInit();
	DeleteTestFiles();
	DoTests();
	TestEnvDestroy();

	__UHEAP_MARKEND;
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;

	User::Heap().Check();
	return KErrNone;
	}
	
#else//_SQLPROFILER	

TInt E32Main()
	{
	TheTest.Title();
	
 	TheTest.Start(_L("This test works only if the test is built with _SQLPROFILER macro defined!"));
	TheTest.End();
	TheTest.Close();
	
	return KErrNone;
	}
	
#endif//_SQLPROFILER
