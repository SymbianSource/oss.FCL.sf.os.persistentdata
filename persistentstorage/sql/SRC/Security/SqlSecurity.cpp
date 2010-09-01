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

#include "SqlSecurityImpl.h"
#include "SqlPanic.h"
#include "UTraceSql.h"

const TInt32 KEndOfSPStream = -1;//If found in the stream, given as an argument to RSqlSecurityPolicy::InternalizeL(),
							     //then there are no more policies in the stream.

/**
Initializes RSqlSecurityPolicy instance data members with their default values.

@capability None
*/
EXPORT_C RSqlSecurityPolicy::RSqlSecurityPolicy() :
	iImpl(NULL)
	{
	}

/**
Initializes RSqlSecurityPolicy instance.

@param aDefaultPolicy Default security policy which will be used for the database and all database objects.

@return KErrNone, the operation has completed successfully;
		KErrNoMemory, an out of memory condition has occured.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@see TSecurityPolicy

@capability None
*/
EXPORT_C TInt RSqlSecurityPolicy::Create(const TSecurityPolicy& aDefaultPolicy)
	{
	SQLUTRACE_PROFILER(this);
	TRAPD(err, CreateL(aDefaultPolicy));
	return err;
	}

/**
Initializes RSqlSecurityPolicy instance.

@param aDefaultPolicy Default security policy which will be used for the database and all database objects.

@leave  KErrNoMemory, an out of memory condition has occured.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@see TSecurityPolicy

@capability None
*/
EXPORT_C void RSqlSecurityPolicy::CreateL(const TSecurityPolicy& aDefaultPolicy)
	{
	SQLUTRACE_PROFILER(this);
	iImpl = CSqlSecurityPolicy::NewL(aDefaultPolicy);
	}

/**
Frees the allocated by RSqlSecurityPolicy instance memory and other resources.

@capability None
*/
EXPORT_C void RSqlSecurityPolicy::Close()
	{
	SQLUTRACE_PROFILER(this);
	delete iImpl;
	iImpl = NULL;
	}


/**
Sets a database security policy of a specific type.

Sets database security policy (aPolicy argument) of aPolicyType type.
If the aPolicyType database security policy has already been set then it will be replaced with the supplied policy.

@param aPolicyType Database security policy type: RSqlSecurityPolicy::ESchema, RSqlSecurityPolicy::ERead, RSqlSecurityPolicy::EWrite.
@param aPolicy The database security policy.

@panic SqlDb 4 Invalid aPolicyType value.

@return KErrNone

@see RSqlSecurityPolicy::TPolicyType
@see TSecurityPolicy

@capability None
*/
EXPORT_C TInt RSqlSecurityPolicy::SetDbPolicy(TPolicyType aPolicyType, const TSecurityPolicy& aPolicy)
	{
	SQLUTRACE_PROFILER(this);
	__SQLASSERT_ALWAYS(aPolicyType >= ESchemaPolicy && aPolicyType <= EWritePolicy, ESqlPanicBadArgument);
	Impl().SetDbPolicy(aPolicyType, aPolicy);
	return KErrNone;
	}
	
/**
Sets a database object security policy of a specific type.

If there is no entry in the security policy container for the object with aObjectName name, then a new entry for this 
object will be created and all object security policies will be initialized with the default security policy. 
The specific database object policy, refered by aPolicyType parameter, will be set after that.

If an entry for aObjectName object already exists, its security policy of "aPolicyType" type will be 
reinitialized with the data of aPolicy parameter.

@param aObjectType Database object type. At the moment there is only one database object type - RSqlSecurityPolicy::ETable.
@param aObjectName Database object name. It cannot be a null descriptor.
@param aPolicyType Database object security policy type: RSqlSecurityPolicy::EReadPolicy, RSqlSecurityPolicy::EWritePolicy.
@param aPolicy Database security policy.

@return KErrNone, the operation has completed successfully;
		KErrNoMemory, an out of memory condition has occured.

@panic SqlDb 4 Invalid aPolicyType value.
@panic SqlDb 4 Invalid aObjectType value (It has to be RSqlSecurityPolicy::ETable).
@panic SqlDb 4 Invalid aObjectName value (Null descriptor).

@see RSqlSecurityPolicy::TObjectType
@see RSqlSecurityPolicy::TPolicyType
@see TSecurityPolicy

@capability None
*/
EXPORT_C TInt RSqlSecurityPolicy::SetPolicy(TObjectType aObjectType, const TDesC& aObjectName, 
									  TPolicyType aPolicyType, const TSecurityPolicy& aPolicy)
	{
	SQLUTRACE_PROFILER(this);
	__SQLASSERT_ALWAYS(aObjectType == ETable, ESqlPanicBadArgument);
	__SQLASSERT_ALWAYS(aObjectName.Length() > 0, ESqlPanicBadArgument);
	__SQLASSERT_ALWAYS(aPolicyType >= EReadPolicy && aPolicyType <= EWritePolicy, ESqlPanicBadArgument);
	return Impl().SetPolicy(aObjectType, aObjectName, aPolicyType, aPolicy);
	}

/**
Gets the default database security policy.

@return The default security policy.
				   
@see TSecurityPolicy

@capability None
*/	
EXPORT_C TSecurityPolicy RSqlSecurityPolicy::DefaultPolicy() const
	{
	SQLUTRACE_PROFILER(this);
	return Impl().DefaultPolicy();
	}

/**
Gets a database security policy of the specified type.

@param aPolicyType Database security policy type: RSqlSecurityPolicy::ESchemaPolicy, RSqlSecurityPolicy::EReadPolicy, 
				   RSqlSecurityPolicy::EWritePolicy.

@return The requested database security policy.
				   
@panic SqlDb 4 Invalid aPolicyType value.

@see RSqlSecurityPolicy::TPolicyType
@see TSecurityPolicy

@capability None
*/	
EXPORT_C TSecurityPolicy RSqlSecurityPolicy::DbPolicy(TPolicyType aPolicyType) const
	{
	SQLUTRACE_PROFILER(this);
	__SQLASSERT_ALWAYS(aPolicyType >= ESchemaPolicy && aPolicyType <= EWritePolicy, ESqlPanicBadArgument);
	return Impl().DbPolicy(aPolicyType);
	}
	
/**
Gets a database object security policy of the specified type.

If no security policy of the specified type exists for that database object - the default security policy
will be returned.

@param aObjectType Database object type. At the moment there is only one database object type - RSqlSecurityPolicy::ETable.
@param aObjectName Database object name. It cannot be a null descriptor.
@param aPolicyType Database object security policy type: RSqlSecurityPolicy::EReadPolicy, RSqlSecurityPolicy::EWritePolicy.

@return The requested security policy.

@panic SqlDb 4 Invalid aPolicyType value.
@panic SqlDb 4 Invalid aObjectType value (It has to be RSqlSecurityPolicy::ETable).
@panic SqlDb 4 Invalid aObjectName value (Null descriptor).

@see RSqlSecurityPolicy::TObjectType
@see RSqlSecurityPolicy::TPolicyType
@see TSecurityPolicy

@capability None
*/
EXPORT_C TSecurityPolicy RSqlSecurityPolicy::Policy(TObjectType aObjectType, const TDesC& aObjectName, 
												 TPolicyType aPolicyType) const
	{
	SQLUTRACE_PROFILER(this);
	__SQLASSERT_ALWAYS(aObjectType == ETable, ESqlPanicBadArgument);
	__SQLASSERT_ALWAYS(aObjectName.Length() > 0, ESqlPanicBadArgument);
	__SQLASSERT_ALWAYS(aPolicyType >= EReadPolicy && aPolicyType <= EWritePolicy, ESqlPanicBadArgument);
	return Impl().Policy(aObjectType, aObjectName, aPolicyType);
	}

/**
Externalizes RSqlSecurityPolicy instance to a write stream.

@param aStream Stream to which RSqlSecurityPolicy instance should be externalised.

@leave KErrNoMemory, an out of memory condition has occured.

@capability None
*/
EXPORT_C void RSqlSecurityPolicy::ExternalizeL(RWriteStream& aStream) const
	{
	SQLUTRACE_PROFILER(this);
	RSqlSecurityPolicy::TObjectType objectType;
	TPtrC objectName;
	RSqlSecurityPolicy::TPolicyType policyType;
	TSecurityPolicy policy;
	//Default policy
	policy = Impl().DefaultPolicy();
	aStream << policy.Package();
	//Database policies
	policy = Impl().DbPolicy(RSqlSecurityPolicy::ESchemaPolicy);
	aStream << policy.Package();
	policy = Impl().DbPolicy(RSqlSecurityPolicy::EReadPolicy);
	aStream << policy.Package();
	policy = Impl().DbPolicy(RSqlSecurityPolicy::EWritePolicy);
	aStream << policy.Package();
	//Database object policies
	TSqlSecurityPolicyIterator it(Impl());
	while(it.Next(objectType, objectName, policyType, policy))
		{
		aStream << static_cast <TInt32> (objectType);
		aStream << objectName;
		aStream << static_cast <TInt32> (policyType);
		aStream << policy.Package();
		}
	//Object policy stream - end
	aStream << KEndOfSPStream;
	}
	
/**
Initializes RSqlSecurityPolicy instance from a stream.
In case of an error the original security policy data is preserved.

@param aStream A read stream containing the data with which the RSqlSecurityPolicy instance will be initialized.

@leave KErrNoMemory, an out of memory condition has occured.
                     Note that the function may leave with other system-wide error codes.

@capability None
*/
EXPORT_C void RSqlSecurityPolicy::InternalizeL(RReadStream& aStream)
	{
	SQLUTRACE_PROFILER(this);
	TSecurityPolicy policy;
	TBuf8<sizeof(TSecurityPolicy)> policyBuf;
	//Default policy
	aStream >> policyBuf;
	policy.Set(policyBuf);
	//Create new sql security policy object	and initialize it with the policies read from the input stream
	RSqlSecurityPolicy newPolicy;
	newPolicy.CreateL(policy);
	CleanupClosePushL(newPolicy);
	//Database policies
	aStream >> policyBuf;
	policy.Set(policyBuf);
	__SQLLEAVE_IF_ERROR(newPolicy.SetDbPolicy(RSqlSecurityPolicy::ESchemaPolicy, policy));
	aStream >> policyBuf;
	policy.Set(policyBuf);
	__SQLLEAVE_IF_ERROR(newPolicy.SetDbPolicy(RSqlSecurityPolicy::EReadPolicy, policy));
	aStream >> policyBuf;
	policy.Set(policyBuf);
	__SQLLEAVE_IF_ERROR(newPolicy.SetDbPolicy(RSqlSecurityPolicy::EWritePolicy, policy));
	//Database object policies
	for(;;)
		{
		TInt32 objectType;
		aStream >> objectType;
		if(objectType == KEndOfSPStream)
			{
			break;	
			}
		TBuf<KMaxFileName> objectName;
		aStream >> objectName;
		TInt32 policyType;
		aStream >> policyType;
		aStream >> policyBuf;
		policy.Set(policyBuf);
		__SQLLEAVE_IF_ERROR(newPolicy.SetPolicy(static_cast <RSqlSecurityPolicy::TObjectType> (objectType), objectName, static_cast <RSqlSecurityPolicy::TPolicyType> (policyType), policy));
		}
	//Swap the original sql security policy with the new sql security policy
	CSqlSecurityPolicy* temp = newPolicy.iImpl;
	newPolicy.iImpl = iImpl;
	iImpl = temp;
	//Destroy the old policy (which was swapped)
	CleanupStack::PopAndDestroy(&newPolicy);
	}

/**
Destroys the existing iImpl object and replaces it with aImpl parameter.

@internalComponent
*/
void RSqlSecurityPolicy::Set(CSqlSecurityPolicy& aImpl)
	{
	delete iImpl;
	iImpl = &aImpl;
	}

/**
@return A reference to the implementation object.

@panic SqlDb 2 Create() has not previously been called on  this RSqlSecurityPolicy object.

@internalComponent
*/
CSqlSecurityPolicy& RSqlSecurityPolicy::Impl() const
	{
	__SQLASSERT_ALWAYS(iImpl != NULL, ESqlPanicInvalidObj);
	return *iImpl;	
	}
