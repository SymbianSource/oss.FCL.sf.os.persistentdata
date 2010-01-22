// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "SqlPanic.h"			//ESqlPanicInvalidObj, ESqlPanicObjExists
#include "SqlDatabaseImpl.h"	//CSqlDatabaseImpl

/**
Gets the category of the return code value that is returned by
a call to the SQL API.

A call to the SQL API may complete with a non-zero return code indicating that some
unexpected behaviour has occurred. This can be categorised in a number of ways,
for example, as a Symbian OS error, or as a database error etc etc. 

This function takes the return code value and gets the category associated with
that value. The categories are defined by the enum values of
the TSqlRetCodeClass enum.

@param aSqlRetCode The return code value returned from a call to any (member)
                   function that forms part of the SQL interface.

@return The category associated with the specified return code value. This is
        one of the TSqlRetCodeClass enum values.

@see TSqlRetCodeClass

@capability None

@publishedAll
@released
*/
EXPORT_C TSqlRetCodeClass SqlRetCodeClass(TInt aSqlRetCode)
	{
	if(aSqlRetCode >= 0)
		{
		return ESqlInformation;	
		}
	else if(aSqlRetCode <= KSqlErrGeneral)
		{
		return ESqlDbError;	
		}
	return ESqlOsError;
	}

/**
Initialises the pointer to the implementation object to NULL.

@capability None
*/
EXPORT_C RSqlDatabase::RSqlDatabase() :
	iImpl(NULL)
	{
	}
	
/**
Creates a new shared non-secure or private secure database.

@param aDbFileName The full path name of the file that is to host the database.
@param aConfig the configuration string "PARAM=VALUE;....".
			   The following parameters can be set using the configuration string:
			   cache_size=value - where "value" is the cache size in pages.
			   					  "value" must be a positive integer number;
			   page_size=value - where "value" is the page size in bytes.
			   					 The "page_size" parameter can accept the following values:
			   					 512, 1024, 2048, 4096, 8192, 16384, 32768;
			   encoding=value - where "value" is the desired database encoding.
			   					 "value" could be either "UTF-8" or "UTF-16";
        	   compaction=value - where "value" is the desired compaction mode. 
                  				  "value" could be either "background", "synchronous" or "manual".

@return KErrNone, the operation has completed successfully; 
        KErrNoMemory, an out of memory condition has occurred;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrAlreadyExists, the file already exists;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the file is already open;
        KErrArgument, the file name refers to a secure database, invalid configuration string, invalid parameter values
        			  in the configuration string.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None
*/
EXPORT_C TInt RSqlDatabase::Create(const TDesC& aDbFileName, const TDesC8* aConfig)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aDbFileName));
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(aConfig ? (void)UTF::Printf(UTF::TTraceContext(UTF::EInternals), 
			KStrParam, 2, aConfig) : void(0));
	
	TRAPD(err, iImpl = CSqlDatabaseImpl::NewL(ESqlSrvDbCreate, aDbFileName, NULL, aConfig));
	return err;
	}
	
/**
Creates a new shared secure database.

@param aDbFileName       The name of the file that is to host the database.
                         The format of the name is \<drive\>:\<[SID]database file name excluding the path\>.
                         "[SID]" refers to the application SID.
@param aSecurityPolicy The container for the security policies.
@param aConfig the configuration string "PARAM=VALUE;...."
			   The following parameters can be set using the configuration string:
			   cache_size=value - where "value" is the cache size in pages.
			   					  "value" must be a positive integer number;
			   page_size=value - where "value" is the page size in bytes.
			   					 The "page_size" parameter can accept the following values:
			   					 512, 1024, 2048, 4096, 8192, 16384, 32768;
			   encoding=value - where "value" is the desired database encoding.
			   					 "value" could be either "UTF-8" or "UTF-16";
         	   compaction=value - where "value" is the desired compaction mode. 
                  				 "value" could be either "background", "synchronous" or "manual".
			   

@return KErrNone, the operation has completed successfully;
        KErrNoMemory, an out of memory condition has occurred;
        KErrArgument, the file name does not refer to a secure database;
        KErrArgument, system table name found in the security policies (aSecurityPolicy),
        			  invalid configuration string, invalid parameter values
        			  in the configuration string;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrAlreadyExists, the file already exists;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the file is already open;
        KErrPermissionDenied, the caller does not satisfy the relevant database security policies.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability The calling application must satisfy the database policy of RSqlSecurityPolicy::ESchemaPolicy type;

@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C TInt RSqlDatabase::Create(const TDesC& aDbFileName,
						const RSqlSecurityPolicy& aSecurityPolicy, const TDesC8* aConfig)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aDbFileName));
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(aConfig ? (void)UTF::Printf(UTF::TTraceContext(UTF::EInternals), 
			KStrParam, 2, aConfig) : void(0));
	
	TRAPD(err, 	iImpl = CSqlDatabaseImpl::NewL(ESqlSrvDbCreateSecure, aDbFileName,
			&aSecurityPolicy.Impl(), aConfig));
	return err;
	}
	
/**
Opens an existing database, which can be:
@code
 - shared non-secure;
 - shared secure;
 - private secure;
@endcode

@param aDbFileName The name of the file that hosts the database. If this is
                   a secure database, then the format of the name must be:
                   \<drive\>:\<[SID]database file name excluding the path\>.
                   If this is a non-secure database, then aDbFileName has to be the full database file name.
                   "[SID]" refers to SID of the application which created the database.
@param aConfig the configuration string "PARAM=VALUE;...."
			   The following parameters can be set using the configuration string:
			   cache_size=value - where "value" is the cache size in pages.
			   					  "value" must be a positive integer number;
			   

@return KErrNone, the operation has completed successfully;
        KErrNoMemory, an out of memory condition has occurred;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the file is already open;
        KErrNotFound, database file not found;
        KErrArgument, invalid configuration string, invalid parameter values
        			  in the configuration string;
        KErrGeneral, missing or invalid security policies (if the database to be opened is a secure database);
        KErrPermissionDenied, the caller does not satisfy the relevant database security policies;
        KErrNotSupported, incompatible SQL security version (if the database to be opened is a secure database).
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None, if aDbFileName refers to a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy or 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::EWritePolicy database policy type, if aDbFileName refers to a secure database;

@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C TInt RSqlDatabase::Open(const TDesC& aDbFileName, const TDesC8* aConfig)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aDbFileName));
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(aConfig ? (void)UTF::Printf(UTF::TTraceContext(UTF::EInternals), 
			KStrParam, 2, aConfig) : void(0));
	
	TRAPD(err, iImpl = CSqlDatabaseImpl::NewL(ESqlSrvDbOpen, aDbFileName, NULL, aConfig));
	return err;
	}

/**
Creates a new shared non-secure or private secure database.
 
@param aDbFileName The full path name of the file that is to host the database.
@param aConfig the configuration string "PARAM=VALUE;...."
			   The following parameters can be set using the configuration string:
			   cache_size=value - where "value" is the cache size in pages.
			   					  "value" must be a positive integer number;
			   page_size=value - where "value" is the page size in bytes.
			   					 The "page_size" parameter can accept the following values:
			   					 512, 1024, 2048, 4096, 8192, 16384, 32768;
			   encoding=value - where "value" is the desired database encoding.
			   					 "value" could be either "UTF-8" or "UTF-16";
        	   compaction=value - where "value" is the desired compaction mode. 
                  				 "value" could be either "background", "synchronous" or "manual".
			   

@leave  KErrNoMemory, an out of memory condition has occurred;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrAlreadyExists, the file already exists;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the file is already open;
        KErrArgument, the file name refers to a secure database,
        			  invalid configuration string, invalid parameter values
        			  in the configuration string.
                      Note that the function may leave with database specific errors categorised as ESqlDbError and
                      other system-wide error codes.

@capability None
*/
EXPORT_C void RSqlDatabase::CreateL(const TDesC& aDbFileName, const TDesC8* aConfig)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aDbFileName));
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(aConfig ? (void)UTF::Printf(UTF::TTraceContext(UTF::EInternals), 
			KStrParam, 2, aConfig) : void(0));
	
	iImpl = CSqlDatabaseImpl::NewL(ESqlSrvDbCreate, aDbFileName, NULL, aConfig);
	}
	
/**
Creates a new shared secure database.
@param aDbFileName       The name of the file that is to host the database.
                         The format of the name is \<drive\>:\<[SID]database file name excluding the path\>.
                         "[SID]" refers to the application SID.
@param aSecurityPolicy   The container for the security policies.
@param aConfig the configuration string "PARAM=VALUE;...."
			   The following parameters can be set using the configuration string:
			   cache_size=value - where "value" is the cache size in pages.
			   					  "value" must be a positive integer number;
			   page_size=value - where "value" is the page size in bytes.
			   					 The "page_size" parameter can accept the following values:
			   					 512, 1024, 2048, 4096, 8192, 16384, 32768;
			   encoding=value - where "value" is the desired database encoding.
			   					 "value" could be either "UTF-8" or "UTF-16";
			   
			   compaction=value - where "value" is the desired compaction mode. 
                  				  "value" could be either "background", "synchronous" or "manual".
                  				  
@leave  KErrNoMemory, an out of memory condition has occurred;
        KErrArgument, the file name does not refer to a secure database;
        KErrArgument, system table name found in the security policies (aSecurityPolicy),
        			  invalid configuration string, invalid parameter values
        			  in the configuration string;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrAlreadyExists, the file already exists;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the file is already open;
        KErrPermissionDenied, the caller does not satisfy the relevant database security policies.
                      Note that the function may leave with database specific errors categorised as ESqlDbError and
                      other system-wide error codes.

@capability The calling application must satisfy the database policy of RSqlSecurityPolicy::ESchemaPolicy type;

@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C void RSqlDatabase::CreateL(const TDesC& aDbFileName,
				const RSqlSecurityPolicy& aSecurityPolicy, const TDesC8* aConfig)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aDbFileName));
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(aConfig ? (void)UTF::Printf(UTF::TTraceContext(UTF::EInternals), 
			KStrParam, 2, aConfig) : void(0));
	
	iImpl = CSqlDatabaseImpl::NewL(ESqlSrvDbCreateSecure, aDbFileName,
						&aSecurityPolicy.Impl(), aConfig);
	}
	
/**
Opens an existing database, which can be:
@code
 - shared non-secure;
 - shared secure;
 - private secure;
@endcode

@param aDbFileName The name of the file that hosts the database. If this is
                   a secure database, then the format of the name must be:
                   \<drive\>:\<[SID]database file name excluding the path\>.
                   If this is a non-secure database, then aDbFileName has to be the full database file name.
                   "[SID]" refers to SID of the application which created the database.
@param aConfig the configuration string "PARAM=VALUE;...."
			   The following parameters can be set using the configuration string:
			   cache_size=value - where "value" is the cache size in pages.
			   					  "value" must be a positive integer number;
			   

@leave  KErrNoMemory, an out of memory condition has occurred;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the file is already open;
        KErrNotFound, database file not found;
        KErrArgument, invalid configuration string, invalid parameter values
        			  in the configuration string;
        KErrGeneral, missing or invalid security policies (if the database to be opened is a secure database);
        KErrPermissionDenied, the caller does not satisfy the relevant database security policies;
        KErrNotSupported, incompatible SQL security version (if the database to be opened is a secure database).
                      Note that the function may leave with database specific errors categorised as ESqlDbError and
                      other system-wide error codes.

@capability None, if aDbFileName refers to a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy or 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::EWritePolicy database policy type, if aDbFileName refers to a secure database;

@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C void RSqlDatabase::OpenL(const TDesC& aDbFileName, const TDesC8* aConfig)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aDbFileName));
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(aConfig ? (void)UTF::Printf(UTF::TTraceContext(UTF::EInternals), 
			KStrParam, 2, aConfig) : void(0));
	
	iImpl = CSqlDatabaseImpl::NewL(ESqlSrvDbOpen, aDbFileName, NULL, aConfig);
	}

/**
Closes this handle to the database.

The function frees memory and any allocated resources.

You can reuse this object, but you must reinitialise it by calling
RSqlDatabase::Create() or RSqlDatabase::Open().

@see RSqlDatabase::Create()
@see RSqlDatabase::Open()

@capability None
*/
EXPORT_C void RSqlDatabase::Close()
	{
	SQLUTRACE_PROFILER(this);
	delete iImpl;
	iImpl = NULL;
	}
	
/**
Attaches an existing database to the current database connection.

The attached database can be read, written or modified. 
One database can be attched multiple times to the same connection, using different logical database names.
Tables in an attached database can be referred to using "database-name.table-name" syntax.
If an attached table doesn't have a duplicate table name in the main database, it doesn't 
require a database name prefix. 

Transactions involving multiple attached databases are atomic.

@param aDbFileName The name of the file that hosts the database. If this is
                   a secure database, then the format of the name must be:
                   \<drive\>:\<[SID]database file name excluding the path\>.
                   If this is a private or shared non-secure database, then aDbFileName has to be the full 
                   database file name. "[SID]" refers to SID of the application which created the attached database.
@param aDbName Logical database name. It must be unique (per database connection). This is the name which can
               be used for accessing tables in the attached database. The syntax is "database-name.table-name".

@return KErrNone, the operation has completed successfully;
        KErrNoMemory, an out of memory condition has occurred;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the file is already open;
        KErrNotFound, database file not found;
        KErrGeneral, missing or invalid security policies (if the database to be opened is a secure database);
        KErrPermissionDenied, the caller does not satisfy the relevant database security policies;
        KErrNotSupported, incompatible SQL security version (if the database to be opened is a secure database).
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None, if aDbFileName refers to a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy or 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::EWritePolicy database policy type, if aDbFileName refers to a secure database;

@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C TInt RSqlDatabase::Attach(const TDesC& aDbFileName, const TDesC& aDbName)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrStrParam16, &aDbFileName, &aDbName));
	
	return Impl().Attach(aDbFileName, aDbName);
	}
		
/**
Detaches previously attached database.

@param aDbName Logical database name. 
			   The logical name of the database to be detached.

@return KErrNone, the operation completed has successfully;
        KErrNotFound, no attached database with aDbName name;
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None
*/
EXPORT_C TInt RSqlDatabase::Detach(const TDesC& aDbName)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aDbName));
	
	return Impl().Detach(aDbName);
	}

/**
Copies a database file to the specified location.

Note that this is a static function, and its use is not
restricted to any specific RSqlDatabase instance.

@param aSrcDbFileName Source database file name.
					  If this is a secure database, then the format of the name must be:
					  \<drive\>:\<[SID]database file name excluding the path\>.
					  If this is a non-secure database, then aDbFileName has to be the full database file name.
					  "[SID]" refers to SID of the application which created the database.
@param aDestDbFileName Destination database file name.
					  If this is a secure database, then the format of the name must be:
					  \<drive\>:\<[SID]database file name excluding the path\>.
					  If this is a non-secure database, then aDbFileName has to be the full database file name.
					  "[SID]" refers to SID of the application which performs the copying operation.

The allowed copying operations are:
- secure to secure database. Only the application created the database is allowed to copy it.
- non-secure to non-secure database. No restrictions apply to this operation.

@return KErrNone, the operation completed has successfully;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the file is already open;
        KErrNotFound, database file not found;
        KErrPermissionDenied, the SID of the calling application does not match the SID of source or destination database.
                      Note that other system-wide error codes may also be returned.

@capability None

Note that if the source database is a secure database, only the application, which created the database, can copy it.
*/
EXPORT_C TInt RSqlDatabase::Copy(const TDesC& aSrcDbFileName, const TDesC& aDestDbFileName)
	{
	SQLUTRACE_PROFILER(0);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrStrParam16, 
			&aSrcDbFileName, &aDestDbFileName));

	return CSqlDatabaseImpl::Copy(aSrcDbFileName, aDestDbFileName);
	}
	
/**
Deletes the specified database file.

Note that this is a static function, and its use is not
restricted to any specific RSqlDatabase instance.

@param aDbFileName The name of the database file.
				   If this is a secure database, then the format of the name must be:
				   \<drive\>:\<[SID]database file name excluding the path\>.
				   If this is a private or shared non-secure database, then aDbFileName has to be the 
				   full database file name. "[SID]" refers to SID of the application which created the database.

@return KErrNone, the operation has completed successfully;
        KErrBadName, the file name is invalid - it has either a zero length or it is the name of a directory;
        KErrNotReady, the drive does not exist or is not ready;
        KErrInUse, the database file is in use;
        KErrNotFound, the database file cannot be found;
        KErrAccessDenied, access to the database file is denied (e.g. it might be a read-only file);
        KErrPermissionDenied, the SID of the calling application does not match the SID of the database;
                      Note that other system-wide error codes may also be returned.

@capability None

Note that if the database to be deleted is a secure database, only the application, which created the database, can delete it.
*/
EXPORT_C TInt RSqlDatabase::Delete(const TDesC& aDbFileName)
	{
	SQLUTRACE_PROFILER(0);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aDbFileName));
	
	return CSqlDatabaseImpl::Delete(aDbFileName);
	}
	
/**
Initializes aSecurityPolicy output parameter with a copy of the database security policies.
The caller is responsible for destroying the initialized aSecurityPolicy paramemter.

Note that there may be no security policies in force for this database.

@param aSecurityPolicy Input/Output parameter, which will be initialized with the database
						 security policies.

@return KErrNone, the operation has completed successfully;
		KErrNotSupported, the current database is not a secure database;
        KErrNoMemory, an out of memory condition has occurred;

@capability None
*/
EXPORT_C TInt RSqlDatabase::GetSecurityPolicy(RSqlSecurityPolicy& aSecurityPolicy) const
	{
	SQLUTRACE_PROFILER(this);
	TRAPD(err, CSqlSecurityPolicy* securityPolicy = Impl().CloneSecurityPolicyL();
		aSecurityPolicy.Set(*securityPolicy));
	
	return err;
	}
	
/**
Initializes aSecurityPolicy output parameter with a copy of the database security policies.
The caller is responsible for destroying the initialized aSecurityPolicy paramemter.

Note that there may be no security policies in force for this database.

@param aSecurityPolicy Input/Output parameter, which will be initialized with the database
						 security policies.

@leave  KErrNotSupported, the current database is not a secure database;
        KErrNoMemory, an out of memory condition has occurred;

@capability None
*/
EXPORT_C void RSqlDatabase::GetSecurityPolicyL(RSqlSecurityPolicy& aSecurityPolicy) const
	{
	SQLUTRACE_PROFILER(this);
	CSqlSecurityPolicy* securityPolicy = Impl().CloneSecurityPolicyL();
	aSecurityPolicy.Set(*securityPolicy);	
	}
	
/**
Sets the transaction isolation level for the database.

A transaction isolation level defines the way in which a transaction
interacts with other transactions that may be in progress concurrently.

Transaction isolation levels are defined by the values of the
RSqlDatabase::TIsolationLevel enum.

The default isolation level is RSqlDatabase::ESerializable

Note that the isolation level is not a persistent property of the database.
It is set to the default value, i.e. RSqlDatabase::ESerializable,
whenever the database is created or opened.

@param aIsolationLevel The isolation level to be set.

@return KErrNone, if the operation has completed successfully;
		KErrNotSupported, invalid (not supported) transaction isolation level;

@see RSqlDatabase::TIsolationLevel

@capability None
*/
EXPORT_C TInt RSqlDatabase::SetIsolationLevel(RSqlDatabase::TIsolationLevel aIsolationLevel)
	{
	SQLUTRACE_PROFILER(this);
	return Impl().SetIsolationLevel(aIsolationLevel);
	}
	
/**
Executes one or more 16-bit SQL statements.

This method should be used for executing DDL/DML statements, but note the following point:
- if an SQL statement contains one or more parameters, then the function will execute it,
giving the parameters default NULL values.

This class (RSqlDatabase) does not offer functions for setting the parameter values. 
Use the RSqlStatement class instead.

If the call to this function fails because of a database-specific type error 
(i.e. the error is categorised as of type ESqlDbError), then a textual description of
the error can be obtained calling RSqlDatabase::LastErrorMessage().

@param aSqlStmt A string of 16-bit wide characters containing one or more DDL/DML SQL statements;
                each statement is separated by a ';' character.

@return >=0, The operation has completed successfully. The number of database rows that were 
			 changed/inserted/deleted by the most recently completed DDL/DML sql statement.
			 Exception: If the executed statement is "DELETE FROM <table>", then the function returns 0 
			 if the operation has completed successfully (disregarding the number of the deleted rows);
        KErrNoMemory, an out of memory condition has occured;
        KSqlErrGeneral, a syntax error has occurred - text describing the problem
                        can be obtained by calling 	RSqlDatabase::LastErrorMessage();
        KErrDiskFull, There is no available disk space to complete the operation. If the executed statement is a DELETE
        			  statement, try to use the reserved disk space (if there is a reserved disk space) to complete the operation.
        			  In all other cases the database connection should be closed and some disk space freed before reopening 
        			  the database; 
        KErrPermissionDenied, the caller does not satisfy the relevant database security policies.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None, if current RSqlDatabase object represents a handle to a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement modifies a secure database schema; 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement reads from a secure database; 
            RSqlSecurityPolicy::EWritePolicy or
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement writes to a secure database;

@see RSqlStatement
@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase::LastErrorMessage()
@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C TInt RSqlDatabase::Exec(const TDesC& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aSqlStmt));

	return Impl().Exec(aSqlStmt);
	}
	
/**
Executes one or more 8-bit SQL statements.

This method should be used for executing DDL/DML statements, but note the following point:
- if an SQL statement contains one or more parameters, then the function will execute it,
giving the parameters default NULL values.

This class (RSqlDatabase) does not offer functions for setting the parameter values. 
Use the RSqlStatement class instead.

If the call to this function fails because of a database-specific type error 
(i.e. the error is categorised as ESqlDbError), then a textual description of
the error can be obtained calling RSqlDatabase::LastErrorMessage().

@param aSqlStmt A string of 8-bit wide characters containing one or more DDL/DML SQL statements;
                each statement is separated by a ';' character.

@return >=0, The operation has completed successfully. The number of database rows that were 
			 changed/inserted/deleted by the most recently completed DDL/DML sql statement.
			 Exception: If the executed statement is "DELETE FROM <table>", then the function returns 0 
			 if the operation has completed successfully (disregarding the number of the deleted rows);
        KErrNoMemory,  an out of memory condition has occured;
        KSqlErrGeneral, a syntax error has occurred - text describing the problem
                        can be obtained by calling 	RSqlDatabase::LastErrorMessage();
        KErrDiskFull, There is no available disk space to complete the operation. If the executed statement is a DELETE
        			  statement, try to use the reserved disk space (if there is a reserved disk space) to complete the operation.
        			  In all other cases the database connection should be closed and some disk space freed before reopening 
        			  the database; 
        KErrPermissionDenied, the caller does not satisfy the relevant database security policies;
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None, if current RSqlDatabase object represents a handle to a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement modifies a secure database schema; 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement reads from a secure database; 
            RSqlSecurityPolicy::EWritePolicy or
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement writes to a secure database;

@see RSqlStatement
@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase::LastErrorMessage()
@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C TInt RSqlDatabase::Exec(const TDesC8& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam, 1, &aSqlStmt));

	return Impl().Exec(aSqlStmt);
	}

/**
Executes one or more 16-bit SQL statements asynchronously to allow client to avoid being blocked 
by server activity.

No other operations can be performed on current RSqlDatabase object and RSqlStatement objects using it
until the asynchronous operation completes.

This method should be used for executing DDL/DML statements, but note the following point:
- if an SQL statement contains one or more parameters, then the function will execute it,
giving the parameters default NULL values.

This class (RSqlDatabase) does not offer functions for setting the parameter values. 
Use the RSqlStatement class instead.

If the call to this function fails because of a database-specific type error 
(i.e. the error is categorised as ESqlDbError), then a textual description of
the error can be obtained calling RSqlDatabase::LastErrorMessage().

@param aSqlStmt A string of 16-bit wide characters containing one or more DDL/DML SQL statements;
                each statement is separated by a ';' character.
@param aStatus Completion status of asynchronous request, one of the following:
			>=0, The operation has completed successfully. The number of database rows that were 
			   	   changed/inserted/deleted by the most recently completed DDL/DML sql statement.
			 Exception: If the executed statement is "DELETE FROM <table>", then the function returns 0 
			 if the operation has completed successfully (disregarding the number of the deleted rows);
        	KErrNoMemory,  an out of memory condition has occured;
        	KSqlErrGeneral, a syntax error has occurred - text describing the problem
                        can be obtained by calling 	RSqlDatabase::LastErrorMessage();
        	KErrDiskFull, There is no available disk space to complete the operation. If the executed statement is a DELETE
        			  statement, try to use the reserved disk space (if there is a reserved disk space) to complete the operation.
        			  In all other cases the database connection should be closed and some disk space freed before reopening 
        			  the database; 
        	KErrPermissionDenied, the caller does not satisfy the relevant database security policies;
                      Note that aStatus may be set with database specific errors categorised as ESqlDbError, 
                      and other system-wide error codes.


@capability None, if current RSqlDatabase object represents a handle to a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement modifies a secure database schema; 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement reads from a secure database; 
            RSqlSecurityPolicy::EWritePolicy or
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement writes to a secure database;

@see RSqlStatement
@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase::LastErrorMessage()
@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C void RSqlDatabase::Exec(const TDesC& aSqlStmt, TRequestStatus& aStatus)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aSqlStmt));

	Impl().Exec(aSqlStmt, aStatus);
	}

/**
Executes one or more 8-bit SQL statements asynchronously to allow client to avoid being blocked 
by server activity.

No other operations can be performed on current RSqlDatabase object and RSqlStatement objects using it
until the asynchronous operation completes.

This method should be used for executing DDL/DML statements, but note the following point:
- if an SQL statement contains one or more parameters, then the function will execute it,
giving the parameters default NULL values.

This class (RSqlDatabase) does not offer functions for setting the parameter values. 
Use the RSqlStatement class instead.

If the call to this function fails because of a database-specific type error 
(i.e. the error is categorised as ESqlDbError), then a textual description of
the error can be obtained calling RSqlDatabase::LastErrorMessage().

@param aSqlStmt A string of 8-bit wide characters containing one or more DDL/DML SQL statements;
                each statement is separated by a ';' character.
@param aStatus Completion status of asynchronous request, one of the following:
			>=0, The operation has completed successfully. The number of database rows that were 
			   	   changed/inserted/deleted by the most recently completed DDL/DML sql statement.
			 Exception: If the executed statement is "DELETE FROM <table>", then the function returns 0 
			 if the operation has completed successfully (disregarding the number of the deleted rows);
        	KErrNoMemory,  an out of memory condition has occured;
        	KSqlErrGeneral, a syntax error has occurred - text describing the problem
                        can be obtained by calling 	RSqlDatabase::LastErrorMessage();
        	KErrDiskFull, There is no available disk space to complete the operation. If the executed statement is a DELETE
        			  statement, try to use the reserved disk space (if there is a reserved disk space) to complete the operation.
        			  In all other cases the database connection should be closed and some disk space freed before reopening 
        			  the database; 
        	KErrPermissionDenied, the caller does not satisfy the relevant database security policies;
                      Note that aStatus may be set with database specific errors categorised as ESqlDbError, 
                      and other system-wide error codes.


@capability None, if current RSqlDatabase object represents a handle to a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement modifies a secure database schema; 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement reads from a secure database; 
            RSqlSecurityPolicy::EWritePolicy or
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement writes to a secure database;

@see RSqlStatement
@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase::LastErrorMessage()
@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
*/
EXPORT_C void RSqlDatabase::Exec(const TDesC8& aSqlStmt, TRequestStatus& aStatus)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam, 1, &aSqlStmt));

	Impl().Exec(aSqlStmt, aStatus);
	}
	
/**
Retrieves a reference to the textual description of the error returned by the
most recent call to any of the functions:
- RSqlDatabase::Exec()
- RSqlStatement::Exec()
- RSqlStatement::Next()
- RSqlStatement::Reset()

Note that the function can only return a reference to text for
database-specific type errors, i.e. those errors that are categorised as of
type ESqlDbError.

@return A non-modifiable pointer descriptor representing the most recent error
        message. Note that message may be NULL, i.e. the descriptor may have
        zero length.
 
@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase::Exec()
@see RSqlStatement::Exec()
@see RSqlStatement::Next()
@see RSqlStatement::Reset()

@capability None
*/
EXPORT_C TPtrC RSqlDatabase::LastErrorMessage() const
	{
	SQLUTRACE_PROFILER(0);
	return Impl().LastErrorMessage();
	}

/**
Returns the ROWID of the most recent successful INSERT into the database 
from this database connection.

@return >0, the ROWID of the most recent successful INSERT into the database
			from this database connection;
		0, 	if no successful INSERTs have ever occurred from this database connection
		<0, if one of the system-wide error codes is returned

@capability None
*/
EXPORT_C TInt64 RSqlDatabase::LastInsertedRowId() const
	{
	SQLUTRACE_PROFILER(0);
	return Impl().LastInsertedRowId();	
	}

/**
Checks the database transaction state.

@return True, if the database is in transaction, false otherwise.

@capability None
*/
EXPORT_C TBool RSqlDatabase::InTransaction() const
	{
	SQLUTRACE_PROFILER(0);
	return Impl().InTransaction();
	}
	
/**
Returns the database file size, in bytes.

@return >= 0, 		  the operation has completed successfully. The number is the size of the main
					  database file;
        KErrNoMemory, an out of memory condition has occurred;
        KErrTooBig,   the database is too big and the size cannot fit into 32-bit signed integer;
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None
*/
EXPORT_C TInt RSqlDatabase::Size() const
	{
	SQLUTRACE_PROFILER(0);
	return Impl().Size();
	}

/**
Returns the database file size and free space, in bytes.

@param aSize An output parameter. If the call was successfull the aSize object will contain information
			 about the database size and database free space.
@param aDbName The attached database name or KNullDesC for the main database

@return KErrNone, The operation has completed succesfully;
        KErrBadName, Invalid (too long) attached database name;
        KSqlErrGeneral, There is no an attached database with aDbName name;
                  Note that database specific errors categorised as ESqlDbError, and
                  other system-wide error codes may also be returned.
                  
@capability None
*/
EXPORT_C TInt RSqlDatabase::Size(RSqlDatabase::TSize& aSize, const TDesC& aDbName) const
	{
	SQLUTRACE_PROFILER(0);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 2, &aDbName));
	
	return Impl().Size(aSize, aDbName);
	}

/**
Compacts the database.
This function should be used for databases that have been configured for a manual compaction during 
the database creation.
The function has no effect if the database has been configured for an auto compaction.
The function has no effect if the aSize argument value is zero.
The function has no effect also if there aren't any free pages in the database file.
If the database has been configured for a background compaction, then the function works as if the database
has been configured for a manual compaction.

@param aSize Can be one of:
				RSqlDatabase::EMaxCompaction - requests a full database compaction. All free pages
				  (if any exists) will be removed;
				Positive 32-bit signed integer value - the server will attempt to compact the database removing 
				  at most aSize bytes from the database file, rounded up to the nearest page count, 
				  e.g. request for removing 1 byte will remove one free page from the database;

@param aDbName The attached database name or KNullDesC for the main database

@return Zero or positive integer - the operation has completed succesfully, the return value is the
         						   size of the removed free space in bytes,
		KErrArgument, Invalid aSize value;
        KErrBadName, Invalid (too long) attached database name;
        KSqlErrReadOnly, Read-only database;
        KSqlErrGeneral, There is no an attached database with aDbName name;
                  Note that database specific errors categorised as ESqlDbError, and
                  other system-wide error codes may also be returned.
                  
@capability None
*/
EXPORT_C TInt RSqlDatabase::Compact(TInt64 aSize, const TDesC& aDbName)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KSizeStrParam16, 1, aSize, 2, &aDbName));

	return Impl().Compact(aSize, aDbName);
	}
	
/**
Compacts the database asynchronously.
This function should be used for databases that have been configured for a manual compaction during 
the database creation.
The function has no effect if the database has been configured for an auto compaction.
The function has no effect if the aSize argument value is zero.
The function has no effect also if there aren't any free pages in the database file.
If the database has been configured for a background compaction, then the function works as if the database
has been configured for a manual compaction.

@param aStatus Completion status of asynchronous request, one of the following:
		Zero or positive integer - the operation has completed succesfully, the return value is the
         						   size of the removed free space in bytes,
		KErrArgument, Invalid aSize value;
        KErrBadName, Invalid (too long) attached database name;
        KSqlErrReadOnly, Read-only database;
        KSqlErrGeneral, There is no an attached database with aDbName name;
                  Note that database specific errors categorised as ESqlDbError, and
                  other system-wide error codes may also be returned.


@param aSize Can be one of:
				 RSqlDatabase::EMaxCompaction - requests a full database compaction. All free pages
				  (if any exists) will be removed;
				 Positive 32-bit signed integer value - the server will attempt to compact the database removing 
				  at most aSize bytes from the database file, rounded up to the nearest page count, 
				  e.g. request for removing 1 byte will remove one free page from the database;

@param aDbName The attached database name or KNullDesC for the main database
                  
@capability None
*/
EXPORT_C void RSqlDatabase::Compact(TInt64 aSize, TRequestStatus& aStatus, const TDesC& aDbName)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KSizeStrParam16, 1, aSize, 3, &aDbName));

	Impl().Compact(aSize, aDbName, aStatus);
	}

/**
Reserves a predefined amount of disk space on the drive where the database file is.

At the moment the max possible amount, allowed by the file server, is reserved on the database file drive.
Use this call to ensure that if your "delete records" transaction fails because of "out of
disk space" circumstances, you can get an access to the already reserved disk space and 
complete your transaction successfully the second time.

There is no strong, 100% guarantee, that the reserved disk space will always help the client
in "low memory" situations.

@param aSpace This parameter is not used at the moment.

@return KErrNone,          The operation has completed succesfully;
	    KErrNoMemory,      Out of memory condition has occured;
        KErrAlreadyExists, The space has already been reserved;
                           Note that other system-wide error codes may also be returned.

@see RSqlDatabase::FreeReservedSpace()
@see RSqlDatabase::GetReserveAccess()
@see RSqlDatabase::ReleaseReserveAccess()
*/
EXPORT_C TInt RSqlDatabase::ReserveDriveSpace(TInt aSize)
	{
	SQLUTRACE_PROFILER(this);
	//Usage of the IPC call arguments: 
	//Arg 0: [out]  requested size
	return Impl().Session().SendReceive(ESqlSrvDbReserveDriveSpace, TIpcArgs(aSize));
	}
	
/**
Frees the reserved disk space.

@see RSqlDatabase::ReserveDriveSpace()
@see RSqlDatabase::GetReserveAccess()
@see RSqlDatabase::ReleaseReserveAccess()
*/
EXPORT_C void RSqlDatabase::FreeReservedSpace()
	{
	SQLUTRACE_PROFILER(this);
	Impl().Session().SendReceive(ESqlSrvDbFreeReservedSpace);
	}
	
/**
Gives the client an access to the already reserved disk space.

@return KErrNone,     The operation has completed succesfully;
        KErrNotFound, An attempt is made to get an access to a disk space, which is not reserved yet;
        KErrInUse,    An access to the reserved space has already been given;
                      Note that other system-wide error codes may also be returned.

@see RSqlDatabase::ReserveDriveSpace()
@see RSqlDatabase::FreeReservedSpace()
@see RSqlDatabase::ReleaseReserveAccess()
*/
EXPORT_C TInt RSqlDatabase::GetReserveAccess()
	{
	SQLUTRACE_PROFILER(this);
	return Impl().Session().SendReceive(ESqlSrvDbGetReserveAccess);
	}
	
/**
Releases the access to the reserved disk space.

@see RSqlDatabase::ReserveDriveSpace()
@see RSqlDatabase::FreeReservedSpace()
@see RSqlDatabase::GetReserveAccess()
*/
EXPORT_C void RSqlDatabase::ReleaseReserveAccess()
	{
	SQLUTRACE_PROFILER(this);
	Impl().Session().SendReceive(ESqlSrvDbReleaseReserveAccess);
	}

/**
Returns a reference to the implementation object of RSqlDatabase - CSqlDatabaseImpl.

@panic SqlDb 2 Create() or Open() has not previously been called on this RSqlDatabase object.

@internalComponent
*/
CSqlDatabaseImpl& RSqlDatabase::Impl() const
	{
	__SQLASSERT_ALWAYS(iImpl != NULL, ESqlPanicInvalidObj);
	return *iImpl;	
	}
