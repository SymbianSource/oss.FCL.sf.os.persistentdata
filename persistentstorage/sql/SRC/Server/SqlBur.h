// Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __SQLBUR_H__
#define __SQLBUR_H__

#include <e32base.h>
#include <f32file.h>
#include <f32file64.h>
#include <e32property.h>
#include <connect/abclient.h> // MactiveBackupDataClient
#include "SqlSrvBurInterface.h"

using namespace conn;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////                     Backup database file header format                           ///////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////// No version (Version 0)
//  8 chars          8 chars          8 chars             up to 256 characters (512 bytes)
// <32-bit checksum><32-bit filesize><32-bit filenamelen><filename - UTF16 encoded>

//////             Version 2
//  8 chars          8 chars   4 chars     16 chars         8 chars             up to 256 characters (512 bytes)
// <32-bit checksum><FFFFAA55><Version N#><64-bit filesize><32-bit filenamelen><filename - UTF16 encoded>

const TInt KBackupHeaderVersion = 2;            //Current backup database file header version

const TUint32 KMagicNum = 0xFFFFAA55;           //Magic number. If the "old database file size" field in the header
                                                //has this value, then the header version is 2+
const TInt KMaxHeaderSize = 256 + KMaxFileName; //The size of the buffer used for the operations on the header

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------
// CSqlBackupClient
//-----------------------------------------

/**
	This class is called by the Backup and Restore Framework
	when a backup or restore is requested by the user
	It implements the active proxy backup and restore as
	defined in the MActiveBackupDataClient interface.
	
	@internalComponent
*/

// derives from the framework mixin MActiveBackupClient
class CSqlBackupClient : public CActive, public MActiveBackupDataClient
	{
	public:
		static CSqlBackupClient *NewLC(MSqlSrvBurInterface *aBurInterface);
		static CSqlBackupClient *NewL(MSqlSrvBurInterface *aBufInterface);
		
		~CSqlBackupClient();
		
		// AO implementations
		void StartL();
		void NotifyChange();
		
		// impl of virtuals from MActiveBackupDataClient	
		void AllSnapshotsSuppliedL();
		void ReceiveSnapshotDataL(TDriveNumber aDrive, TDesC8& aBuffer, TBool aLastSection);
		TUint GetExpectedDataSize(TDriveNumber aDrive);
		void GetSnapshotDataL(TDriveNumber aDrive, TPtr8& aBuffer, TBool& aFinished);
		void InitialiseGetBackupDataL(TDriveNumber aDrive);
		void GetBackupDataSectionL(TPtr8& aBuffer, TBool& aFinished);
		void InitialiseRestoreBaseDataL(TDriveNumber aDrive);
		void RestoreBaseDataSectionL(TDesC8& aBuffer, TBool aFinished);
		void InitialiseRestoreIncrementDataL(TDriveNumber aDrive);
		void RestoreIncrementDataSectionL(TDesC8& aBuffer, TBool aFinished);
		void RestoreComplete(TDriveNumber aDrive);
		void InitialiseGetProxyBackupDataL(TSecureId aSid, TDriveNumber aDrive);
		void InitialiseRestoreProxyBaseDataL(TSecureId aSid, TDriveNumber aDrive);
		void TerminateMultiStageOperation();
		TUint GetDataChecksum(TDriveNumber aDrive);	
		
		// to validate successful BUR
		TUint64 CheckSumL(const RFile64 &aOpenFile) const;
		
		// for debug
		//void SetConsole(CConsoleBase *aConsole);
	private:
		CSqlBackupClient(MSqlSrvBurInterface *aInterface);
		void ConstructL();
		
		// active object methods
		void RunL();
		void DoCancel();
		TInt RunError(TInt aError);
		
		// used to determine what the BUR status is and to respond as required
		void TestBurStatusL();
	
		// this is used to ask the SQL server for a list of databases to backup
		// we ask the database server because the decision to backup is not
		// only based on the UID but also the database backup flag stored in
		// the database metadata - and we don't want to have to know how that
		// is implemented
		void GetBackupListL(TSecureId aSid);
		void CopyBufData(const TDesC8& aInBuf, TInt& aInBufReadPos, TDes& aOutBuf, TInt aDataLen);
		
	private:
	
		// state machine for backup
		enum
			{
			EBackupNoFileOpen=0, // not currently processing a file
			EBackupOpenNothingSent, // file open and ready, but nothing sent yet
			EBackupOpenPartHeaderSent, // part of the header is sent, but more remains
			EBackupOpenAllHeaderSent, // all of the header is sent, ready to send the data
			EBackupEndOfFile // all done, tidy up after backup
			};

		// state machine for restore
		// this is more complicated because we are driven by the backup engine
		// and have incomplete information most of the time
		enum
			{
			ERestoreExpectChecksum=0, 		// checksum marks the start of the next file
			ERestoreExpectOldFileSize, 		// the size of the file - backup file header version 0
			ERestoreExpectVersion, 			// backup header version
			ERestoreExpectFileSize, 		// the size of the file, backup file header version 2+
			ERestoreExpectFileNameSize, 	// the size of the file name
			ERestoreExpectFileName, 		// the name of the file to restore
			ERestoreExpectData, 			// now for the data
			ERestoreComplete 				// tidy up after restore
			};
			
		CActiveBackupClient *iActiveBackupClient;
		RProperty iBurProperty;//B&R property published by the B&R server. SQL server subscribes for notifications.
		MSqlSrvBurInterface *iInterface; // the SQL server
		RArray<TParse> iFileList; // which is populated by the SQL server
		RFile64 iFile;
		TInt iFileIndex;
		TUint iState;
		TBuf<KMaxHeaderSize> iBuffer; // used for the header data
		TInt iHeaderSent; // how many header bytes sent so far
		TUint32 iChecksum; // used by restore
		TInt64 iFileSize; // used by restore
		TUint32 iFileNameSize; // used by restore
		TBool iAnyData; // set to true if the restore actually sends any data to us
		TSecureId iSid; // the sid being backed up/restored

	};

#endif // __SQLBUR_H__
