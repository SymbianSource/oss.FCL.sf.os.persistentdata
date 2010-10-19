// Copyright (c) 2004-2010 Nokia Corporation and/or its subsidiary(-ies).
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

namespace DBSC
{

//////////////////////////////////////////////////////////////////////////////////////////////
//class CPolicyBase

/**
*/
inline CPolicyBase::CPolicyBase()
	{
	}

/**
@return A const reference to the controlled collection of R/W/S policies.
*/
inline const CPolicyBase::RPolicyCollection& CPolicyBase::PolicyCollection() const
	{
	return iPolicyCollection;
	}

//////////////////////////////////////////////////////////////////////////////////////////////
//class CDbPolicy

/**
*/
inline CDbPolicy::CDbPolicy()
	{
	}

//////////////////////////////////////////////////////////////////////////////////////////////
//class CTblPolicy

/**
*/
inline CTblPolicy::CTblPolicy(const CDbPolicy* aDbPolicy) :
	iDbPolicy(aDbPolicy)
	{
	__ASSERT(iDbPolicy);
	}

/**
@return A const reference to the table name.
*/
inline const TDesC& CTblPolicy::TableName() const
	{
	DB_INVARIANT();
	return *iTblName;
	}

//////////////////////////////////////////////////////////////////////////////////////////////
//class CPolicyDomain

/**
CPolicyDomain collection of table security policies  - the granularity.
@internalComponent
*/
const TInt KTblPolicyCollGranularity = 32;

/**
@param aUid The domain UID
*/
inline CPolicyDomain::CPolicyDomain(TUid aUid) :
	iUid(aUid),
	iTPCollection(KTblPolicyCollGranularity)
	{
	}

/**
@return Policy domain UID.
*/
inline TUid CPolicyDomain::Uid() const
	{
	DB_INVARIANT();
	return iUid;
	}

/**
@return Backup&restore SID.
*/
inline TSecureId CPolicyDomain::BackupSID() const
	{
	DB_INVARIANT();
	return iBackupSID;
	}

//////////////////////////////////////////////////////////////////////////////////////////////
//class TPolicyDomainBuilder

/**
TPolicyDomainBuilder is a friend class of CPolicyDomain, which means that it can access
CPolicyDomain's data members and add/update new policies there.
The idea is that TPolicyDomainBuilder will be used by the implementors of MPolicyDomainLoader
interface, removing the need of making them friends of CPolicyDomain or visible from CPolicyDomain.
@param aPolicyDomain A reference to the policy domain object, which collection has to be 
                     created by the TPolicyDomainBuilder instance.
*/
inline TPolicyDomainBuilder::TPolicyDomainBuilder(CPolicyDomain& aPolicyDomain) :
	iPolicyDomain(aPolicyDomain)
	{
	}

/**
The method adds a table policy to the related CPolicyDomain collection.
@param aTblPolicy A pointer to CTblPolicy instance, which has to be added to 
                  the related CPolicyDomain collection. CPolicyDomain collection takes the
				  ownership on the supplied CTblPolicy instance.
*/
inline void TPolicyDomainBuilder::AddTblPolicyL(CTblPolicy* aTblPolicy)
	{
	__ASSERT(aTblPolicy);
	__LEAVE_IF_ERROR(iPolicyDomain.iTPCollection.Append(aTblPolicy));
	}

/**
The method initializes CPolicyDomain::iBackupSID data member.
The backup&restore SID can be ECapability_None, which means - no one is allowed to do backup&restore
for the databases, covered by current policy domain.
@param aTblPolicy aSecureId SID of the process, which is allowed to do backup&restore
                  for databases covered by current TPolicyDomainBuilder object.
*/
inline void TPolicyDomainBuilder::SetBackupSID(TSecureId& aSecureId)
	{
	iPolicyDomain.iBackupSID = aSecureId;
	}

//////////////////////////////////////////////////////////////////////////////////////////////
//class TPolicyDomainReader

/**
TPolicyDomainReader is a friend class of CPolicyDomain, which means that it can access
CPolicyDomain's data members and iterate through the policies collection.
The idea is that TPolicyDomainReader will be used by the implementors of MPolicyDomainPersister
interface, removing the need of making them friends of CPolicyDomain or visible from CPolicyDomain.
@param aPolicyDomain A reference to the policy domain object, which collection has to be 
                     traversed by the TPolicyDomainReader instance.
*/
inline TPolicyDomainReader::TPolicyDomainReader(const CPolicyDomain& aPolicyDomain) :
	iPolicyDomain(aPolicyDomain),
	iIndex(0)
	{
	}

/**
@return A const reference to the existing CDbPolicy instance - part of the related
        CPolicyDomain security policies collection.
*/
inline const CDbPolicy& TPolicyDomainReader::DbPolicy() const
	{
	__ASSERT(iPolicyDomain.iDbPolicy);
	return *iPolicyDomain.iDbPolicy;
	}

/**
Resets the iterator for a new scan from the beginning of the controlled table 
policies collection.
*/
inline void TPolicyDomainReader::ResetTblPos() const
	{
	iIndex = 0;
	}

/**
@return The count of security policies in the controlled table policies collection.
*/
inline TInt TPolicyDomainReader::TblPolicyCount() const
	{
	return iPolicyDomain.iTPCollection.Count();
	}

/**
@return A const pointer to the next CTblPolicy instance in the controlled collection
        of table security policies.
*/
inline const CTblPolicy* TPolicyDomainReader::NextTblPolicy() const
	{
	return iIndex < iPolicyDomain.iTPCollection.Count() ? iPolicyDomain.iTPCollection[iIndex++] : NULL;
	}

/**
@return Backup&restore process SID.
*/
inline TSecureId TPolicyDomainReader::BackupSID() const
	{
	return iPolicyDomain.iBackupSID;
	}

} //end of - namespace DBSC
