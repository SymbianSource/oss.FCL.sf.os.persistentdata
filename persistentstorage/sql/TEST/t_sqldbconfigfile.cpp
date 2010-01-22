// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <stdlib.h>
#include <bautils.h>
#include <hal.h>
#include <sqldb.h>
#include "sqlite3.h"
#include "SqliteSymbian.h"
#include "SqlResourceTester.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
/// This test works only if the whole SQL component is built with SYSLIBS_TEST macro defined! ///
/////////////////////////////////////////////////////////////////////////////////////////////////

RTest TheTest(_L("t_sqldbconfigfile test"));

#ifdef SYSLIBS_TEST	

extern TBool IsStatementSupported(const TDesC& aStatementIn, const TDesC& aDbName, TDes& aStatementOut);

RFs TheFs;
RSqlDatabase TheDb;
sqlite3 *TheDbHandle = NULL;

_LIT(KTestDir, "c:\\test\\");
_LIT(KSqlSrvName, "sqlsrv.exe");

enum TConfigParamType {EPrmCacheSize, EPrmPageSize, EPrmDbEncoding};

TInt KillProcess(const TDesC& aProcessName);

_LIT(KCfgDb1, "c:[1111C1C1]a.db"); // shared, secure db
_LIT(KCfgDb2, "c:[1111C1C1]b.db"); // shared, secure db (no config file for it)
_LIT(KCfgDb3, "c:\\private\\1111C1C1\\c.db"); // private, secure db
_LIT(KCfgDb4, "c:\\test\\d.db"); // public db
_LIT(KCfgDb5, "c:[1111C1C1]e.db"); // shared, secure db (config file created before db is created)

_LIT(KCfgDb1ConfigFilePath, "c:\\private\\10281e17\\cfg[1111C1C1]a.db.0%d"); // config file version %d for a.db
_LIT(KCfgDb3ConfigFileV01Path, "c:\\private\\10281e17\\cfgc.db.01"); // config file v01 for c.db (unsupported)
_LIT(KCfgDb4ConfigFileV01Path, "c:\\private\\10281e17\\cfgd.db.01"); // config file v01 for d.db (unsupported)
_LIT(KCfgDb5ConfigFileV01Path, "c:\\private\\10281e17\\cfg[1111C1C1]e.db.01"); // config file v01 for e.db
_LIT(KCfgDb1CorruptConfigFilePath, "c:\\private\\10281e17\\cfg[1111C1C1]a.db.invalidextension"); // invalid config file name syntax

// config file v01 contents for db1 (also tests that upper and lower case statements are both supported)
_LIT8(KCfgDb1ConfigFile01Stmts, "CREATE INDEX idx ON table1(i1);CREATE INDEX idx2 ON table1(i2);INSERT INTO table1 (i1,i2,i3) values(5,8,9);DELETE FROM table1;create index multiidx ON TABLE1(i2,i3)");
// config file v01 contents for db3 (will not be processed as db3 is not a shared, secure db)
_LIT8(KCfgDb3ConfigFile01Stmts, "CREATE INDEX idx ON table3(i1)");
// config file v01 contents for db4 (will not be processed as db4 is not a shared, secure db)
_LIT8(KCfgDb4ConfigFile01Stmts, "CREATE INDEX idx ON table4(i1)");
// config file v01 contents for db5 (will eventually be processed after db5 itself is created)
_LIT8(KCfgDb5ConfigFile01Stmts, "CREATE INDEX idx ON table5(i1);\r\n");
// config file valid contents (used for v02 and others)
_LIT8(KCfgConfigFileValidStmt, "CREATE INDEX newIdx ON table1(i3)  ;"); 
// config file v03 empty contents
_LIT8(KCfgDb1ConfigFileV03EmptyStmt, ""); 
// config file v04 unsupported contents
_LIT8(KCfgDb1ConfigFileV04UnsupportedStmt, "DELETE FROM table1"); 
// config file v05 only whitespace contents
_LIT8(KCfgDb1ConfigFileV05OnlyWhitespaceStmt, " \r\n   \r\n"); 
// config file v06 invalid schema contents
_LIT8(KCfgDb1ConfigFileV06InvalidSchemaStmt, "CREATE INDEX thisIdx ON table999(i3)"); 
// config file v07 unsupported comment style
_LIT8(KCfgDb1ConfigFileV07InvalidCommentedStmt, "CREATE INDEX ind1 ON table1(i2) // create an index"); 
// config file v08 sequence of different statements
_LIT8(KCfgDb1ConfigFileV08SeqStmt, ";  CREATE INDEX IdxFirst ON table1(i3)\r\n;  DELETE FROM table1;INSERT INTO table1 (i1,i2,i3) values(6,7,8);;CREATE INDEX IdxSecond ON table1(i1);");
// config file v09 whitespace before and after statement
_LIT8(KCfgDb1ConfigFileV09WhitespacePreAndPostStmt, "  CREATE INDEX intIdx ON table1(i1)       "); 
// config file v10 valid contents
_LIT8(KCfgDb1ConfigFileV10ValidStmt, "CREATE INDEX i3Index ON table1(i3)\n"); 
// config file v11 valid contents (also tests that any amount spaces and tabs are allowed between 'CREATE' and 'INDEX')
_LIT8(KCfgDb1ConfigFileV11ValidStmt, "CREATE     INDEX i1Index ON table1(i1);\nCREATE	INDEX i2Index ON table1(i2)");
// config file v12 invalid stmt plus valid stmt
_LIT8(KCfgDb1ConfigFileV12InvalidPlusValidStmt, "CREATE UNIQUE INDEX uniqueIdx ON table1(i1);CREATE INDEX v12Idx ON table1(i2)");
// config file v13 supported SQL comment style
_LIT8(KCfgDb1ConfigFileV13SQLCommentStmt, "CREATE INDEX v13Idx ON table1(i1) -- this is an SQL comment");
// config file v14 supported 'C' comment style
_LIT8(KCfgDb1ConfigFileV14CCommentStmt, "CREATE INDEX v14Idx ON table1(i3) /* this is a C comment */;");

//KLongDbName1 is "long" enough to allow "-journal" to be added at the end.
_LIT(KLongDbName1,  "c:[1111C1C1]a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a1234.db");
_LIT(KLongCfgName1, "c:\\private\\10281e17\\cfg[1111C1C1]a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a123456789a1234.db.01");

_LIT(KSqlSrvPrivatePath, "\\private\\10281e17\\");
_LIT(KGetSettingsSql, "SELECT * FROM symbian_settings WHERE Reserved==%d");
_LIT(KResetCollationDllSql, "UPDATE symbian_settings SET CollationDllName='hjagafsff'");
_LIT(KGetCollationDllSql, "SELECT CollationDllName FROM symbian_settings");

_LIT(KAttachDb1, "Db1");
_LIT(KAttachDb2, "Db2");
_LIT(KAttachDb5, "Db5");

_LIT(KDb1CheckNumRecords, "SELECT * FROM table1");
_LIT(KDb2CheckNumRecords, "SELECT * FROM table2");
_LIT(KDb3CheckNumRecords, "SELECT * FROM table3");
_LIT(KDb4CheckNumRecords, "SELECT * FROM table4");
_LIT(KDb5CheckNumRecords, "SELECT * FROM table5");
_LIT(KDbCheckNumIndices, "SELECT name FROM sqlite_master WHERE type = 'index'");

///////////////////////////////////////////////////////////////////////////////////////
// Destroy functions

TInt KillProcess(const TDesC& aProcessName)
	{
	TFullName name;
	//RDebug::Print(_L("Find and kill \"%S\" process.\n"), &aProcessName);
	TBuf<64> pattern(aProcessName);
	TInt length = pattern.Length();
	pattern += _L("*");
	TFindProcess procFinder(pattern);

	while (procFinder.Next(name) == KErrNone)
		{
		if (name.Length() > length)
			{//If found name is a string containing aProcessName string.
			TChar c(name[length]);
			if (c.IsAlphaDigit() ||
				c == TChar('_') ||
				c == TChar('-'))
				{
				// If the found name is other valid application name
				// starting with aProcessName string.
				//RDebug::Print(_L(":: Process name: \"%S\".\n"), &name);
				continue;
				}
			}
		RProcess proc;
		if (proc.Open(name) == KErrNone)
			{
			proc.Kill(0);
			//RDebug::Print(_L("\"%S\" process killed.\n"), &name);
			}
		proc.Close();
		}
	return KErrNone;
	}

void DeleteCfgFilesAndDbs()
	{
 	(void)RSqlDatabase::Delete(KLongDbName1);
 	(void)RSqlDatabase::Delete(KCfgDb1);
 	(void)RSqlDatabase::Delete(KCfgDb2);	
 	(void)RSqlDatabase::Delete(KCfgDb3);
 	(void)RSqlDatabase::Delete(KCfgDb4);
 	(void)RSqlDatabase::Delete(KCfgDb5);
 	(void)TheFs.Delete(KLongCfgName1);
 	(void)TheFs.Delete(KCfgDb3ConfigFileV01Path);
 	(void)TheFs.Delete(KCfgDb4ConfigFileV01Path);
 	(void)TheFs.Delete(KCfgDb5ConfigFileV01Path);
 	CFileMan* fileMan = 0;	
 	TRAPD(err, fileMan = CFileMan::NewL(TheFs));
 	if(KErrNone == err)
 		{
 		(void)fileMan->Delete(_L("c:\\private\\10281e17\\cfg[1111C1C1]a.db.*"));
		delete fileMan;			
 		}
  	}

void DestroyTestEnv()
	{
	if(TheDbHandle)
		{
		sqlite3_close(TheDbHandle);	
		TheDbHandle = NULL;
		}
	TheDb.Close();
	(void)KillProcess(KSqlSrvName);
	DeleteCfgFilesAndDbs();
	TheFs.Close();
	}
	
///////////////////////////////////////////////////////////////////////////////////////
// Test macros and functions

void Check(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		DestroyTestEnv();
		TheTest(EFalse, aLine);
		}
	}
void Check(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		DestroyTestEnv();
		TheTest.Printf(_L("*** Expected error: %d, got: %d\r\n"), aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST(arg) ::Check((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

// OOM test functions

static TInt TheHandleCount1;
static TInt TheHandleCount2;
static TInt TheAllocatedCellsCount;

void MarkHandles()
	{
	RThread().HandleCount(TheHandleCount1, TheHandleCount2);
	}

void CheckHandles()
	{
	TInt endHandleCount1;
	TInt endHandleCount2;

	RThread().HandleCount(endHandleCount1, endHandleCount2);

	TEST(TheHandleCount1 == endHandleCount1);
	TEST(TheHandleCount2 == endHandleCount2);
	}

void MarkAllocatedCells()
	{
	TheAllocatedCellsCount = User::CountAllocCells();
	}

void CheckAllocatedCells()
	{
	TInt allocatedCellsCount = User::CountAllocCells();
	TEST(allocatedCellsCount == TheAllocatedCellsCount);
	}

///////////////////////////////////////////////////////////////////////////////////////
// Set up functions

TInt DoCreateSecurityPolicy(RSqlSecurityPolicy& securityPolicy)
	{
	const TSecurityPolicy KDefaultPolicy(TSecurityPolicy::EAlwaysPass);
	if((KErrNone != securityPolicy.Create(KDefaultPolicy))
	   ||
	   (KErrNone != securityPolicy.SetDbPolicy(RSqlSecurityPolicy::ESchemaPolicy, KDefaultPolicy))
	   ||
	   (KErrNone != securityPolicy.SetDbPolicy(RSqlSecurityPolicy::EWritePolicy, KDefaultPolicy))
	   ||
	   (KErrNone != securityPolicy.SetDbPolicy(RSqlSecurityPolicy::EReadPolicy, KDefaultPolicy)))
		{
		return KErrGeneral;
		}
		
	return KErrNone;
	}
	
void CreateCfgFiles()
	{
 	// Create v01 of the test config files
 	
 	RFile file;
	TFileName fileName;
	TInt v1 = 1;
	fileName.Format(KCfgDb1ConfigFilePath(), v1);
	TInt err = file.Create(TheFs, fileName, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 pDb1((const TUint8*)KCfgDb1ConfigFile01Stmts().Ptr(), KCfgDb1ConfigFile01Stmts().Length());
	err = file.Write(pDb1);	
	file.Close();	
	TEST2(err, KErrNone);
	
	fileName.Copy(KCfgDb3ConfigFileV01Path);
	err = file.Create(TheFs, fileName, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 pDb3((const TUint8*)KCfgDb3ConfigFile01Stmts().Ptr(), KCfgDb3ConfigFile01Stmts().Length());
	err = file.Write(pDb3);	
	file.Close();	
	TEST2(err, KErrNone);	
	
	fileName.Copy(KCfgDb4ConfigFileV01Path);
	err = file.Create(TheFs, fileName, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 pDb4((const TUint8*)KCfgDb4ConfigFile01Stmts().Ptr(), KCfgDb4ConfigFile01Stmts().Length());
	err = file.Write(pDb4);	
	file.Close();	
	TEST2(err, KErrNone);
	
	fileName.Copy(KCfgDb5ConfigFileV01Path); // create the config file for Db5 before the database has been created
	err = file.Create(TheFs, fileName, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 pDb5((const TUint8*)KCfgDb5ConfigFile01Stmts().Ptr(), KCfgDb5ConfigFile01Stmts().Length());
	err = file.Write(pDb5);	
	file.Close();	
	TEST2(err, KErrNone);	
	}

void CreateCfgDbs()
	{
	// Create the test databases
	
 	TBuf<100> sql;
 		
 	RSqlSecurityPolicy securityPolicy;
 	TInt err = DoCreateSecurityPolicy(securityPolicy);
 	TEST2(err, KErrNone);
 	
 	err = TheDb.Create(KCfgDb1, securityPolicy);
 	TEST2(err, KErrNone);
 	sql.Copy(_L("CREATE TABLE table1(i1 INTEGER, i2 INTEGER, i3 INTEGER)"));
 	err = TheDb.Exec(sql);
 	TEST(err >= 0);
 	sql.Copy(_L("INSERT INTO table1 (i1,i2,i3) values(1,2,3)"));
 	err = TheDb.Exec(sql);
 	TEST(err == 1);
 	TheDb.Close();
 	
 	err = TheDb.Create(KCfgDb2, securityPolicy);
 	TEST2(err, KErrNone);
 	sql.Copy(_L("CREATE TABLE table2(i1 INTEGER, i2 INTEGER, i3 INTEGER)"));
 	err = TheDb.Exec(sql);
 	TEST(err >= 0);
 	sql.Copy(_L("INSERT INTO table2 (i1,i2,i3) values(4,5,6)"));
 	err = TheDb.Exec(sql);
 	TEST(err == 1);
 	TheDb.Close();
 	
 	securityPolicy.Close();
 
 	err = TheDb.Create(KCfgDb3);
 	TEST2(err, KErrNone);
 	sql.Copy(_L("CREATE TABLE table3(i1 INTEGER, i2 INTEGER)"));
 	err = TheDb.Exec(sql);
 	TEST(err >= 0);
 	sql.Copy(_L("INSERT INTO table3 (i1,i2) values(7,8)"));
 	err = TheDb.Exec(sql);
 	TEST(err == 1);
 	TheDb.Close();	
 
 	err = TheDb.Create(KCfgDb4);
 	TEST2(err, KErrNone);
 	sql.Copy(_L("CREATE TABLE table4(i1 INTEGER, i2 INTEGER, i3 INTEGER)"));
 	err = TheDb.Exec(sql);
 	TEST(err >= 0);
 	sql.Copy(_L("INSERT INTO table4 (i1,i2,i3) values(9,10,11)"));
 	err = TheDb.Exec(sql);
 	TEST(err == 1);
 	TheDb.Close();
 	}

void CreateCfgFilesAndDbs()
	{
	CreateCfgFiles();
	CreateCfgDbs();
	
	// Must now kill the SQL Server so that the config files 
	// created above are found and processed when it restarts
	(void)KillProcess(KSqlSrvName);
 	}
 	
 void CreateDb5()
	{
	// Create the Db5 test database (a config file for it already exists)
	
 	TBuf<100> sql;
 		
 	RSqlSecurityPolicy securityPolicy;
 	TInt err = DoCreateSecurityPolicy(securityPolicy);
 	TEST2(err, KErrNone);
 	
 	err = TheDb.Create(KCfgDb5, securityPolicy);
 	TEST2(err, KErrNone);
 	sql.Copy(_L("CREATE TABLE table5(i1 INTEGER, i2 INTEGER, i3 INTEGER)"));
 	err = TheDb.Exec(sql);
 	TEST(err >= 0);
 	sql.Copy(_L("INSERT INTO table5 (i1,i2,i3) values(1,2,3)"));
 	err = TheDb.Exec(sql);
 	TEST(err == 1);
 	
 	TheDb.Close();
 	securityPolicy.Close();
	}

void SetupTestEnv()
    {
	TInt err = TheFs.Connect();
	TEST2(err, KErrNone);

	err = TheFs.MkDir(KTestDir);
	TEST(err == KErrNone || err == KErrAlreadyExists);

	err = TheFs.CreatePrivatePath(EDriveC);
	TEST(err == KErrNone || err == KErrAlreadyExists);

	// Create the cfg dbs and config files
 	DeleteCfgFilesAndDbs(); // just incase any previous files are lingering
 	CreateCfgFilesAndDbs();
	}

///////////////////////////////////////////////////////////////////////////////////////
//
TInt CalcTimeMs(TUint32 aStartTicks, TUint32 aEndTicks)
	{
	static TInt freq = 0;
	if(freq == 0)
		{
		TEST2(HAL::Get(HAL::EFastCounterFrequency, freq), KErrNone);
		}
	TInt64 diffTicks = (TInt64)aEndTicks - (TInt64)aStartTicks;
	if(diffTicks < 0)
		{
		diffTicks = KMaxTUint32 + diffTicks + 1;
		}
	const TInt KMicroSecIn1Sec = 1000000;
	TInt32 us = (diffTicks * KMicroSecIn1Sec) / freq;
	return us / 1000;
	}

///////////////////////////////////////////////////////////////////////////////////////
// Config file replacement functions

void UpgradeDbConfigFile(const TInt aCurrentVersion)
	{
	(void)KillProcess(KSqlSrvName);
	TFileName oldFile;
	oldFile.Format(KCfgDb1ConfigFilePath(), aCurrentVersion);
	TInt err = TheFs.Delete(oldFile);
	TEST2(err, KErrNone);
	RFile file;
	TFileName newFile;
	TInt newVersion = aCurrentVersion+1;
	newFile.Format(KCfgDb1ConfigFilePath(), newVersion);
	err = file.Create(TheFs, newFile, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	
	switch(newVersion)
		{
			case 2:
				{
				TPtrC8 p((const TUint8*)KCfgConfigFileValidStmt().Ptr(), KCfgConfigFileValidStmt().Length());
				err = file.Write(p);
				break;
				}
			case 3:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV03EmptyStmt().Ptr(), KCfgDb1ConfigFileV03EmptyStmt().Length());
				err = file.Write(p);
				break;
				}
			case 4:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV04UnsupportedStmt().Ptr(), KCfgDb1ConfigFileV04UnsupportedStmt().Length());
				err = file.Write(p);
				break;	
				}
			case 5:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV05OnlyWhitespaceStmt().Ptr(), KCfgDb1ConfigFileV05OnlyWhitespaceStmt().Length());
				err = file.Write(p);
				break;
				}	
			case 6:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV06InvalidSchemaStmt().Ptr(), KCfgDb1ConfigFileV06InvalidSchemaStmt().Length());
				err = file.Write(p);
				break;
				}
			case 7:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV07InvalidCommentedStmt().Ptr(), KCfgDb1ConfigFileV07InvalidCommentedStmt().Length());
				err = file.Write(p);	
				break;
				}
			case 8:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV08SeqStmt().Ptr(), KCfgDb1ConfigFileV08SeqStmt().Length());
				err = file.Write(p);	
				break;
				}
			case 9:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV09WhitespacePreAndPostStmt().Ptr(), KCfgDb1ConfigFileV09WhitespacePreAndPostStmt().Length());
				err = file.Write(p);	
				break;
				}
			case 12:
				{
				// also delete version 10 of file
				oldFile.Format(KCfgDb1ConfigFilePath(), 10);
				err = TheFs.Delete(oldFile);
				TEST2(err, KErrNone);	
				
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV12InvalidPlusValidStmt().Ptr(), KCfgDb1ConfigFileV12InvalidPlusValidStmt().Length());
				err = file.Write(p);
				break;
				}
			case 13:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV13SQLCommentStmt().Ptr(), KCfgDb1ConfigFileV13SQLCommentStmt().Length());
				err = file.Write(p);	
				break;
				}
			case 14:
				{
				TPtrC8 p((const TUint8*)KCfgDb1ConfigFileV14CCommentStmt().Ptr(), KCfgDb1ConfigFileV14CCommentStmt().Length());
				err = file.Write(p);	
				break;
				}
			default:
				{
				err = KErrArgument;
				break;
				}		
		}
	file.Close();
	TEST2(err, KErrNone);
	}	
	
void DowngradeDbConfigFile(const TInt aCurrentVersion)
	{
	(void)KillProcess(KSqlSrvName);
	TFileName oldFile;
	oldFile.Format(KCfgDb1ConfigFilePath(), aCurrentVersion);
	TInt err = TheFs.Delete(oldFile);
	TEST2(err, KErrNone);
	RFile file;
	TFileName newFile;
	TInt previousVersion = aCurrentVersion-1;
	TEST(previousVersion > 0);
	newFile.Format(KCfgDb1ConfigFilePath(), previousVersion);
	err = file.Create(TheFs, newFile, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 p((const TUint8*)KCfgConfigFileValidStmt().Ptr(), KCfgConfigFileValidStmt().Length());
	err = file.Write(p);
	file.Close();
	TEST2(err, KErrNone);
	}
	
void CreateCorruptDbConfigFile(const TInt aCurrentVersion)
	{
	(void)KillProcess(KSqlSrvName);
	TFileName oldFile;
	oldFile.Format(KCfgDb1ConfigFilePath(), aCurrentVersion);
	TInt err = TheFs.Delete(oldFile);
	TEST2(err, KErrNone);
	RFile file;
	err = file.Create(TheFs, KCfgDb1CorruptConfigFilePath(), EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 p((const TUint8*)KCfgConfigFileValidStmt().Ptr(), KCfgConfigFileValidStmt().Length());
	err = file.Write(p);
	file.Close();
	TEST2(err, KErrNone);
	}	
	
void CreateTwoVersionsOfConfigFile()
	{
	(void)KillProcess(KSqlSrvName);
	TInt err = TheFs.Delete(KCfgDb1CorruptConfigFilePath);
	TEST2(err, KErrNone);
	RFile file;
	TFileName newFile;
	TInt nextVersion = 10;
	newFile.Format(KCfgDb1ConfigFilePath(), nextVersion);
	err = file.Create(TheFs, newFile, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 p10((const TUint8*)KCfgDb1ConfigFileV10ValidStmt().Ptr(), KCfgDb1ConfigFileV10ValidStmt().Length());
	err = file.Write(p10);
	file.Close();
	TEST2(err, KErrNone);
	++nextVersion;
	newFile.Format(KCfgDb1ConfigFilePath(), nextVersion);
	err = file.Create(TheFs, newFile, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 p11((const TUint8*)KCfgDb1ConfigFileV11ValidStmt().Ptr(), KCfgDb1ConfigFileV11ValidStmt().Length());
	err = file.Write(p11);
	file.Close();
	TEST2(err, KErrNone);	
	}
 
///////////////////////////////////////////////////////////////////////////////////////
// DoDbCfgTests() functions

TBool GuessSystemSettingsTable(const TDesC& aDbName, TInt aVersion)
	{
	// Note: We have to use SQLite directly to access the settings
	// table as the SQL Server denies permission to access this table
	// as it is in a shared, secure database
	
	TBool guessIsCorrect = EFalse;
	
	TParse parse;
	parse.Set(aDbName, &KSqlSrvPrivatePath, 0);
	
	TBuf8<KMaxFileName + 1> dbFileName;
	dbFileName.Copy(parse.FullName());
	
	TheDbHandle = NULL;
	TInt rc = sqlite3_open((const char*)dbFileName.PtrZ(), &TheDbHandle);
	TEST2(rc, SQLITE_OK);
	
	TBuf<100> queryBuf;
	queryBuf.Format(KGetSettingsSql(), aVersion);	
	
	sqlite3_stmt* stmtHandle = NULL;
	const void* stmtTailZ = NULL;
	rc = sqlite3_prepare16_v2(TheDbHandle, queryBuf.PtrZ(), -1, &stmtHandle, &stmtTailZ);
	TEST2(rc, SQLITE_OK);
	
	rc = sqlite3_step(stmtHandle);
	if(SQLITE_ROW == rc)
		{
		guessIsCorrect = ETrue;
		rc = sqlite3_step(stmtHandle);
		TEST2(rc, SQLITE_DONE);
		}
	
	sqlite3_finalize(stmtHandle);
	sqlite3_close(TheDbHandle);
	TheDbHandle = NULL;
	
	return guessIsCorrect;
	}

void CheckSystemSettingsTable(const TDesC& aDbName, TInt aVersion)
	{
	// Note: We have to use SQLite directly to access the settings
	// table as the SQL Server denies permission to access this table
	// as it is in a shared, secure database
	
	TParse parse;
	parse.Set(aDbName, &KSqlSrvPrivatePath, 0);
	
	TBuf8<KMaxFileName + 1> dbFileName;
	dbFileName.Copy(parse.FullName());
	
	sqlite3 *dbHandle = NULL;
	TInt rc = sqlite3_open((const char*)dbFileName.PtrZ(), &dbHandle);
	TEST2(rc, SQLITE_OK);
	
	TBuf<100> queryBuf;
	queryBuf.Format(KGetSettingsSql(), aVersion);	
	
	sqlite3_stmt* stmtHandle = NULL;
	const void* stmtTailZ = NULL;
	rc = sqlite3_prepare16_v2(dbHandle, queryBuf.PtrZ(), -1, &stmtHandle, &stmtTailZ);
	TEST2(rc, SQLITE_OK);
	
	rc = sqlite3_step(stmtHandle);
	TEST2(rc, SQLITE_ROW);
	
	rc = sqlite3_step(stmtHandle);
	TEST2(rc, SQLITE_DONE);
	
	sqlite3_finalize(stmtHandle);
	sqlite3_close(dbHandle);
	}

void CheckNumberRecordsL(const TDesC& aStmt)
	{
	// There should always be only 1 record in the table
	// in each database as INSERT and DELETE statements are
	// not supported in the config files
	
	// Prepare stmt
	RSqlStatement stmt;
	stmt.PrepareL(TheDb, aStmt);
	
	// Get each row
	TUint numRows = 0;
	TInt err = stmt.Next();
	while(KSqlAtRow == err)
		{
		numRows++;
		err = stmt.Next();
		}

	if (KSqlAtEnd != err)
		{
		User::Leave(KErrCorrupt);
		}
		
	if (numRows != 1)
		{
		User::Leave(KErrArgument);
		}

	// Close stmt
	stmt.Close();	
	}
	
TInt NumberIndicesExpectedInDb1(const TInt aExpectedStoredVersion)
	{
	TInt numIndices = -1;
	
	switch(aExpectedStoredVersion)
		{
     	// Only files 01 - 04, 09 and 11 will be successfully processed and so stored in the settings table
		case 1:
			{
			numIndices = 3; // 3 indices should be added to db1 based on config file 01
			break;
			}
		case 2:
			{
			numIndices = 4; // 1 more index should be added to db1 based on config file 02
			break;
			}
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			{
			numIndices = 4; // no more indices should be added to db1 based on config file 03 - 07
			break;
			}
		case 8:
			{
			numIndices = 6; // 2 more indices should be added to db1 based on config file 08
			break;
			}
		case 9:
			{
			numIndices = 7; // 1 more index should be added to db1 based on config file 09
			break;
			}
		case 11:
			{
			numIndices = 9; // 2 more indices should be added to db1 based on config file 11
			break;
			}	
		case 12:
			{
			numIndices = 10; // 1 more index should be added to db1 based on config file 12
			break;
			}
		case 13:
			{
			numIndices = 11; // 1 more index should be added to db1 based on config file 13
			break;
			}	
		case 14:
			{
			numIndices = 12; // 1 more index should be added to db1 based on config file 14
			break;
			}		
		}
		
	return numIndices;	
	}

void CheckNumberIndicesL(const TInt aNumIndicesExpected)
	{
	// Prepare stmt
	RSqlStatement stmt;
	stmt.PrepareL(TheDb, KDbCheckNumIndices);
	
	// Get each entry of type 'index' in the 'sqlite_master' table
	TUint numEntries = 0;
	TInt err = stmt.Next();
	while(KSqlAtRow == err)
		{
		numEntries++;
		err = stmt.Next();
		}

	if (KSqlAtEnd != err)
		{
		User::Leave(KErrCorrupt);
		}
		
	if (numEntries != aNumIndicesExpected)
		{
		User::Leave(KErrArgument);
		}

	// Close stmt
	stmt.Close();	
	}

void DoCfgOpenTests(TInt aExpectedStoredVersion = 1)
	{	
	// Open a shared, secure database - config ops should be applied on it
	TheTest.Printf(_L("===CfgOpen: Open shared, secure database\r\n"));
	TInt err = TheDb.Open(KCfgDb1);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb1CheckNumRecords)); // there should still be only 1 record in the table
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(NumberIndicesExpectedInDb1(aExpectedStoredVersion))); // there should now be 3 indices in the table
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb1, aExpectedStoredVersion); // check that the ops in the specified config file have been applied
		
	// Open again the same shared, secure database - no config should occur (it has already been done)
	TheTest.Printf(_L("===CfgOpen: Open shared, secure database again\r\n"));
	err = TheDb.Open(KCfgDb1);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb1CheckNumRecords)); 
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(NumberIndicesExpectedInDb1(aExpectedStoredVersion)));
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb1, aExpectedStoredVersion);
	
	// Open a shared, secure database - no config should occur (there is no config file for this database)
	TheTest.Printf(_L("===CfgOpen: Open shared, secure database (that has no config file)\r\n"));
	err = TheDb.Open(KCfgDb2);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb2CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(0)); 
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb2, 0);
	
	// Open a private, secure database - no config should occur
	TheTest.Printf(_L("===CfgOpen: Open private, secure database\r\n"));
	err = TheDb.Open(KCfgDb3);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb3CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(0)); 
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb3, 0);
	
	// Open a public database - no config should occur
	TheTest.Printf(_L("===CfgOpen: Open public database\r\n"));
	err = TheDb.Open(KCfgDb4);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb4CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(0)); 
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb4, 0);
	}
	
void DoUpgradedCfgOpenTest()
	{
	// Upgrade the config file for KCfgDb1, i.e. replace v1 file with v2 file
	UpgradeDbConfigFile(1);
	DoCfgOpenTests(2);	
	}

void DoBadCfgOpenTests()
	{
	// Test an empty config file
	UpgradeDbConfigFile(2); // the current file version is 02, replace it with 03
	DoCfgOpenTests(3); // version 03 will be successfully processed and stored in the settings table
	
	// Test a config file with unsupported operations (these will be ignored)
	UpgradeDbConfigFile(3); // the current file version is 03, replace it with 04
	DoCfgOpenTests(4); // version 04 will be successfully processed (the unsupported operations are ignored) and stored in the settings table

	// Test a config file with only whitespace in it
	UpgradeDbConfigFile(4); // the current file version is 04, replace it with 05
	DoCfgOpenTests(5); // version 05 will be successfully processed (the whitespace is ignored) and stored in the settings table

	// Test a config file with operations on an invalid table
	UpgradeDbConfigFile(5); // the current file version is 05, replace it with 06
	DoCfgOpenTests(6); // version 06 will be successfully processed (the failed-to-execute operation will be ignored) and stored in the settings table

	// Test a config file that contains an invalid comment style
	UpgradeDbConfigFile(6); // the current file version is 06, replace it with 07
	DoCfgOpenTests(7); // version 07 will be successfully processed (the line with invalid comment syntax will be ignored) and stored in the settings table

	// Test a config file that contains a sequence of statements as one statement (this is currently unsupported)
	UpgradeDbConfigFile(7); // the current file version is 07, replace it with 08
	DoCfgOpenTests(8); // version 08 will be successfully processed (the line with a sequence of statements is ignored) and stored in the settings table	

	// Test a config file that contains whitespace before and after the SQL statement
	UpgradeDbConfigFile(8); // the current file version is 08, replace it with 09
	DoCfgOpenTests(9); // version 09 will be successfully processed and stored in the settings table	

	// Test a config file that has a lower extension number
	DowngradeDbConfigFile(9); // the current file version is 09, replace it with 08
	DoCfgOpenTests(9); // version 08 will NOT be processed (as it is a lower version than that stored), and 09 will remain stored in the settings table

	// Test a config file that has an invalid extension
	CreateCorruptDbConfigFile(8); // the current file version is 08, replace it with a file with an invalid extension
	DoCfgOpenTests(9); // the invalid file will NOT be processed, and 09 will remain stored in the settings table			
	
	// Test two versions of the config file (two versions should not be present at the same time)
	CreateTwoVersionsOfConfigFile(); // the current file has an invalid extension, delete it and create version 10 and version 11
	DoCfgOpenTests(11); // only version 11 will be processed (as it is the highest version), and 11 will be stored in the settings table
	
	// Test a config file that contains an invalid statement and a valid statement
	UpgradeDbConfigFile(11); // the current file versions are 10 and 11, replace them with 12
	DoCfgOpenTests(12); // version 12 will be successfully processed (the invalid statement will be ignored and the valid statement executed) and stored in the settings table
	
	// Test a config file that contains a SQL style comment
	UpgradeDbConfigFile(12); // the current file version is 12, replace it with 13
	DoCfgOpenTests(13); // version 13 will be successfully processed (the SQL style comment will be ignored) and stored in the settings table
	
	// Test a config file that contains a 'C' style comment
	UpgradeDbConfigFile(13); // the current file version is 13, replace it with 14
	DoCfgOpenTests(14); // version 14 will be successfully processed (the 'C' style comment will be ignored) and stored in the settings table
	}
		
void DoNewDbCfgOpenTest()
	{
	// Create Db5 - a config file already exists for this database
	CreateDb5();
	
	// Open the shared, secure database Db5 - config ops should be applied on it
	TheTest.Printf(_L("===NewDbCfg: Open shared, secure database\r\n"));
	TInt err = TheDb.Open(KCfgDb5);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb5CheckNumRecords)); // there should still be only 1 record in the table
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(1)); // there should now be 1 index in the table
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb5, 1); // check that the ops in the specified config file have been applied
		
	// Open again the same shared, secure database - no config should occur (it has already been done)
	TheTest.Printf(_L("===NewDbCfg: Open shared, secure database again\r\n"));
	err = TheDb.Open(KCfgDb5);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb5CheckNumRecords)); 
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(1));
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb5, 1);
	}
	
void DoCfgAttachTests(TInt aExpectedStoredVersion = 0)
	{			
	// Open a private, secure database - no config should occur
	TheTest.Printf(_L("===CfgAttach: Open private, secure database\r\n"));
	TInt err = TheDb.Open(KCfgDb3);
	TEST2(err, KErrNone);
	// Attach a shared, secure database - the db1 config file should not be processed
	TheTest.Printf(_L("===CfgAttach: Attach shared, secure database\r\n"));
	err = TheDb.Attach(KCfgDb1, KAttachDb1);
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb1, aExpectedStoredVersion); // check that the config file has not been processed for db1
	CheckSystemSettingsTable(KCfgDb3, 0);
	
	// Open a public database - no config should occur
	TheTest.Printf(_L("===CfgAttach: Open public database\r\n"));
    err = TheDb.Open(KCfgDb4);
	TEST2(err, KErrNone);
	// Attach a shared, secure database - no config should occur (there is no config file for this database)
	TheTest.Printf(_L("===CfgAttach: Attach shared, secure database (that has no config file)\r\n"));
	err = TheDb.Attach(KCfgDb2, KAttachDb2);
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb2, 0);
	CheckSystemSettingsTable(KCfgDb4, 0);
	}
	
void DoUpgradedCfgAttachTest()
	{
	// Upgrade the config file for KCfgDb1, i.e. replace v1 file with v2 file
	UpgradeDbConfigFile(1);	
	DoCfgAttachTests(2);
	}
	
void DoBadCfgAttachTests()
	{
	// Test an empty config file
	UpgradeDbConfigFile(2); // the current file version is 02, replace it with 03
	DoCfgAttachTests(3); // version 03 will not be processed so no update to settings table
	
	// Test a config file with unsupported operations
	UpgradeDbConfigFile(3); // the current file version is 03, replace it with 04
	DoCfgAttachTests(4); // version 04 will not be processed so no update to settings table
	
	// Test a config file with only whitespace in it
	UpgradeDbConfigFile(4); // the current file version is 04, replace it with 05
	DoCfgAttachTests(5); // version 05 will not be processed so no update to settings table
	
	// Test a config file with operations on an invalid table
	UpgradeDbConfigFile(5); // the current file version is 05, replace it with 06
	DoCfgAttachTests(6); // version 06 will not be processed so no update to settings table
	
	// Test a config file that contains an invalid comment style
	UpgradeDbConfigFile(6); // the current file version is 06, replace it with 07
	DoCfgAttachTests(7); // version 07 will not be processed so no update to settings table
	
	// Test a config file that contains a sequence of statements as one statement (this is currently unsupported)
	UpgradeDbConfigFile(7); // the current file version is 07, replace it with 08
	DoCfgAttachTests(8); // version 08 will not be processed so no update to settings table
	
	// Test a config file that contains whitespace before and after the SQL statement
	UpgradeDbConfigFile(8); // the current file version is 08, replace it with 09
	DoCfgAttachTests(9); // version 09 will not be processed so no update to settings table
	
	// Test a config file that has a lower extension number
	DowngradeDbConfigFile(9); // the current file version is 09, replace it with 08
	DoCfgAttachTests(9); // version 08 will not be processed so no update to settings table
	
	// Test a config file that has an invalid extension
	CreateCorruptDbConfigFile(8); // the current file version is 08, replace it with a file with an invalid extension
	DoCfgAttachTests(9); // the invalid file will not be processed so no update to settings table
	
	// Test two versions of the config file (two versions should not be present at the same time)
	CreateTwoVersionsOfConfigFile(); // the current file has an invalid extension, delete it and create version 10 and version 11
	DoCfgAttachTests(11); // neither version 10 or 11 will be processed so no update to settings table
		
	// Test a config file that contains an invalid statement and a valid statement
	UpgradeDbConfigFile(11); // the current file versions are 10 and 11, replace them with 12
	DoCfgAttachTests(12); // version 12 will not be processed so no update to settings table

	// Test a config file that contains a SQL style comment
	UpgradeDbConfigFile(12); // the current file version is 12, replace it with 13
	DoCfgAttachTests(13); // version 13 will not be processed so no update to settings table
	
	// Test a config file that contains a 'C' style comment
	UpgradeDbConfigFile(13); // the current file version is 13, replace it with 14
	DoCfgAttachTests(14); // version 14 will not be processed so no update to settings table
	}
	
void DoNewDbCfgAttachTest()
	{
	// Create Db5 - a config file already exists for this
	CreateDb5();
	
	// Open a private, secure database - no config should occur
	TheTest.Printf(_L("===NewDbCfgAttach: Open private, secure database\r\n"));
	TInt err = TheDb.Open(KCfgDb3);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb3CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(0)); 
	TEST2(err, KErrNone);
	
	// Attach a shared, secure database - the db5 config file should not be processed
	TheTest.Printf(_L("===NewDbCfgAttach: Attach shared, secure database\r\n"));
	err = TheDb.Attach(KCfgDb5, KAttachDb5);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb5CheckNumRecords)); // there should still be only 1 record in the table
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(0)); // there should still be no indices in the table
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb5, 1); // check that the config file has been processed for db1
	CheckSystemSettingsTable(KCfgDb3, 0);
	}
	
void ResetStoredCollationDll()
	{
	// Note: We have to use SQLite directly to access the settings
	// table as the SQL Server denies permission to access this table	
	// as it is in a shared, secure database
	
	TParse parse;
	parse.Set(KCfgDb1, &KSqlSrvPrivatePath, 0);
	
	TBuf8<KMaxFileName + 1> dbFileName;
	dbFileName.Copy(parse.FullName());
	
	sqlite3 *dbHandle = NULL;
	TInt rc = sqlite3_open((const char*)dbFileName.PtrZ(), &dbHandle);
	TEST2(rc, SQLITE_OK);
	
	TBuf<100> queryBuf;
	queryBuf.Append(KResetCollationDllSql());	
	
	sqlite3_stmt* stmtHandle = NULL;
	const void* stmtTailZ = NULL;
	rc = sqlite3_prepare16_v2(dbHandle, queryBuf.PtrZ(), -1, &stmtHandle, &stmtTailZ);
	TEST2(rc, SQLITE_OK);
	
	rc = sqlite3_step(stmtHandle);
	TEST2(rc, SQLITE_DONE);
	
	sqlite3_finalize(stmtHandle);
	sqlite3_close(dbHandle);
	}
	
void CheckCollationDllUpdated()
	{
	// Note: We have to use SQLite directly to access the settings
	// table as the SQL Server denies permission to access this table
	// as it is in a shared, secure database
	
	TParse parse;
	parse.Set(KCfgDb1, &KSqlSrvPrivatePath, 0);
	
	TBuf8<KMaxFileName + 1> dbFileName;
	dbFileName.Copy(parse.FullName());
	
	sqlite3 *dbHandle = NULL;
	TInt rc = sqlite3_open((const char*)dbFileName.PtrZ(), &dbHandle);
	TEST2(rc, SQLITE_OK);
	
	TBuf<100> queryBuf;
	queryBuf.Append(KGetCollationDllSql());	
	
	sqlite3_stmt* stmtHandle = NULL;
	const void* stmtTailZ = NULL;
	rc = sqlite3_prepare16_v2(dbHandle, queryBuf.PtrZ(), -1, &stmtHandle, &stmtTailZ);
	TEST2(rc, SQLITE_OK);
	
	rc = sqlite3_step(stmtHandle);
	TEST2(rc, SQLITE_ROW);
	
	const TUint16* collationDllName = (const TUint16*)sqlite3_column_text16(stmtHandle, 0);
	TInt collationDllNameLen  = sqlite3_column_bytes(stmtHandle, 0) / sizeof(TUint16);
	TPtrC ptr(collationDllName, collationDllNameLen);
	
	rc = sqlite3_step(stmtHandle);
	TEST2(rc, SQLITE_DONE);
	
	sqlite3_finalize(stmtHandle);
	sqlite3_close(dbHandle);
	
	_LIT(KTestCollationDllName, "hjagafsff");//The same as the used in KResetCollationDllSql statement
	TEST(ptr != KTestCollationDllName);
	}

/**
@SYMTestCaseID			SYSLIB-SQL-UT-4010
@SYMTestCaseDesc		New database configuration files feature - unit test.
						The test opens several test databases, and the database configuration
						file for one of the shared, secure databases is processed. This is
						repeated for a variety of versions of the database configuration file, 
						both valid and invalid. The same tests are repeated for when attaching, 
						rather than opening, the test databases. The test also verifies that
						reindexing occurs if necessary when a database is opened or attached,
						regardless of whether or not database configuration is also required.
						Finally, the test also verifies that a config file that exists before
						the database itself is created will be processed when the database is
						opened for the first time.				
@SYMTestPriority		High
@SYMTestActions			New database configuration files feature - unit test.
@SYMTestExpectedResults The test must not fail
@SYMCR 					LMAN-79SJ7L
*/
void DoDbCfgTests()
	{
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-UT-4010 DoDbCfgTests "));
		
	// Do 'open' tests for new db config file feature
	DoCfgOpenTests();         // open the test databases
 	DoUpgradedCfgOpenTest();  // upgrade the config file for db1 and reopen the test databases
 	DoBadCfgOpenTests();      // corrupt the config file for db1 (in a variety of ways) and reopen the test databases
 	DoNewDbCfgOpenTest();		  // create a db for which a config file already exists and then open the db
 
 	// Recreate the original dbs and config files
 	DeleteCfgFilesAndDbs();
 	CreateCfgFilesAndDbs();

 	// Do 'attach' tests for new db config file feature
	DoCfgAttachTests(1);		// attach the test databases
 	DoUpgradedCfgAttachTest(); // upgrade the config file for db1 and reattach the test databases
 	DoBadCfgAttachTests();     // corrupt the config file for db1 (in a variety of ways) and reattach the test databases	
    DoNewDbCfgAttachTest();	   // create a db for which a config file already exists and then attach the db
	
	// Recreate the original dbs and config files
 	DeleteCfgFilesAndDbs();
 	CreateCfgFilesAndDbs();
 	
 	// Do the test that causes both reindexing and db configuration to occur when db1 is opened
	ResetStoredCollationDll();
	DoCfgOpenTests();
	CheckCollationDllUpdated();
	
	// Recreate the original dbs and config files
 	DeleteCfgFilesAndDbs();
 	CreateCfgFilesAndDbs();
 	
 	// Do the test that causes reindexing to occur when db1 is attached (no db configuration occurs for attach)
	ResetStoredCollationDll();
	DoCfgAttachTests(1);
	CheckCollationDllUpdated(); // db1's stored collation dll name should now match db3's (to which Db1 is attached) stored collation name
	}	
		
/**
@SYMTestCaseID			SYSLIB-SQL-UT-4013
@SYMTestCaseDesc		New database configuration files feature - OOM test.
						The test opens a shared secure database for which there
						exists a database configuration file that is to be processed.
						The open call is executed within an OOM loop, which causes
						an OOM error to occur at different stages of the open call. 
						The test verifies that OOM errors are handled correctly and 
						that on the final loop - when no OOM errors occur - the database is 
						successfully opened and the database configuration file is processed.	
@SYMTestPriority		High
@SYMTestActions			New database configuration files feature - OOM test.
@SYMTestExpectedResults The test must not fail
@SYMCR 					LMAN-79SJ7L
*/
void DoDbCfgOOMTests()
	{
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-UT-4013 DoDbCfgOOMTests "));

	// Recreate the original dbs and config files
	DeleteCfgFilesAndDbs();
	CreateCfgFilesAndDbs();
	
	// The server is stopped at the end of CreateCfgFilesAndDbs().
	// Before the test loop below begins we attach Db1 so that the 
	// server starts up again and stores Db1's security policy in
	// CSqlServer::iSecurityMap. Otherwise there will be unmatching 
	// allocated cells in the test below. Do not call Detach() before 
	// Close(). We 'attach' Db1 here rather than 'open' it because 
	// open is the API that we want to test in the while loop
	TInt err = TheDb.Open(KCfgDb3);
	TEST2(err, KErrNone);
	err = TheDb.Attach(KCfgDb1, KAttachDb1);
	TEST2(err, KErrNone);
	TheDb.Close();
	
	err = KErrNoMemory;
	TInt failingAllocationNo = 0;
	while(err == KErrNoMemory)
		{		
		MarkHandles();
		MarkAllocatedCells();
		
		__UHEAP_MARK;

		TheTest.Printf(_L("%d\r"), failingAllocationNo + 1);

		// Set failure rate
		TSqlResourceTester::SetDbHeapFailure(RHeap::EDeterministic, ++failingAllocationNo);
		err = TheDb.Open(KCfgDb1);
		if(err != KErrNoMemory)
			{
			TEST2(err, KErrNone);
			TheDb.Close();
			if(GuessSystemSettingsTable(KCfgDb1, 0))
				{
				err = KErrNoMemory;
				}
			}
		
		// Reset failure rate
		TSqlResourceTester::SetDbHeapFailure(RHeap::ENone, 0);
		
		TheDb.Close();
		
		__UHEAP_MARKEND;

		CheckAllocatedCells();	    	
		CheckHandles();	    	
		}
	TEST2(err, KErrNone);
	TheTest.Printf(_L("\r\n"));
	CheckSystemSettingsTable(KCfgDb1, 1); // check that the settings table has been updated with v01 of the config file
	}
	
/**
@SYMTestCaseID			SYSLIB-SQL-UT-4014
@SYMTestCaseDesc		New database configuration files feature - File I/O failures test.
						The test opens a shared secure database for which there
						exists a database configuration file that is to be processed.
						The open call is executed within a file I/O failure loop, which
						causes a selection of file I/O errors to occur at different stages 
						of the open call. 
						The test verifies that file I/O errors are handled correctly and 
						that on the final loop - when no file I/O errors occur - the database 
						is successfully opened and the database configuration file is processed.
						NOTE: This test also acts as the test case for defect DEF116688.
@SYMTestPriority		High
@SYMTestActions			New database configuration files feature - File I/O failures test.
@SYMTestExpectedResults The test must not fail
@SYMCR					LMAN-79SJ7L
*/
void DoDbCfgFileIOFailuresTests()
	{
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-UT-4014 DoDbCfgFileIOFailuresTests "));

	// Recreate the original dbs and config files
	DeleteCfgFilesAndDbs();
	CreateCfgFilesAndDbs();
	
	// The server is stopped at the end of CreateCfgFilesAndDbs().
	// Before the test loop below begins we start the server again
	// by opening Db3) so that any server start up file i/o ops will 
	// succeed. We don't open Db1 here because opening Db1 is what 
	// we want to test in the while loop
	TInt err = TheDb.Open(KCfgDb3);
	TEST2(err, KErrNone);
	TheDb.Close();

	TBool isFinished = EFalse;
	err = KErrNotFound;
	TInt iter = 0;
	TheTest.Printf(_L("Iteration\r\n"));
	for(TInt cnt = 1; !isFinished; ++cnt)
		{	
		for(TInt fsError = KErrNotFound; fsError >= KErrUnderflow; --fsError) // errors -1 to -10 will be generated
			{
			TheTest.Printf(_L("%d/%d   \r"), ++iter, fsError);
			(void)TheFs.SetErrorCondition(fsError, cnt); // set error code and how soon it is to occur
			err = TheDb.Open(KCfgDb1);
			(void)TheFs.SetErrorCondition(KErrNone);
			TheDb.Close();
			if(err != KErrNone)
				{
				// An error has occured. We know that this means that the database 
				// configuration part of the open call wasn't reached (because the
				// database configuration part returns KErrNone even if it fails).
				// But check anyway that the database content is still the same as
				// before, i.e. the settings table has not been updated
				CheckSystemSettingsTable(KCfgDb1, 0);
				}
			else
				{
				// Either the database configuration file for Db1 has been successfully
				// processed or it failed part-way through being processed (KErrNone is 
				// returned in this case too).
				// If it was the former then the the settings table will have 
				// been updated to store v01 of the config file
				if(GuessSystemSettingsTable(KCfgDb1, 1))
					{
					isFinished = ETrue;	
					break;
					}
				}
			}
		}
	TheTest.Printf(_L("\r\n"));
	TEST2(err, KErrNone);		
	// Check that the database configuration was allowed to be successfully applied in one of the loops
	CheckSystemSettingsTable(KCfgDb1, 1); // check that the settings table has been updated with v01 of the config file		
	}
	
/**
@SYMTestCaseID			SYSLIB-SQL-UT-4015
@SYMTestCaseDesc		New database configuration files feature - Performance test.
						The test measures:
						- the startup time of the SQL Server when 0 and 4 database 
						configuration files exist
						- the time taken to open a shared, secure database when a 
						database configuration file does and does not exist for it
						- the time taken to open a shared, secure database when a 
						database configuration file exists for it but it has already
						been processed
						- the time taken to attach a shared, secure database when a 
						database configuration file does and does not exist for it
@SYMTestPriority		High
@SYMTestActions			New database configuration files feature - Performance test.
@SYMTestExpectedResults The test must not fail
@SYMCR 					LMAN-79SJ7L
*/
void DoDbCfgPerfTests()
	{
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-UT-4015 DoDbCfgPerfTests "));

	// Recreate the original dbs
	DeleteCfgFilesAndDbs();
	CreateCfgDbs(); 
	(void)KillProcess(KSqlSrvName); // stop the server

	// Measure the start up time of the server when
	// there are no database configuration files to be cached.
	// Open Db4 (public db):
	// No reindexing required
	// Database configuration is not considered
	TUint32 start = User::FastCounter();
	TInt err = TheDb.Open(KCfgDb4);
	TUint32 end = User::FastCounter();
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb4CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(0));
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb4, 0);
	TInt ms = CalcTimeMs(start, end);
	TheTest.Printf(_L("Execution time: Server startup (via Open call) - no database config files to cache: %d ms\r\n"), ms);
	
	// Measure the time to open Db1 when no database
	// configuration file exists for it.
	// Open Db1 (shared, secure db):
	// No reindexing required
	// Database configuration is considered but no config file is found
	start = User::FastCounter();
	err = TheDb.Open(KCfgDb1);
	end = User::FastCounter();
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb1CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(0));
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb1, 0);
	ms = CalcTimeMs(start, end);
	TheTest.Printf(_L("Execution time: Open shared, secure Db1 - no config file is found: %d ms\r\n"), ms);
	
	// Measure the time to attach Db1 (database configuration will not be considered).
	// Attach Db1 (shared, secure db):
	// No reindexing required
	// Database configuration is not considered
	TheDb.Open(KCfgDb3);
	TEST2(err, KErrNone);
	start = User::FastCounter();	
	err = TheDb.Attach(KCfgDb1, KAttachDb1);
	end = User::FastCounter();
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb3, 0);
	CheckSystemSettingsTable(KCfgDb1, 0);
	ms = CalcTimeMs(start, end);
	TheTest.Printf(_L("Execution time: Attach shared, secure Db1 - database config is not considered: %d ms\r\n"), ms);
	
	// Create the 4 version 01 config files now
	CreateCfgFiles(); 
	(void)KillProcess(KSqlSrvName); // stop the server so that the files are found when it is restarted
	
	// Measure the start up time of the server when
	// there are 4 database configuration files to be cached.
	// Open Db4 (public db):
	// No reindexing required
	// Database configuration is not considered
	start = User::FastCounter();
	err = TheDb.Open(KCfgDb4);
	end = User::FastCounter();
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb4CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(0));
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb4, 0);
	ms = CalcTimeMs(start, end);
	TheTest.Printf(_L("Execution time: Server startup (via Open call) - 4 database config files to cache: %d ms\r\n"), ms);
	
	// Measure the time to open Db1 when a database
	// configuration file exists for it.
	// Open Db1 (shared, secure db):
	// No reindexing required
	// Database configuration is considered, a file is found and config is applied (3 indices are created)
	start = User::FastCounter();
	err = TheDb.Open(KCfgDb1);
	end = User::FastCounter();
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb1CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(3));
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb1, 1);
	ms = CalcTimeMs(start, end);
	TheTest.Printf(_L("Execution time: Open shared, secure Db1 - config file is found and applied: %d ms\r\n"), ms);
	
	// Measure the time to open Db1 when a database
	// configuration file exists for it but is has already been processed.
	// Open Db1 (shared, secure db):
	// No reindexing required
	// Database configuration is considered, a file is found but it has already been processed
	start = User::FastCounter();
	err = TheDb.Open(KCfgDb1);
	end = User::FastCounter();
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb1CheckNumRecords));
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(3));
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb1, 1);
	ms = CalcTimeMs(start, end);
	TheTest.Printf(_L("Execution time: Open shared, secure Db1 - config file is found but already processed: %d ms\r\n"), ms);	
	
	// Measure the time to attach Db1 (database configuration will not be considered).
	// Attach Db1 (shared, secure db):
	// No reindexing required
	// Database configuration is not considered
	TheDb.Open(KCfgDb3);
	TEST2(err, KErrNone);
	start = User::FastCounter();	
	err = TheDb.Attach(KCfgDb1, KAttachDb1);
	end = User::FastCounter();
	TEST2(err, KErrNone);
	TheDb.Close();
	CheckSystemSettingsTable(KCfgDb3, 0);
	CheckSystemSettingsTable(KCfgDb1, 1);
	ms = CalcTimeMs(start, end);
	TheTest.Printf(_L("Execution time: Attach shared, secure Db1 - database config is not considered: %d ms\r\n"), ms);
	}

void TestStatementsL()
	{
	_LIT(KDbName,					"attachDb");
	_LIT(KCreateIndex,				"CREATE INDEX idx ON tbl(ColA)");
	_LIT(KCreateIndexIfNotExists,   "CREATE INDEX IF NOT EXISTS idx ON tbl(ColA)");
	_LIT(KCreateUniqueIndex,		"CREATE UNIQUE INDEX idx ON tbl(ColA)");
	_LIT(KNonSupported,				"CREATE idx ON tbl(ColA)");
	TBuf<200> buf;

	// supported statements
	
	TBool rc = IsStatementSupported(KCreateIndex, KDbName, buf); 
	TEST(rc);
	
	rc = IsStatementSupported(KCreateIndexIfNotExists, KDbName, buf); 
	TEST(rc);

	// unsupported statements
	rc = IsStatementSupported(KCreateUniqueIndex, KDbName, buf); 
	TEST(!rc);

	rc = IsStatementSupported(KNonSupported, KDbName, buf); 
	TEST(!rc);
	}

/**
@SYMTestCaseID			SYSLIB-SQL-UT-4030
@SYMTestCaseDesc		Test IsStatementSupportedLC function.
						This function checks either SQL statement from DB configuration file is supported or not.
						In the case of supported statement, function allocates buffer and copies statement into that buffer.
						Call IsStatementSupportedLC on several supported and unsupported SQL statements.
@SYMTestPriority		High
@SYMTestActions			Test IsStatementSupportedLC function.
@SYMTestExpectedResults The test must not fail
@SYMDEF					DEF118058
*/
void DoIsStatementSupportedTests()
	{
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-UT-4030 DoIsStatementSupportedTests "));
	TInt err = KErrNone;
	TRAP(err, TestStatementsL());
	TEST2(err, KErrNone);
	}

void DoLongDbNameTest()
	{
	TheTest.Next(_L("'Long database name' tests"));
	//Create the database	
 	RSqlSecurityPolicy securityPolicy;
 	TInt err = DoCreateSecurityPolicy(securityPolicy);
 	TEST2(err, KErrNone);
 	err = TheDb.Create(KLongDbName1, securityPolicy);
 	TEST2(err, KErrNone);
 	err = TheDb.Exec(_L("CREATE TABLE table1(i1 INTEGER, i2 INTEGER, i3 INTEGER)"));
 	TEST(err >= 0);
 	err = TheDb.Exec(_L("INSERT INTO table1 (i1,i2,i3) values(1,2,3)"));
 	TEST(err == 1);
 	TheDb.Close();
 	//Kill the server (to reload config file names at the server startup)
	(void)KillProcess(KSqlSrvName);
	///////////////////////////////////////////////////////////////////////
	TheTest.Printf(_L("Open a database with a long name\r\n"));
	//Create cfg file
 	RFile file;
	TFileName fileName;
	err = file.Create(TheFs, KLongCfgName1, EFileRead | EFileWrite);
	TEST2(err, KErrNone);
	TPtrC8 pDb1((const TUint8*)KCfgConfigFileValidStmt().Ptr(), KCfgConfigFileValidStmt().Length());
	err = file.Write(pDb1);	
	file.Close();	
	TEST2(err, KErrNone);
	//Open the database
	err = TheDb.Open(KLongDbName1);
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberRecordsL(KDb1CheckNumRecords)); // there should still be only 1 record in the table
	TEST2(err, KErrNone);
	TRAP(err, CheckNumberIndicesL(1)); // there should now be 1 index in the table
	TEST2(err, KErrNone);
	TheDb.Close();
	const TInt KVersion = 1;
	CheckSystemSettingsTable(KLongDbName1, KVersion); // check that the ops in the specified config file have been applied
	///////////////////////////////////////////////////////////////////////
	TheTest.Printf(_L("Attach a database with a long logical name\r\n"));
	//Attached database test
	//Recreate the database
 	(void)RSqlDatabase::Delete(KLongDbName1);
 	err = TheDb.Create(KLongDbName1, securityPolicy);
 	TEST2(err, KErrNone);
 	err = TheDb.Exec(_L("CREATE TABLE table1(i1 INTEGER, i2 INTEGER, i3 INTEGER)"));
 	TEST(err >= 0);
 	err = TheDb.Exec(_L("INSERT INTO table1 (i1,i2,i3) values(1,2,3)"));
 	TEST(err == 1);
 	TheDb.Close();
 	//Kill the server (to reload config file names at the server startup)
	(void)KillProcess(KSqlSrvName);
	//Open the main database
 	err = TheDb.Open(KCfgDb1);
 	securityPolicy.Close();
	TEST2(err, KErrNone);
	//Attach a database with a very long logical name
	TFileName attachDbName;
	attachDbName.SetLength(KMaxFileName);
	attachDbName.Fill(TChar('A'));
 	err = TheDb.Attach(KLongDbName1, attachDbName);
	TEST2(err, KErrNone);
 	err = TheDb.Detach(attachDbName);
	TEST2(err, KErrNone);
 	TheDb.Close();
	CheckSystemSettingsTable(KLongDbName1, KVersion); // check that the ops in the specified config file have been applied
	//Cleanup
 	(void)RSqlDatabase::Delete(KLongDbName1);
	}

#endif	//SYSLIBS_TEST

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();
	TheTest(tc != NULL);
	
	__UHEAP_MARK;

#ifdef SYSLIBS_TEST	
	TheTest.Start(_L("t_sqldbconfigfile tests"));

	// Set up the test environment
	SetupTestEnv();
	//Init sqlite library
	sqlite3SymbianLibInit();

	// Do tests for database config files
	DoDbCfgTests();
	
	// Do OOM tests for database config files
	DoDbCfgOOMTests();
	
	// Do file I/O failure tests for database config files
	DoDbCfgFileIOFailuresTests();

	// Do performance tests for database config files
	DoDbCfgPerfTests();

	// Test IsStatementSupportedLC function
	DoIsStatementSupportedTests();

	//Test with a very long database file (and config file) name
	DoLongDbNameTest();
 
  	// Destroy the test environment
 	DestroyTestEnv();
	sqlite3SymbianLibFinalize();
	CloseSTDLIB();
	
	TheTest.End();
#else
 	TheTest.Start(_L("This test works only if the whole SQL component is built with SYSLIBS_TEST macro defined!"));
	TheTest.End();
#endif	
	
	__UHEAP_MARKEND;
	
	TheTest.Close();
	
	delete tc;
	
	User::Heap().Check();
	return KErrNone;
	}
