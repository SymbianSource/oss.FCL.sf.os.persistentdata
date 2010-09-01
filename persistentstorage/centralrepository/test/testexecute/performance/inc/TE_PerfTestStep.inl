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

// class TRepStatistics
inline TRepStatistics::TRepStatistics()
	: iRepUid(0), iUseCount(0), iSumLoadTicks(0), iCacheMisses(0)
	{
	}

inline TRepStatistics::TRepStatistics(const TRepStatistics& aRepStatistics)
	{
	iRepUid = aRepStatistics.iRepUid;
	iUseCount = aRepStatistics.iUseCount;
	iSumLoadTicks = aRepStatistics.iSumLoadTicks;
	iCacheMisses = aRepStatistics.iCacheMisses;
	}

inline TRepStatistics::TRepStatistics(TUint32 aRepUid, TUint32 aLoadTicks)
	: iRepUid(aRepUid), iUseCount(1), iSumLoadTicks(aLoadTicks), iCacheMisses(0)
	{
	}

inline TBool TRepStatistics::MatchUid(TUint32 aRepUid) const
	{
	return iRepUid == aRepUid;
	}

//-------------------------------------
// class TIpcStatistics

inline TIpcStatistics::TIpcStatistics()
	: iUseCount(0), iSumElapsedTicks(0)
	{
	}

