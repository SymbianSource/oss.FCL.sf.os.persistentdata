// Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
// os_symbian.cpp
// The Symbian OS porting layer - multi-threaded implementation.
// SQLite never accesses the file system and the OS services directly.
// SQLite uses for that sqlite3_vfs and sqlite3_file objects.
// sqlite3_vfs and sqlite3_file functionality is implemented in this file - 
// TVfs and TFileIo classes.
// 
//

/**
 @file
 @see TVfs
 @see TFileIo
*/

#ifdef  SQLITE_OS_SYMBIAN

extern "C" 
	{
	#include "sqliteInt.h"
	#include "os.h"
	#include "os_common.h"
	}
#include <e32math.h>
#include "os_symbian.h"
#include "UTraceSqlite.h"

#ifdef SQLITE_TEST

//Count the number of fullsyncs and normal syncs.  This is used to test
//that syncs and fullsyncs are occuring at the right times.
extern "C" int sqlite3_sync_count = 0;
extern "C" int sqlite3_fullsync_count = 0;

//The following variable, if set to a non-zero value, becomes the result
//returned from sqlite3OsCurrentTime().  This is used for testing.
extern "C" int sqlite3_current_time = 0;

#endif//SQLITE_TEST

_LIT(KCwd, ".\\");

//Used for the random numbers generation
static inline TInt64& Seed()
	{
	static TInt64 seed = 0;
	if(seed == 0)
		{
		TTime now;
		now.UniversalTime();
		seed = now.Int64();
		}
	return seed;
	}

/**
Os2SqliteErr() is called at the end of many of the interface functions of the OS porting layer (wherever it is appropriate - 
TFileIo and TVfs interfaces). The purpose of this function is to identify the "out of memory" and "disk is full" errors
reported by the used Symbian OS APIs (aOsErr parameter) and report them to SQLite as SQLITE_FULL and SQLITE_NOMEM errors.
The KErrEof error (TFileIo::Read() can return KErrEof) is reported to SQLite as SQLITE_IOERR_SHORT_READ. The rest of failures
are reported as the error specified in aDefaultErr parameter.
 
@param aOsErr      Symbian OS error
@param aDefaultErr The default SQLite error that should be used if the aOsErr parameter is not one of:
                     KErrNone, KErrEof, KErrNoMemory, KErrDiskFull
@return SQLITE_OK,               The OS porting layer function call has completed successfully, 
          SQLITE_IOERR_SHORT_READ, The amount of the data read is less than the requested amount,
          SQLITE_IOERR_NOMEM,      Out of memory,
          SQLITE_FULL,             The disk is full,
          aDefaultErr,             The rest of failures will be reported as aDefaultErr.
*/
static TInt Os2SqliteErr(TInt aOsErr, TInt aDefaultErr)
	{
	switch(aOsErr)
		{
		case KErrNone:
			return SQLITE_OK;	
		case KErrEof:
			return SQLITE_IOERR_SHORT_READ;
		case KErrNoMemory:
			return SQLITE_IOERR_NOMEM;
		case KErrDiskFull:
			return SQLITE_FULL;
		default:
#ifdef _DEBUG		
			RDebug::Print(_L("SQLite3 C API, Os2SqliteErr(), err=%d\n"), aOsErr);
#endif			
			break;
		}
	return aDefaultErr;
	}
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////  TStaticFs  /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Connects the file session used by the SQLite OS porting layer.
Single RFs instance per process is used.

@return KErrNone The operation was completed successfully,
                 System-wide error code if the operation has failed.
*/
TInt TStaticFs::Connect()
	{
	TInt err = iFs.Connect();
	if(err == KErrNone)	
		{
		err = iFs.ShareAuto();	
		}
	if(err != KErrNone)
		{
		iFs.Close();	
		}
	return err;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////  sqlite3_mutex  /////////////////////////////////////////////////////////////////////////////////////            
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Initializes sqlite3_mutex data members with their default values.
*/
sqlite3_mutex::sqlite3_mutex() :
	iRefCount(0),
	iOwnerThreadId(KMaxTUint64)
	{
	}

/**
Closes the mutex handle.
*/
sqlite3_mutex::~sqlite3_mutex()
	{
	iMutex.Close();
	}

/**
Gives the calling thread an exclusive access to the SQLite resources (global variables, file handles, buffers, cache, etc.).
The calling thread becomes a mutex owner.
If the mutex is already locked by another thread, the calling thread will block until the other thread releases the mutex.
The method can be called by the mutex owning thread more than once, even if the mutex is already entered.

@panic SqliteMt 23 Negative mutex lock counter (in debug builds only)
*/
void sqlite3_mutex::Enter()
	{
    __ASSERT_DEBUG(iRefCount >= 0, User::Panic(KPanicCategory, EPanicMutexLockCounter));
	iMutex.Wait();
	RThread currThread;
	iOwnerThreadId = currThread.Id();
	++iRefCount;
	}
	
/**
Unlocks the mutex. If sqlite3_mutex::Enter() was called more than once by the owning thread, then the number of 
sqlite3_mutex::Leave() calls must eventually match the number of sqlite3_mutex::Enter() calls.
If there are thread(s) blocked on sqlite3_mutex::Enter(), after the mutex gets unlocked one of the waiting threads
will be able to lock the mutex and get an exclusive access to the guarded resources.

@panic SqliteMt 23 Negative mutex lock counter (in debug builds only)
@panic SqliteMt 24 The mutex has been entered (locked) by a different thread than the current one (in debug builds only)
*/
void sqlite3_mutex::Leave()
	{
	__ASSERT_DEBUG(iRefCount > 0, User::Panic(KPanicCategory, EPanicMutexLockCounter));
#ifdef _DEBUG
	RThread currThread;	
	__ASSERT_DEBUG(iOwnerThreadId == currThread.Id(), User::Panic(KPanicCategory, EPanicMutexOwner));
#endif
	--iRefCount;
	iMutex.Signal();
	}

/**
Returns true if the mutex is already locked (entered).

@return True if the mutex is locked, false otherwise
*/
TBool sqlite3_mutex::IsHeld() const
	{
	RThread currThread;
	return iRefCount > 0 && iOwnerThreadId == currThread.Id();
	}

/**
Creates the mutex.

@return KErrNone The operation was completed successfully,
                 System-wide error code if the operation has failed.
*/
TInt sqlite3_mutex::Create()
	{
	return iMutex.CreateLocal();
	}

/**
Creates new CRecursiveMutex object.

@return A pointer to the created CRecursiveMutex object or NULL if the operation has failed.
*/
CRecursiveMutex* CRecursiveMutex::New()
	{
	CRecursiveMutex* self = new CRecursiveMutex;
	if(self)
		{
		if(self->Create() != KErrNone)
			{
			delete self;	
			self = NULL;
			}
		}
	return self;
	}

CRecursiveMutex::~CRecursiveMutex()
	{
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////  TMutexApi  ////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Initializes the mutex system.
No-op function.

@return SQLITE_OK
*/
int TMutexApi::Init()
	{
	SQLUTRACE_PROFILER(0);
	return SQLITE_OK;
	}
	
/**
Finalizes the mutex system.
No-op function.

@return SQLITE_OK
*/
int TMutexApi::End()
	{
	SQLUTRACE_PROFILER(0);
	return SQLITE_OK;
	}
	
/**
Creates a new mutex.
If the request is for a static mutex, a pointer to already created static mutex will be returned.

@param aType  The mutex type: static, fast, recursive
@return A pointer to the created mutex or NULL if the operation has failed
*/
sqlite3_mutex* TMutexApi::Alloc(int aType)
	{
	SQLUTRACE_PROFILER(0);
	sqlite3_mutex* mutex = NULL;
	switch(aType)
		{
		case SQLITE_MUTEX_FAST:
		case SQLITE_MUTEX_RECURSIVE:
			mutex = CRecursiveMutex::New();
			break;
		default:
			mutex = ::StaticMutex(aType - 2);//"aType - 2" because the first SQLITE_MUTEX_STATIC_<type> mutex definition 
			//value is 2 (SQLITE_MUTEX_FAST is 0, SQLITE_MUTEX_RECURSIVE is 1). 
			break;	
		}
	return mutex;
	}
	
/**
Destroys a mutex, created previously by a call to TMutexApi::Alloc().
@param aMutex Pointer to the mutex object that has to be destroyed
*/
void TMutexApi::Free(sqlite3_mutex* aMutex)
	{
	SQLUTRACE_PROFILER(0);
	delete aMutex;
	}
	
/**
Locks the mutex.
See sqlite3_mutex::Enter() for more details.

@param aMutex Pointer to the mutex object

@see sqlite3_mutex::Enter()
*/
void TMutexApi::Enter(sqlite3_mutex* aMutex)
	{
	SQLUTRACE_PROFILER(0);
	aMutex->Enter();
	}
	
/**
No-op. Always returns SQLITE_BUSY.

@return SQLITE_BUSY
*/
int TMutexApi::Try(sqlite3_mutex*)
	{
	SQLUTRACE_PROFILER(0);
	return SQLITE_BUSY;
	}
	
/**
Unlocks the mutex.
See sqlite3_mutex::Leave() for more details.

@param aMutex Pointer to the mutex object

@see sqlite3_mutex::Leave()
*/
void TMutexApi::Leave(sqlite3_mutex* aMutex)
	{
	SQLUTRACE_PROFILER(0);
	aMutex->Leave();
	}
	
/**
Checks whether the mutex is locked or not.
See sqlite3_mutex::IsHeld() for more details.

@param aMutex Pointer to the mutex object

@return True if the mutex is locked, false otherwise

@see sqlite3_mutex::IsHeld()
*/
int TMutexApi::Held(sqlite3_mutex* aMutex)
	{
	SQLUTRACE_PROFILER(0);
	return aMutex->IsHeld();
	}
	
/**
Checks whether the mutex is locked or not.
See sqlite3_mutex::IsHeld() for more details.

@param aMutex Pointer to the mutex object

@return False if the mutex is locked, true otherwise

@see sqlite3_mutex::IsHeld()
*/
int TMutexApi::Notheld(sqlite3_mutex* aMutex)
	{
	SQLUTRACE_PROFILER(0);
	return !aMutex->IsHeld();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////       SQLite init/release functions     ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Initializes the OS porting layer global data.
*/
extern "C" SQLITE_EXPORT int sqlite3_os_init(void)
	{
	return sqlite3_vfs_register(VfsApi(), 1);
	}

/**
Destroys the OS porting layer global data.
*/
extern "C" SQLITE_EXPORT int sqlite3_os_end(void)
	{
	return sqlite3_vfs_unregister(VfsApi());
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////  TheFileIoApi  /////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Single sqlite3_io_methods instance, which data members (function pointers) are initialized with the addresses of
TFileIo members. 
TheFileIoApi is used by SQLite for performing OS independent file I/O.

@see TFileIo
@see TVfs

@internalComponent
*/
const static sqlite3_io_methods TheFileIoApi = 
	{
	1,						//Version
	&TFileIo::Close,
	&TFileIo::Read,
	&TFileIo::Write,
	&TFileIo::Truncate,
	&TFileIo::Sync,
	&TFileIo::FileSize,
	&TFileIo::Lock,
	&TFileIo::Unlock,
	&TFileIo::CheckReservedLock,
	&TFileIo::FileControl,
	&TFileIo::SectorSize,
	&TFileIo::DeviceCharacteristics
	};
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////  TheMutexMethods  //////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
sqlite3_mutex_methods is a structure declared in sqlite3.h header file. 
sqlite3_mutex_methods defines the mutex interface used by SQLite and implemented by the OS porting layer.
*/
static sqlite3_mutex_methods TheMutexMethods =
	{
	&TMutexApi::Init,
	&TMutexApi::End,
	&TMutexApi::Alloc,
	&TMutexApi::Free,
	&TMutexApi::Enter,
	&TMutexApi::Try,
	&TMutexApi::Leave,
	&TMutexApi::Held,
	&TMutexApi::Notheld
	};

extern "C" sqlite3_mutex_methods* sqlite3DefaultMutex(void)
	{
	return &TheMutexMethods;
	};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////          UTF16<-->UTF8, conversion functions    ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
The function converts aFileName to UTF16 encoded file name, and stores the UTF16 encoded file name
to the place pointed by aFileNameDestBuf argument.
If the UTF16 conversion of the file name failed because the file name is too long or NULL, 
the function returns EFalse. 

@param aFileName Expected to point to UTF8 encoded, zero terminated string.
				 Max allowed aFileName length is KMaxFileName (excluding terminating 0 character).
@param aFileNameDestBuf Output parameter. Will hold UTF16, non-zero-terminated string.
						The max length must be at least KMaxFileName characters.
                         
@return True if the conversion has been completed successfully						 
*/
static TBool ConvertToUnicode(const char *aFileName, TDes& aFileNameDestBuf)
	{
	if(aFileName)
		{
		wchar_t* dest = reinterpret_cast <wchar_t*> (const_cast <TUint16*> (aFileNameDestBuf.Ptr()));
		TInt len = mbstowcs(dest, aFileName, KMaxFileName);
		//Check the file name length. If it is longer than KMaxFileName characters, then the file name is not valid.
		if(len > 0 && len <= KMaxFileName)
			{
			aFileNameDestBuf.SetLength(len);
			return ETrue;
			}
		}
	return EFalse;
	}

/**
The function converts aFileName to UTF8 encoded file name, and stores the UTF8 encoded file name
to the place pointed by aFileNameDestBuf argument.
If the UTF8 conversion of the file name failed because the file name is too long or NULL, 
the function returns EFalse. 

@param aFileName Expected to point to UTF16 encoded, zero terminated string.
				 Max allowed aFileName length is KMaxFileName (excluding terminating 0 character).
@param aFileNameDestBuf Output parameter. Will hold UTF8, non-zero-terminated string.
						The max length must be at least KMaxFileName characters.
                         
@return True if the conversion has been completed successfully						 
*/
static TBool ConvertFromUnicode(const TDesC& aFileName, TDes8& aFileNameDestBuf)
	{
	char* dest = reinterpret_cast <char*> (const_cast <TUint8*> (aFileNameDestBuf.Ptr()));
	const wchar_t* src = reinterpret_cast <const wchar_t*> (aFileName.Ptr());
	TInt len = wcstombs(dest, src, KMaxFileName);
	//Check the file name length. If it is longer than KMaxFileName characters, then the file name is not valid.
	if(len > 0 && len <= KMaxFileName)
		{
		aFileNameDestBuf.SetLength(len);
		return ETrue;
		}
	return EFalse;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////       TDbFile class definition    ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const TInt KFileBufSize = 8 * 1024;	

/**
Initializes TDbFile data members with their default values.
*/
inline TDbFile::TDbFile() :
	iFileBuf(KFileBufSize),
	iFullName(NULL),
	iSharedLockByte(0),
	iLockType(SQLITE_LOCK_NONE),
	iReadOnly(EFalse),
	iSectorSize(0),
	iDeviceCharacteristics(-1)
	{
	pMethods = 0;
	}

/**
Casts the passed sqlite3_file pointer to a reference to the derived class - TDbFile&.
All sqlite3_file pointers passed to TFileIo methods are actually pointers to TDbFile instances. 
So the cast is safe.

@param aDbFile A pointer to a sqlite3_file instance

@return A TDbFile reference. 
@see TFileIo
@see TVfs
@see TDbFile

@panic Sqlite 20 In _DEBUG mode if aDbFile is NULL.

@internalComponent
*/
static inline TDbFile& DbFile(sqlite3_file* aDbFile)
	{
	__ASSERT_DEBUG(aDbFile != 0, User::Panic(KPanicCategory, EPanicNullDbFilePtr));
	return *(static_cast <TDbFile*> (aDbFile));
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////       TFileIo class definition    ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
SQLite OS porting layer API.

Closes the file referred by aDbFile parameter.
If aDbFile.iFullName data member is not NULL, then the file will be deleted.

@param aDbFile A pointer to a TDbFile instance, than contains the file handle to be closed.

@return SQLITE_OK

@see TDbFile
*/
/* static */ int TFileIo::Close(sqlite3_file* aDbFile)
	{
	SQLUTRACE_PROFILER(aDbFile);
	TDbFile& dbFile = ::DbFile(aDbFile);
	dbFile.iFileBuf.Close();	
	if(dbFile.iFullName)
        {//"iFullName" will not be NULL only when TVfs::Open() is called with SQLITE_OPEN_DELETEONCLOSE flag.
         //That means - SQlite expects the file to be deleted after the file close operation. 
		(void)TStaticFs::Fs().Delete(*dbFile.iFullName);
		delete dbFile.iFullName;
		dbFile.iFullName = NULL;
		}
    OpenCounter(-1);
	return SQLITE_OK;
	}

/**
SQLite OS porting layer API.

Reads from the file referred by the aDbFile parameter.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle to be read from.
@param aBuf Output parameter. The data read from the file will be copied there.
			The buffer size must be at least aAmt bytes.
@param aAmt The amount of data to be read form the file.
@param aOffset The offset in the file where the read operation should start.

@return SQLITE_FULL,       			The disk is full,
	    SQLITE_IOERR_SHORT_READ, 	The amount of the data read is less than aAmt,
	    SQLITE_IOERR_READ, 			File read error,
	    SQLITE_IOERR_NOMEM,			An out of memory condition has occured,
	    SQLITE_OK,					The operation has completed successfully.
	    
@see TDbFile
*/
/* static */ int TFileIo::Read(sqlite3_file* aDbFile, void* aBuf, int aAmt, sqlite3_int64 aOffset)
	{
	SQLUTRACE_PROFILER(aDbFile);
	SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KFileRead, aAmt, aOffset));
	SimulateIOError(return SQLITE_IOERR_READ);
	TDbFile& dbFile = ::DbFile(aDbFile);
	TPtr8 ptr((TUint8*)aBuf, 0, aAmt);
	TInt err = dbFile.iFileBuf.Read(aOffset, ptr);
	TInt cnt = ptr.Length();
	TInt sqliteErr = ::Os2SqliteErr(err, SQLITE_IOERR_READ);
	if(cnt != aAmt && (sqliteErr == SQLITE_OK || sqliteErr == SQLITE_IOERR_SHORT_READ))
		{
		Mem::FillZ(static_cast <TUint8*> (aBuf) + cnt, aAmt - cnt);
		sqliteErr = SQLITE_IOERR_SHORT_READ;
		}
	return sqliteErr;
	}

/**
SQLite OS porting layer API.

Writes to the file referred by the aDbFile parameter.
"Write beyond the end of the file" operations are allowed.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle to be written to.
@param aData The data to be written to the file. The buffer size must be at least aAmt bytes.
@param aAmt The amount of data to be written to the file.
@param aOffset The offset in the file where the write operation should start.

@return SQLITE_IOERR_WRITE, the file write operation has failed or the file is read-only,
		SQLITE_FULL,       	The disk is full,
	    SQLITE_IOERR_NOMEM,	An out of memory condition has occured,
	    SQLITE_OK,			The operation has completed successfully.
	    
@see TDbFile
*/
/* static */ int TFileIo::Write(sqlite3_file* aDbFile, const void* aData, int aAmt, sqlite3_int64 aOffset)
	{
	SQLUTRACE_PROFILER(aDbFile);
	SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KFileWrite, aAmt, aOffset));
	SimulateIOError(return SQLITE_IOERR_WRITE);
	SimulateDiskfullError(return SQLITE_FULL);
	TDbFile& dbFile = ::DbFile(aDbFile);
	TInt err = KErrAccessDenied;
	if(!dbFile.iReadOnly)
		{
		TPtrC8 ptr((const TUint8*)aData, aAmt);
		err = dbFile.iFileBuf.Write(aOffset, ptr);
		}
	return ::Os2SqliteErr(err, SQLITE_IOERR_WRITE);
	}

/**
SQLite OS porting layer API.

Truncates the file referred by the aDbFile parameter.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.
@param aLength The new file size in bytes.

@return SQLITE_IOERR_TRUNCATE, the file truncate operation has failed or the file is read-only,
		SQLITE_FULL,       	The disk is full,
	    					The file truncate operation has failed,
	    SQLITE_IOERR_NOMEM,	An out of memory condition has occured,
	    SQLITE_OK,			The operation has completed successfully.
	    
@see TDbFile
*/
/* static */ int TFileIo::Truncate(sqlite3_file* aDbFile, sqlite3_int64 aLength)
	{
	SQLUTRACE_PROFILER(aDbFile);
	SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KFileTruncate, aLength));
	SimulateIOError(return SQLITE_IOERR_TRUNCATE);
	TDbFile& dbFile = ::DbFile(aDbFile);
	TInt err = KErrAccessDenied;
	if(!dbFile.iReadOnly)
		{
		err = dbFile.iFileBuf.SetSize(aLength);
		}
	return ::Os2SqliteErr(err, SQLITE_IOERR_TRUNCATE);
	}

/**
SQLite OS porting layer API.

Flushes the file referred by the aDbFile parameter.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.
@param aFlags  This parameter is not used in the production builds. It may be one of 
			   SQLITE_SYNC_NORMAL or SQLITE_SYNC_FULL and is used only by the TCL test suite.

@return SQLITE_IOERR_FSYNC,	This is a read-only file, or  the file flush operation has failed,
	    SQLITE_IOERR_NOMEM,	An out of memory condition has occured,
	    SQLITE_FULL,       		The disk is full,
   	    SQLITE_OK,			The operation has completed successfully.

@see TDbFile
*/
/* static */int TFileIo::Sync(sqlite3_file* aDbFile, int aFlags)
	{
	SQLUTRACE_PROFILER(aDbFile);
	SimulateIOError(return SQLITE_IOERR_FSYNC);
	TDbFile& dbFile = ::DbFile(aDbFile);
#ifdef SQLITE_TEST
	if(aFlags & SQLITE_SYNC_FULL)
		{
		sqlite3_fullsync_count++;
		}
	sqlite3_sync_count++;
#else
	aFlags = aFlags;	
#endif
	TInt err = KErrAccessDenied;
	if(!dbFile.iReadOnly)
		{
		err = dbFile.iFileBuf.Flush();
		}
	return ::Os2SqliteErr(err, SQLITE_IOERR_FSYNC);
	}

/**
SQLite OS porting layer API.

Returns the size of the file referred by the aDbFile parameter.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.
@param aSize Output parameter. If the function completes successfully, the file size will be stored there.

@return SQLITE_IOERR_FSTAT,		The file size operation has failed;
	    SQLITE_IOERR_NOMEM,		An out of memory condition has occured;
	    SQLITE_OK,				The operation has completed successfully.
	    
@see TDbFile
*/
/* static */ int TFileIo::FileSize(sqlite3_file* aDbFile, sqlite3_int64* aSize)
	{
	SQLUTRACE_PROFILER(aDbFile);
	SimulateIOError(return SQLITE_IOERR_FSTAT);
	TDbFile& dbFile = ::DbFile(aDbFile);
	TInt err =  dbFile.iFileBuf.Size(*aSize);
	return ::Os2SqliteErr(err, SQLITE_IOERR_FSTAT);
	}

/**
This function is called when SQLite needs to obtain a read lock. This is done by generating a
random file position within the first page beyond the first Gb of the file and locking a single byte there.
There is a possible problem with that random file position, because the database file may be shared between multiple
connections. That increases the possibility of generating the same "random" file position by different connections to the
same file. In order to minimise that, TFileIo::GetReadLock() will generate up to 3 different file positions in a case of
a "lock byte" failure. 
The generated file position will be stored in TDbFile::iSharedLockByte data member and will be used later for the 
unlock operation.

@param aDbFile The Os porting layer file handle
@return KErrNone 	The locking operation has completed successfully,
		KErrLocked	The 1 byte file area that begins from the generated file position is already locked,
				    Some other system-wide error codes in a case of  failure.

@see TFileIo::UnlockReadLock()
*/
/* static */TInt TFileIo::GetReadLock(TDbFile& aDbFile)
	{
	const TInt KLockTryCount = 3;
	TInt rc = KErrLocked;
	for(TInt i=0;i<KLockTryCount;++i)
		{
	    TInt lock = Math::Rand(Seed());
	    //Explanation regarding how the file locking works can be found in os.h file, lines 279-335.
	    //Shortly, in order to read pages from the database the calling thread must obtain a shared lock.
	    //This is done locking a randomly chosen byte - iSharedLockByte.
	    //The calculation of iSharedLockByte is done in a way that:
	    // - All calculated iSharedLockByte fit on a single page, even if the page size is chosen to be the smallest one possible.
	    //       That's why the "% (SHARED_SIZE - 1)" is used in the calculation;
	    // - The locked byte cannot be used for storing data. That is the reason SHARED_FIRST to be set to be a position beyond the
	    //       1Gb boundary;
	    TInt sharedLockByte = (lock & 0x7fffffff) % (SHARED_SIZE - 1);
	    rc = aDbFile.iFileBuf.Lock(SHARED_FIRST + sharedLockByte, 1);
	    if(rc == KErrNone)
	    	{
	    	aDbFile.iSharedLockByte = sharedLockByte;
	    	break;
	    	}
		}
	return rc;
	}

/**
Unlocks the file area previously locked by the GetReadLock() call.
The beginning of the locked area with length 1 byte is stored in TDbFile::iSharedLockByte data member.

@param aDbFile The Os porting layer file handle

@return KErrNone 	The locking operation has completed successfully,
				    Some other system-wide error codes in a case of  failure.

@see TFileIo::GetReadLock()
*/
/* static */TInt TFileIo::UnlockReadLock(TDbFile& aDbFile)
	{
	return aDbFile.iFileBuf.UnLock(SHARED_FIRST + aDbFile.iSharedLockByte, 1);
	}

/**
SQLite OS porting layer API.

Locks the file, referred by the aDbFile parameter, with the specified lock type.
The file lock type is stored for later use by the CheckReservedLock() call.

Sometimes when requesting one lock state, additional lock states
are inserted in between.  The locking might fail on one of the later
transitions leaving the lock state different from what it started but
still short of its goal.  The following chart shows the allowed
transitions and the inserted intermediate states:

SQLITE_LOCK_NONE		-> SQLITE_LOCK_SHARED
SQLITE_LOCK_SHARED 		-> SQLITE_LOCK_RESERVED
SQLITE_LOCK_SHARED 		-> (SQLITE_LOCK_PENDING) 	-> 	SQLITE_LOCK_EXCLUSIVE
SQLITE_LOCK_RESERVED 	-> (SQLITE_LOCK_PENDING) 	-> 	SQLITE_LOCK_EXCLUSIVE
SQLITE_LOCK_PENDING 	-> SQLITE_LOCK_EXCLUSIVE

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.
@param aLockType Lock type: SQLITE_LOCK_NONE, SQLITE_LOCK_SHARED, SQLITE_LOCK_RESERVED, SQLITE_LOCK_PENDING or
				 SQLITE_LOCK_EXCLUSIVE.

@return SQLITE_IOERR_NOMEM,	An out of memory condition has occured;
	    SQLITE_BUSY,	    The requested lock cannot be obtained;
	    SQLITE_LOCK,		File locking error,
	    SQLITE_OK,			The operation has completed successfully.

@see TFileIo::CheckReservedLock()
@see TFileIo::Unlock()
	    
@see TDbFile
*/
/* static */ int TFileIo::Lock(sqlite3_file* aDbFile, int aLockType)
	{
	SQLUTRACE_PROFILER(aDbFile);
	TDbFile& dbFile = ::DbFile(aDbFile);
	//If there is already a lock of this type or more restrictive on the aDbFile, then - do nothing.
	if(dbFile.iLockType >= aLockType)
		{
		return SQLITE_OK;
		}

	//The file flushing here must be done in order to get the file buffer object content (iFileBuf data member))
	//synchronised with the database file content (the database file content may get modified by a different connection
	//at the same time).
	if(aLockType == SQLITE_LOCK_SHARED && !dbFile.iReadOnly)
		{
		TInt err = dbFile.iFileBuf.Flush(ETrue);
		if(err != KErrNone)
			{
			return ::Os2SqliteErr(err, SQLITE_IOERR_LOCK);
			}
		}

	//Make sure the locking sequence is correct
	__ASSERT_DEBUG(dbFile.iLockType != SQLITE_LOCK_NONE || aLockType == SQLITE_LOCK_SHARED, User::Panic(KPanicCategory, EPanicInvalidLock));
	__ASSERT_DEBUG(aLockType != SQLITE_LOCK_PENDING, User::Panic(KPanicCategory, EPanicInvalidLock));
	__ASSERT_DEBUG(aLockType != SQLITE_LOCK_RESERVED || dbFile.iLockType == SQLITE_LOCK_SHARED, User::Panic(KPanicCategory, EPanicInvalidLock));
		
	TInt rc = SQLITE_OK;    //Return code from subroutines
	TBool locked = ETrue;   //Result of a file lock call (the default value means: "lock accuired")
  	TInt newLockType = -1;	//Set dbFile.iLockType to this value before exiting
	TBool gotPendingLock = EFalse;//True if we acquired a SQLITE_LOCK_PENDING lock this time

	//Lock the SQLITE_LOCK_PENDING byte if we need to acquire a SQLITE_LOCK_PENDING lock or
	//SQLITE_LOCK_SHARED lock. If we are acquiring a SQLITE_LOCK_SHARED lock, the acquisition of
	//the SQLITE_LOCK_PENDING byte is temporary.
	newLockType = dbFile.iLockType;
	if(dbFile.iLockType == SQLITE_LOCK_NONE || (aLockType == SQLITE_LOCK_EXCLUSIVE && dbFile.iLockType == SQLITE_LOCK_RESERVED))
		{
		//Try 3 times to get the pending lock.  The pending lock might be
		//held by another reader process who will release it momentarily.
		const TInt KLockTryCnt = 3;
		locked = EFalse;
		for(TInt i=0;i<KLockTryCnt && !locked;++i)
			{
			TInt err = dbFile.iFileBuf.Lock(PENDING_BYTE, 1);
			if(err != KErrNone && err != KErrLocked) 
				{
				return ::Os2SqliteErr(err, SQLITE_IOERR_LOCK);
				}
			locked = (err == KErrNone);
   			if(!locked)
   				{
				const TInt KMs = 10;
				TVfs::Sleep(NULL, KMs * 1000);
   				}
			}
		gotPendingLock = locked;
		}

	//Acquire a shared lock
	if(aLockType == SQLITE_LOCK_SHARED && locked)
		{
		__ASSERT_DEBUG(dbFile.iLockType == SQLITE_LOCK_NONE, User::Panic(KPanicCategory, EPanicInvalidLock));
		TInt err = TFileIo::GetReadLock(dbFile);
		if(err != KErrNone && err != KErrLocked) 
			{
			return ::Os2SqliteErr(err, SQLITE_IOERR_LOCK);
			}
		locked = (err == KErrNone);
		if(locked)
			{
			newLockType = SQLITE_LOCK_SHARED;
			}
  		}

	//Acquire a RESERVED lock
	if(aLockType == SQLITE_LOCK_RESERVED && locked)
		{
		__ASSERT_DEBUG(dbFile.iLockType == SQLITE_LOCK_SHARED, User::Panic(KPanicCategory, EPanicInvalidLock));
		TInt err = dbFile.iFileBuf.Lock(RESERVED_BYTE, 1); 
		if(err != KErrNone && err != KErrLocked) 
			{
			return ::Os2SqliteErr(err, SQLITE_IOERR_LOCK);
			}
		locked = (err == KErrNone);
		if(locked)
			{
			newLockType = SQLITE_LOCK_RESERVED;
			}
		}

	// Acquire a PENDING lock
	if(aLockType == SQLITE_LOCK_EXCLUSIVE && locked)
		{
		newLockType = SQLITE_LOCK_PENDING;
		gotPendingLock = EFalse;
		}

	//Acquire an EXCLUSIVE lock
	if(aLockType == SQLITE_LOCK_EXCLUSIVE && locked)
		{
		__ASSERT_DEBUG(dbFile.iLockType >= SQLITE_LOCK_SHARED, User::Panic(KPanicCategory, EPanicInvalidLock));
		(void)TFileIo::UnlockReadLock(dbFile);
		TInt err = dbFile.iFileBuf.Lock(SHARED_FIRST, SHARED_SIZE);
		if(err != KErrNone && err != KErrLocked)
			{
			return ::Os2SqliteErr(err, SQLITE_IOERR_LOCK);
			}
		locked = (err == KErrNone);
		if(locked)
			{
			newLockType = SQLITE_LOCK_EXCLUSIVE;
			}
		}

	// If we are holding a PENDING lock that ought to be released, then
	// release it now.
	if(gotPendingLock && aLockType == SQLITE_LOCK_SHARED)
		{
		(void)dbFile.iFileBuf.UnLock(PENDING_BYTE, 1);
  		}

	// Update the state of the lock has held in the file descriptor then
	// return the appropriate result code.
	rc = locked ? SQLITE_OK : SQLITE_BUSY;
	dbFile.iLockType = newLockType;
	return rc;
	}

/**
SQLite OS porting layer API.

Lower the locking level on file descriptor id to locktype.  locktype
must be either SQLITE_LOCK_NONE or SQLITE_LOCK_SHARED.

If the locking level of the file descriptor is already at or below
the requested locking level, this routine is a no-op.

It is not possible for this routine to fail if the second argument
is SQLITE_LOCK_NONE. If the second argument is SQLITE_LOCK_SHARED then this routine
might return SQLITE_IOERR;

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.
@param aLockType Lock type: SQLITE_LOCK_NONE, SQLITE_LOCK_SHARED, SQLITE_LOCK_RESERVED, SQLITE_LOCK_PENDING or
				 SQLITE_LOCK_EXCLUSIVE.

@return SQLITE_OK,	 		 The operation has completed successfully,
	     SQLITE_IOERR_UNLOCK, The unlock operation has failed. 

@see TFileIo::CheckReservedLock()
@see TFileIo::Lock()
	    
@see TDbFile
*/
/* static */ int TFileIo::Unlock(sqlite3_file* aDbFile, int aLockType)
	{
	__ASSERT_DEBUG(aLockType <= SQLITE_LOCK_SHARED, User::Panic(KPanicCategory, EPanicInvalidLock));
	
	SQLUTRACE_PROFILER(aDbFile);
	TDbFile& dbFile = ::DbFile(aDbFile);
	TInt rc = SQLITE_OK;
	TInt currLockType = dbFile.iLockType;
	
	if(currLockType >= SQLITE_LOCK_EXCLUSIVE)
		{
		(void)dbFile.iFileBuf.UnLock(SHARED_FIRST, SHARED_SIZE);
		if(aLockType == SQLITE_LOCK_SHARED)
    		{
			TInt err = TFileIo::GetReadLock(dbFile); 
			if(err != KErrNone && err != KErrLocked)
				{
				return ::Os2SqliteErr(err, SQLITE_IOERR_UNLOCK);
				}
			if(err == KErrLocked)
				{
				//This should never happen. We should always be able to reacquire the read lock
				rc = SQLITE_IOERR_UNLOCK;
				}
			}
		}
	if(currLockType >= SQLITE_LOCK_RESERVED)
		{
    	(void)dbFile.iFileBuf.UnLock(RESERVED_BYTE, 1);
		}
	if(aLockType == SQLITE_LOCK_NONE && currLockType >= SQLITE_LOCK_SHARED)
		{
		(void)TFileIo::UnlockReadLock(dbFile);
		}
	if(currLockType>= SQLITE_LOCK_PENDING)
		{
		(void)dbFile.iFileBuf.UnLock(PENDING_BYTE, 1);
		}
		
	dbFile.iLockType = aLockType;
	return rc;
	}

/**
SQLite OS porting layer API.

Checks if the file lock type is SQLITE_LOCK_RESERVED or bigger.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.
@param aResOut Output parameter. It will be set to be non-zero if the stored lock type is bigger or equal 
							     than SQLITE_LOCK_RESERVED.

@return SQLITE_IOERR_CHECKRESERVEDLOCK, The operation has failed,
	    SQLITE_OK 					    The operation has completed successfully.

@see TFileIo::Lock()
@see TFileIo::Unlock()
	    
@see TDbFile
*/
/* static */ int TFileIo::CheckReservedLock(sqlite3_file* aDbFile, int *aResOut)
	{
	SQLUTRACE_PROFILER(aDbFile);
	TDbFile& dbFile = ::DbFile(aDbFile);
	TInt rc;
	if(dbFile.iLockType >= SQLITE_LOCK_RESERVED)
		{
		rc = 1;
  		}
	else
		{
		TInt err = dbFile.iFileBuf.Lock(RESERVED_BYTE, 1);
		if(err != KErrNone && err != KErrLocked)
			{
			return ::Os2SqliteErr(err, SQLITE_IOERR_CHECKRESERVEDLOCK);
			}
		rc = (err == KErrNone);
		if(rc) //non-zero rc means: the lock has been successful (there wasn't a reserved lock on this file)
			{
			(void)dbFile.iFileBuf.UnLock(RESERVED_BYTE, 1);
			}
    	rc = !rc;
		}
	*aResOut = rc;
	return SQLITE_OK;
	}

/**
SQLite OS porting layer API.

Performs an aOp operation on the file referred by the aDbFile parameter.
Since the only supported operation at the moment is SQLITE_FCNTL_LOCKSTATE, and the current lock type is stored as
a data memebr of TDbFile, the function implementation has been optimised - no file I/O calls. The stored file lock type
is retured if the operation is SQLITE_FCNTL_LOCKSTATE.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.
@param aOp File operation type. Currently only SQLITE_FCNTL_LOCKSTATE is supported.
@param aArg An additional input/output parameter which purpose depends on the type of the current file operation.
			If the file operation is SQLITE_FCNTL_LOCKSTATE, then aArg is used as an output parameter, where
			the file lock type is stored.

@return SQLITE_ERROR,	Non-supported operation,
		SQLITE_OK,	The operation has completed successfully.
	    
@see TDbFile
*/
/* static */ int TFileIo::FileControl(sqlite3_file* aDbFile, int aOp, void* aArg)
	{
	SQLUTRACE_PROFILER(aDbFile);
	TDbFile& dbFile = ::DbFile(aDbFile);
	SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KFileFileCtr, aOp, dbFile.iFullName));
	TInt err = KErrNone;
	switch(aOp)
		{
		case SQLITE_FCNTL_LOCKSTATE:
			*(int*)aArg = dbFile.iLockType;
			break;
		default:
			err = KErrArgument;
			break;
		}
	return err == KErrNone ? SQLITE_OK : SQLITE_ERROR;
	}

/**
SQLite OS porting layer API.

Retrieves the sector size of the media of the file referred by the aDbFile parameter.
Since the sector size never changes till the file is open, the function has been optimised - no file I/O calls.
The sector size is retrieved during the TVfs::Open() call and stored in TDbFile::iSectorSize. The SectorSize()
call returns the value of TDbFile::iSectorSize.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.

@return The sector size.

@panic Sqlite 19 In _DEBUG mode - TDbFile::iSectorSize is negative or 0 .
	    
@see TDbFile
@see TVfs::Open()
*/
/* static */ int TFileIo::SectorSize(sqlite3_file* aDbFile)
	{
	SQLUTRACE_PROFILER(aDbFile);
	TDbFile& dbFile = ::DbFile(aDbFile);
	__ASSERT_DEBUG(dbFile.iSectorSize > 0, User::Panic(KPanicCategory, EPanicInternalError));
	if(dbFile.iSectorSize > 0)
		{
		return dbFile.iSectorSize;	
		}
	return SQLITE_DEFAULT_SECTOR_SIZE;
	}

/**
SQLite OS porting layer API.

Retrieves the device characteristics of the device of the file referred by the aDbFile parameter.
Since the device characteristics never change till the file is open, the function has been optimised - no file I/O calls.
The device characteristics are retrieved during the TVfs::Open() call and stored in TDbFile::iDeviceCharacteristics. 
The DeviceCharacteristics() call returns the value of TDbFile::iDeviceCharacteristics.

@param aDbFile A pointer to a TDbFile instance, that contains the file handle.

@return A bit set containing the device characteristics.
	    
@panic Sqlite 19 In _DEBUG mode - TDbFile::iDeviceCharacteristics is negative or 0 .

@see TDbFile
@see TVfs::Open()
*/
/* static */ int TFileIo::DeviceCharacteristics(sqlite3_file* aDbFile)
	{
	SQLUTRACE_PROFILER(aDbFile);
	TDbFile& dbFile = ::DbFile(aDbFile);
	__ASSERT_DEBUG(dbFile.iDeviceCharacteristics >= 0, User::Panic(KPanicCategory, EPanicInternalError));
	if(dbFile.iDeviceCharacteristics >= 0)
		{
		return dbFile.iDeviceCharacteristics;	
		}
	return 0;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////       TVfs class definition     ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Collects information about the drive referred by the aDriveNo parameter.

@param aFs			RFs instance.
@param aDriveNo     The drive about which an information will be collected.
@param aVolumeInfo	Output parameter. A reference to a TVolumeIOParamInfo object where the collected information will be stored.

@return KErrNone,          The operation has completed succesfully;
	    KErrNoMemory,      Out of memory condition has occured;
                           Note that other system-wide error codes may also be returned.
	    
@see TVfs::Open()
*/
/* static */ inline TInt TVfs::DoGetVolumeIoParamInfo(RFs& aFs, TInt aDriveNo, TVolumeIOParamInfo& aVolumeInfo)
	{
	return aFs.VolumeIOParam(aDriveNo, aVolumeInfo);
	}

/**
Retrieves and returns in a bit set the device characteristics.

@param aDriveInfo	A TDriveInfo reference from which the device characteristics will be extracted.
@param aVolumeInfo	A TVolumeIOParamInfo reference from which the device characteristics will be extracted.

@return A bit set containing the device characteristics: 
			SQLITE_IOCAP_SAFE_APPEND, SQLITE_IOCAP_ATOMIC, the atomic block size.
	    
@see TVfs::DoGetDriveInfo();
@see TVfs::DoGetVolumeIoParamInfo();
@see TVfs::Open()
*/
/* static */ TInt TVfs::DoGetDeviceCharacteristics(const TDriveInfo& aDriveInfo, const TVolumeIOParamInfo& aVolumeInfo)
	{
	TInt deviceCharacteristics = 0;	
	if(aDriveInfo.iDriveAtt & (KDriveAttLocal | KDriveAttInternal))
		{
		deviceCharacteristics |= SQLITE_IOCAP_SAFE_APPEND;//Data written first, file size updated second
		}
	if(aDriveInfo.iDriveAtt & KDriveAttTransaction)
		{
		deviceCharacteristics |= SQLITE_IOCAP_ATOMIC;	
		}
	if(aVolumeInfo.iBlockSize >= SQLITE_DEFAULT_SECTOR_SIZE && (aVolumeInfo.iBlockSize & (aVolumeInfo.iBlockSize - 1)) == 0)	
		{
		switch(aVolumeInfo.iBlockSize)
			{
			case 512:
				deviceCharacteristics |= SQLITE_IOCAP_ATOMIC512;
				break;
			case 1024:
				deviceCharacteristics |= SQLITE_IOCAP_ATOMIC1K;
				break;
			case 2048:
				deviceCharacteristics |= SQLITE_IOCAP_ATOMIC2K;
				break;
			case 4096:
				deviceCharacteristics |= SQLITE_IOCAP_ATOMIC4K;
				break;
			case 8192:
				deviceCharacteristics |= SQLITE_IOCAP_ATOMIC8K;
				break;
			case 16384:
				deviceCharacteristics |= SQLITE_IOCAP_ATOMIC16K;
				break;
			case 32768:
				deviceCharacteristics |= SQLITE_IOCAP_ATOMIC32K;
				break;
			case 65536:
				deviceCharacteristics |= SQLITE_IOCAP_ATOMIC64K;
				break;
			default:
				//Do nothing. deviceCharacteristics was initialized with 0 at the beginning of the function body.
				break;
			}
		}
	return deviceCharacteristics;
	}

/**
Retrieves and returns the sector size of the drive referred by the aDriveInfo parameter.
The sector size must be a power of two.
The sector size is extracted only if aDriveInfo refers to a removable device, otherwise the
SQLITE_DEFAULT_SECTOR_SIZE value (512 bytes) will be used as a sector size.

@param aDriveInfo	A TDriveInfo reference.
@param aVolumeInfo	A TVolumeIOParamInfo reference.

@return The sector size of the drive referred by the aDriveInfo parameter.

@panic Sqlite 19 In _DEBUG mode - The sector size is negative, zero or is not a power of two.
	    
@see TVfs::Open()
*/
/* static */ TInt TVfs::DoGetSectorSize(const TDriveInfo& aDriveInfo, const TVolumeIOParamInfo& aVolumeInfo)
	{
	//Initialize the sectorSize variable only if: 
	// - aDriveInfo refers to a removable drive
	// - aVolumeInfo.iBlockSize > SQLITE_DEFAULT_SECTOR_SIZE;
	// - aVolumeInfo.iBlockSize is power of 2;
	TInt sectorSize = SQLITE_DEFAULT_SECTOR_SIZE;
	if(aDriveInfo.iDriveAtt & KDriveAttRemovable)
		{
		if(aVolumeInfo.iBlockSize > SQLITE_DEFAULT_SECTOR_SIZE && (aVolumeInfo.iBlockSize & (aVolumeInfo.iBlockSize - 1)) == 0)
			{
			sectorSize = aVolumeInfo.iBlockSize;
			}
		}
	__ASSERT_DEBUG(sectorSize > 0 && (sectorSize & (sectorSize - 1)) == 0, User::Panic(KPanicCategory, EPanicInternalError));
	return sectorSize;
	}

/**
Retrieves in a bit set the device characteristics of the device of the file referred by the aDbFile parameter.
Retrieves the sector size of the drive of the file referred by the aDbFile parameter. 
The sector size and the device characteristics will be stored in iSectorSize and iDeviceCharacteristics TDbFile data members.
The stored values will be used later by TFileIo::DeviceCharacteristics() and TFileIo::SectorSize().

@param aDbFile	Input/Output parameter. A TDriveInfo reference. The collected information will be stored in TDbDrive
				data members.
@param aRecReadBufSize Output parameter. The recommended buffer size for optimised reading performance.

@return KErrNone,          The operation has completed succesfully;
                           Note that other system-wide error codes may also be returned.

@panic Sqlite 19 In _DEBUG mode - TDbFile::iSectorSize has been already initialized.
@panic Sqlite 19 In _DEBUG mode - TDbFile::iDeviceCharacteristics has been already initialized.

@see TVfs::DoGetDeviceCharacteristics();
@see TVfs::DoGetSectorSize();
@see TVfs::Open()
@see TDbFile
@see TFileIo::DeviceCharacteristics()
@see TFileIo::SectorSize()
*/
/* static */ TInt TVfs::DoGetDeviceCharacteristicsAndSectorSize(TDbFile& aDbFile, TInt& aRecReadBufSize)
	{
	__ASSERT_DEBUG(aDbFile.iDeviceCharacteristics < 0, User::Panic(KPanicCategory, EPanicInternalError));
	__ASSERT_DEBUG(aDbFile.iSectorSize <= 0, User::Panic(KPanicCategory, EPanicInternalError));
	TInt driveNo;
	TDriveInfo driveInfo;
	TInt err = aDbFile.iFileBuf.Drive(driveNo, driveInfo);
	if(err != KErrNone)
		{
		return err;	
		}
	TVolumeIOParamInfo volumeInfo;
	err = TVfs::DoGetVolumeIoParamInfo(TStaticFs::Fs(), driveNo, volumeInfo);
	if(err != KErrNone)
		{
		return err;	
		}
	aDbFile.iDeviceCharacteristics = TVfs::DoGetDeviceCharacteristics(driveInfo, volumeInfo);
	aDbFile.iSectorSize = TVfs::DoGetSectorSize(driveInfo, volumeInfo);
	aRecReadBufSize = volumeInfo.iRecReadBufSize;
	return KErrNone;
	}

//Creates a temporary file. The file location will be the application's session path. 
//If the session path does not exist, then the function will create the session path.
static TInt CreateTempFile(TDbFile& aDbFile, TFileName& aFileNameOut, TInt aFileMode)
	{
	TFileName sessionPath;
	TInt err = TStaticFs::Fs().SessionPath(sessionPath);
	if(err == KErrNone)
		{
		err = aDbFile.iFileBuf.Temp(TStaticFs::Fs(), sessionPath, aFileNameOut, aFileMode);
		if(err == KErrPathNotFound)
			{
			err = TStaticFs::Fs().MkDirAll(sessionPath);
			if(err == KErrNone)
				{
				err = aDbFile.iFileBuf.Temp(TStaticFs::Fs(), sessionPath, aFileNameOut, aFileMode);
				}
			}
		if(err == KErrNone)
			{
			aDbFile.iFullName = aFileNameOut.Alloc();
			if(!aDbFile.iFullName)
				{
				aDbFile.iFileBuf.Close();
				(void)TStaticFs::Fs().Delete(aFileNameOut);
				err = KErrNoMemory;
				}
			}
		}
	return err;
	}

/**
SQLite OS porting layer API.

Opens or creates a file which name is in the aFileName parameter.
If the function succeeds, the file handle and other related information will be stored in the place pointed by the 
aDbFile parameter, a memory block of sizeof(TDbFile) size for which is allocated by the caller.
The function will also retrieve the sector size and the device characteristics and store them in aDbFile,
which is actually a TDbFile pointer, for later use.

@param aFileName Zero-terminated, UTF8 encoded file name.
				 If aFileName is NULL then a temporary file is created.
@param aDbFile Output parameter. The file handle and other related information will be stored there.
@param aFlags  "Open/Create" input flags: 
					SQLITE_OPEN_DELETEONCLOSE,
					SQLITE_OPEN_READWRITE,
					SQLITE_OPEN_EXCLUSIVE,
					SQLITE_OPEN_CREATE
@param aOutFlags  "Open/Create" output flags:
					SQLITE_OPEN_READWRITE,
					SQLITE_OPEN_READONLY

@return SQLITE_CANTOPEN,    The aFileName parameter cannot be converted to UTF16.
							Any other file I/O error will also be reported as SQLITE_CANTOPEN;
	    SQLITE_IOERR_NOMEM,	An out of memory condition has occured;
	    SQLITE_OK,			The operation has completed successfully.
	    
@see TDbFile
*/
/* static */ int TVfs::Open(sqlite3_vfs* aVfs, const char* aFileName, sqlite3_file* aDbFile, int aFlags, int* aOutFlags)
	{
	SQLUTRACE_PROFILER(aVfs);
	TFileName fname;
	if(aFileName && !::ConvertToUnicode(aFileName, fname))
		{
		return SQLITE_CANTOPEN;	
		}
	SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KFileOpen, aDbFile, &fname));
	new (aDbFile) TDbFile;
	TDbFile& dbFile = ::DbFile(aDbFile);
	if(aFileName && (aFlags & SQLITE_OPEN_DELETEONCLOSE))
		{
		dbFile.iFullName = fname.Alloc();
		if(!dbFile.iFullName)
			{
			return SQLITE_IOERR_NOMEM;
			}
		}
	TInt recReadBufSize = -1;
	TInt err = KErrNone;
	TInt fmode = EFileRead;
	if(aFlags & SQLITE_OPEN_READWRITE)
		{
		fmode |= EFileWrite;
		}
	//SQLite TCL tests expect the journal file to be open in shared mode.
	if(aFlags & SQLITE_OPEN_EXCLUSIVE && !(aFlags & SQLITE_OPEN_MAIN_JOURNAL))
		{
		fmode |= EFileShareExclusive;
		}
	else
		{
		fmode |= (fmode & EFileWrite) ? EFileShareAny : EFileShareReadersOnly;
		}
	if(!aFileName)	
		{//Create temporary file
		err = ::CreateTempFile(dbFile, fname, fmode);
		}
	else
		{
		err = KErrGeneral;//The error has to be set here, because, there is case where none of the file create/open operations will be executed
		if(aFlags & SQLITE_OPEN_CREATE)
			{
			err = dbFile.iFileBuf.Create(TStaticFs::Fs(), fname, fmode);
			}
		if(err != KErrNone && err != KErrNoMemory)
			{
			err = dbFile.iFileBuf.Open(TStaticFs::Fs(), fname, fmode);
			}
		if((err != KErrNone && err != KErrNoMemory) && (aFlags & SQLITE_OPEN_READWRITE))
			{
			aFlags &= ~SQLITE_OPEN_READWRITE;
			aFlags |= SQLITE_OPEN_READONLY;
			fmode &= ~EFileWrite;
			err = dbFile.iFileBuf.Open(TStaticFs::Fs(), fname, fmode);
			}
		}
	if(err == KErrNone)
		{
		err = TVfs::DoGetDeviceCharacteristicsAndSectorSize(dbFile, recReadBufSize);
		}
	if(err != KErrNone)
		{
		dbFile.iFileBuf.Close();	
		delete dbFile.iFullName;
		dbFile.iFullName = NULL;
		}
	else
		{
		dbFile.pMethods = &TheFileIoApi;
		dbFile.iReadOnly = !(aFlags & SQLITE_OPEN_READWRITE);
		if(aOutFlags)
			{
			*aOutFlags = dbFile.iReadOnly ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE;
			}
		(void)dbFile.iFileBuf.SetReadAheadSize(dbFile.iSectorSize, recReadBufSize);
		OpenCounter(+1);
		}
	return ::Os2SqliteErr(err, SQLITE_CANTOPEN);
	}

/**
SQLite OS porting layer API.

Deletes a file which name is in the aFileName parameter.

@param aFileName Zero-terminated, UTF8 encoded file name.

@return SQLITE_ERROR,    	The aFileName parameter cannot be converted to UTF16.
							The file name refers to a private secure database;
	    SQLITE_IOERR_NOMEM,	An out of memory condition has occured;
	    SQLITE_IOERR_DELETE,The delete file operation has failed;
	    SQLITE_OK,			The operation has completed successfully.
*/
/* static */ int TVfs::Delete(sqlite3_vfs* aVfs, const char* aFileName, int /*aSyncDir*/)
	{
	SQLUTRACE_PROFILER(aVfs);
	SimulateIOError(return SQLITE_IOERR_DELETE);
	TFileName fname;
	if(!::ConvertToUnicode(aFileName, fname))
		{
		return SQLITE_ERROR;	
		}
	SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KFileName, &fname));
	TInt err = TStaticFs::Fs().Delete(fname);
	return ::Os2SqliteErr(err, SQLITE_IOERR_DELETE);
	}

/**
SQLite OS porting layer API.

Retrieves an information about a file which name is in the aFileName parameter.
The requested information type can be: does the file exist, is the file read-only or read/write.

@param aFileName Zero-terminated, UTF8 encoded file name.
@param aFlags This parameter can be one of: SQLITE_ACCESS_READ, SQLITE_ACCESS_EXISTS or SQLITE_ACCESS_READWRITE.
@param aResOut Output parameter, set to 1 if the tested condition is true, 0 otherwise.

@return SQLITE_OK, 			The call has completed successfully,
		SQLITE_IOERR_NOMEM, An out of memory conditon has occured,
		SQLITE_IOERR_ACCESS,File I/O error;  
*/
/* static */ int TVfs::Access(sqlite3_vfs* aVfs, const char* aFileName, int aFlags, int* aResOut)
	{
	SQLUTRACE_PROFILER(aVfs);
	TFileName fname;
	if(!::ConvertToUnicode(aFileName, fname))
		{
		return SQLITE_IOERR_ACCESS;
		}
	SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KFileName, &fname));
	TEntry entry;
	TInt err = TStaticFs::Fs().Entry(fname, entry);
	if(aFlags == SQLITE_ACCESS_EXISTS && err == KErrNotFound)
		{
		*aResOut = 0;
		return SQLITE_OK;
		}
	if(err != KErrNone)
		{
		return err == KErrNoMemory ? SQLITE_IOERR_NOMEM : SQLITE_IOERR_ACCESS;
		}
	*aResOut = 0;
	switch(aFlags)
		{
		case SQLITE_ACCESS_READ:
			*aResOut =  entry.IsReadOnly();
			break;
		case SQLITE_ACCESS_EXISTS:
			*aResOut = 1;
			break;
		case SQLITE_ACCESS_READWRITE:
			*aResOut = !entry.IsReadOnly();
			break;
		default:
			break;			
		}
	return SQLITE_OK;
	}

/**
SQLite OS porting layer API.

Accepts UTF8 encoded, zero-terminated file as an input argument in the aRelative parameter
and constructs the full file path in the aBuf output parameter.

If the format of aRelative argument is <[SID]FileName.[EXT]>, then the database file name will be 
treated as a name of a secure database file which has to be created/opened in the server's private 
directory on the system drive.

If the format of aRelative argument is <Drive:[SID]FileName.[EXT]>, then the database file name 
will be treated as a name of a secure database file which has to be created/opened in the server's 
private directory on <Drive:> drive. 

If the format of aRelative argument is <Drive:\Path\FileName.[EXT]>, then the database file name
will be treated as a name of a non-secure database file in <Drive:\Path\> directory.
If aRelative contains file handles, then it will be treated as a name of a file belonging to server's
private data cage. 

@param aRelative The input file name, zero-terminated, UTF8 encoded.
@param aBufLen The output buffer length.
@param aBuf Output buffer for the constructed full file name path. The allocated buffer length must be at least aBufLen bytes.

@return SQLITE_ERROR, The aRelative parameter is NULL or cannot be converted to UTF16;
		SQLITE_OK The operation has completed successfully.
*/
/* static */ int TVfs::FullPathName(sqlite3_vfs* aVfs, const char* aRelative, int aBufLen, char* aBuf)
	{
	SQLUTRACE_PROFILER(aVfs);
	if(!aRelative)	//NULL argument
		{
		return SQLITE_ERROR;
		}
	//Convert the received file name to UTF16
	TBuf<KMaxFileName + 1> fname;
	if(!::ConvertToUnicode(aRelative, fname))
		{
		return SQLITE_ERROR;
		}	
	SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KFileName, &fname));
	//Search if the file name begins with ".\" - current directory
	if(fname.Find(KCwd) == 0)
		{
		fname.Delete(0, KCwd().Length());
		}
	fname.Append(TChar(0));//Zero-terminate the converted file name
	TFileName defaultPath;
	TInt err = TStaticFs::Fs().SessionPath(defaultPath);
	if(err != KErrNone)
		{
		return SQLITE_ERROR;
		}
	TParse parse;
	(void)parse.Set(fname, &defaultPath, 0);//If fname does not have a path, defaultPath will be used
	TPtr8 dest8(reinterpret_cast <TUint8*> (aBuf), aBufLen);	
	if(!::ConvertFromUnicode(parse.FullName(), dest8))
		{
		return SQLITE_ERROR;	
		}
	return SQLITE_OK;
	}

/**
SQLite OS porting layer API.

Generates a set of random numbers and stores them in the aBuf output parameter.

@param aBufLen The output buffer length.
@param aBuf Output buffer for the generated random numbers. The allocated buffer length must be at least aBufLen bytes.

@return The length of the used part of the output buffer.
*/
/* static */ int TVfs::Randomness(sqlite3_vfs* aVfs, int aBufLen, char* aBuf)
	{
	SQLUTRACE_PROFILER(aVfs);
	const TInt KRandIterations = aBufLen / sizeof(int);
	for(TInt i=0;i<KRandIterations;++i)
		{
		TInt val = Math::Rand(Seed());
		Mem::Copy(&aBuf[i * sizeof(int)], &val, sizeof(val));
		}
	return KRandIterations * sizeof(int);
	}

/**
SQLite OS porting layer API.

Sleeps for aMicrosec microseconds.

@param aMicrosec The sleep interval in microseconds.

@return The aMicrosec value.
*/
/* static */ int TVfs::Sleep(sqlite3_vfs* aVfs, int aMicrosec)
	{
	SQLUTRACE_PROFILER(aVfs);
	User::AfterHighRes(TTimeIntervalMicroSeconds32(aMicrosec));
	return aMicrosec;
	}

/**
SQLite OS porting layer API.

Retrieves the current date and time.

@param aNow Output parameter, where the data and time value will be stored.
			SQLite processes all times and dates as Julian Day numbers and
			aNow parameter will contain the julian date and time.

@return 0.
*/
/* static */ int TVfs::CurrentTime(sqlite3_vfs* aVfs, double* aNow)
	{
	SQLUTRACE_PROFILER(aVfs);
	TTime now;
	now.UniversalTime();
	TDateTime date = now.DateTime();
	TInt year = date.Year();
	TInt month = date.Month() + 1;
	TInt day = date.Day() + 1;
	
    //Calculate the Julian days
	TInt jd = day - 32076 +
	    1461*(year + 4800 + (month - 14)/12)/4 +
	    367*(month - 2 - (month - 14)/12*12)/12 -
	    3*((year + 4900 + (month - 14)/12)/100)/4;
          	
	*aNow = jd;

    // Add the fractional hours, mins and seconds
	*aNow += (date.Hour() + 12.0) / 24.0;
	*aNow += date.Minute() / 1440.0;
	*aNow += date.Second() / 86400.0;
	
#ifdef SQLITE_TEST
	if( sqlite3_current_time )
		{
    	*aNow = sqlite3_current_time / 86400.0 + 2440587.5;
		}
#endif
	return 0;
	}

/**
SQLite OS porting layer API.

Retrieves a text description of the last OS error.
Note: the method has a default "no-op" implementation at the moment. 

@return 0.
*/
/* static */int TVfs::GetLastError(sqlite3_vfs* aVfs, int /*aBufLen*/, char* /*aBuf*/)
	{
	SQLUTRACE_PROFILER(aVfs);
	return 0;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif//SQLITE_OS_SYMBIAN
