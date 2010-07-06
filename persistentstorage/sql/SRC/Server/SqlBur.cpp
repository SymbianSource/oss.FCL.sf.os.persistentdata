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
//

#include "SqlBur.h"
#include "SqlAssert.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "SqlBurTraces.h"
#endif
#include "SqlTraceDef.h"

#define UNUSED_ARG(arg) arg = arg

//Extracts and returns 32-bit integer from aNumBuf buffer.
static TUint32 GetNumUint32L(const TDesC& aNumBuf)
	{
	TLex lex(aNumBuf);
	lex.SkipSpace();
	TUint32 num = 0xFFFFFFFF;
	__SQLLEAVE_IF_ERROR2(lex.Val(num, EHex));
	return num;
	}

//Extracts and returns 64-bit integer from aNumBuf buffer.
static TInt64 GetNumInt64L(const TDesC& aNumBuf)
	{
	TLex lex(aNumBuf);
	lex.SkipSpace();
	TInt64 num = -1;
	__SQLLEAVE_IF_ERROR2(lex.Val(num, EHex));
	return num;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// string consts
_LIT(KRestoreFilter,"*.rst"); // the filter for restore files
_LIT(KBackupFilter,"*.bak");// the filter for backup files
_LIT(KRestoreSuffix,".bak.rst"); // the suffix for restore files (a shortcut by using double suffix :)

const TUint K8to16bitShift = 1;

/** Standard two phase construction
	@return an instance of the backup client
	@param a pointer to the SQL server which must have implemented the
			TSqlSrvBurInterface interface
	@leave if no memory
*/
CSqlBackupClient* CSqlBackupClient::NewLC(MSqlSrvBurInterface *aInterface)
	{
	CSqlBackupClient *self=(CSqlBackupClient *)new(ELeave) CSqlBackupClient(aInterface);
	CleanupStack::PushL(self);
	self->ConstructL();
	SQL_TRACE_BUR(OstTrace1(TRACE_INTERNALS, CSQLBACKUPCLIENT_NEWLC, "0x%X;CSqlBackupClient::NewLC", (TUint)self));
	return self;
	}

/** Standard two phase construction
	@return an instance of the backup client
	@param a pointer to the SQL server which must have implemented the
			TSqlSrvBurInterface interface
	@leave if no memory
*/
CSqlBackupClient* CSqlBackupClient::NewL(MSqlSrvBurInterface *aInterface)
	{
	CSqlBackupClient *self=(CSqlBackupClient *) NewLC(aInterface);
	CleanupStack::Pop();
	return self;
	}

/** Standard two phase construction
	@param a pointer to the SQL server which must have implemented the
			TSqlSrvBurInterface interface
*/		
CSqlBackupClient::CSqlBackupClient(MSqlSrvBurInterface *aInterface)
: CActive(EPriorityStandard), iInterface(aInterface)
	{
	}

/** Usual tidy up
*/
CSqlBackupClient::~CSqlBackupClient()
	{
	SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_CSQLBACKUPCLIENT2, "0x%X;CSqlBackupClient::~CSqlBackupClient;iFile.SubSessionHandle()=0x%X", (TUint)this, (TUint)iFile.SubSessionHandle()));
	
	// cancel outstanding requests
	Cancel();
	
	// release the pub/sub property
	iBurProperty.Close();
	
	// the file list array
	iFileList.Close();
	
	// close the file
	iFile.Close();
	
	// nuke the active backup client
    delete iActiveBackupClient;
	}

/** Standard two phase construction
	@leave if non memory or StartL leaves
*/	
void CSqlBackupClient::ConstructL()
	{
	// attach to backup/restore publish/subscribe property
	__SQLLEAVE_IF_ERROR(iBurProperty.Attach(KUidSystemCategory,KUidBackupRestoreKey));
	
	// add us to the scheduler
	CActiveScheduler::Add(this);

	// set active and request notification of changes to backup
	// and restore publish/subscribe property
	StartL();	
	}

/** 
Cancel the outstanding B&R request
*/
void CSqlBackupClient::DoCancel()
	{
	iBurProperty.Cancel();
	}

/** Not implemented
	@return a flag indicating whether we actioned the error
	@param the error unused
*/
TInt CSqlBackupClient::RunError(TInt aError)
	{
	UNUSED_ARG(aError);
	SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_RUNERROR, "0x%X;CSqlBackupClient::RunError;aError=%d", (TUint)this, aError));
	// just satisfy it that we did something!
	return KErrNone;
	}

/**	Kick off the BUR client
	@leave if TestBurStatusL leaves
*/
void CSqlBackupClient::StartL()
	{
    TestBurStatusL();
    NotifyChange();
	}

/** Resubscribe and wait for events
*/	
void CSqlBackupClient::NotifyChange()
	{
	SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_NOTIFYCHANGE, "0x%X;CSqlBackupClient::NotifyChange;iBurProperty.Handle()=0x%X", (TUint)this, (TUint)iBurProperty.Handle()));
	iBurProperty.Subscribe(iStatus);
	SetActive();
	}

/** Something happened. Find out what.
	Create an instance of BUR client if required
	Delete it if no longer required
	This is for performance reasons
	@leave if ConfirmReadyForBURL leaves
*/
void CSqlBackupClient::TestBurStatusL()
	{
	SQL_TRACE_BUR(OstTrace1(TRACE_INTERNALS, CSQLBACKUPCLIENT_TESTBURSTATUSL_ENTRY, "Entry;0x%X;CSqlBackupClient::TestBurStatusL", (TUint)this));
	TInt status;
	__SQLTRACE_BURVAR(TInt err = KErrNone);
	if((__SQLTRACE_BUREXPR(err =) iBurProperty.Get(status)) != KErrNotFound)
		{
		status&=KBURPartTypeMask;
#ifdef _SQL_RDEBUG_PRINT
		SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_TESTBURSTATUSL1, "0x%X;CSqlBackupClient::TestBurStatusL;status=%d", (TUint)this, status));
#else
		SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_TESTBURSTATUSL2, "0x%X;CSqlBackupClient::TestBurStatusL;status=%{TBURPartType}", (TUint)this, status));
#endif	    
		switch(status)
			{
			case EBURUnset: // same as EBURNormal
			case EBURNormal:
				delete iActiveBackupClient;
				iActiveBackupClient=NULL;
				break;
			case EBURBackupFull:
			case EBURBackupPartial:
            case EBURRestoreFull:
            case EBURRestorePartial:
				// we only do full backups and full restores
				if(!iActiveBackupClient)
					{
					iActiveBackupClient=CActiveBackupClient::NewL(this);
					}
				iActiveBackupClient->ConfirmReadyForBURL(KErrNone);
				break;
			default:
				break;
			}
		}
	SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_TESTBURSTATUSL_EXIT, "Exit;0x%X;CSqlBackupClient::TestBurStatusL;iProperty.Get() err=%d", (TUint)this, err));
	}

/** Called when BUE notifies a BUR event
	@leave if TestBurStatusL leaves
*/
void CSqlBackupClient::RunL()
	{
	NotifyChange();
	TestBurStatusL();
	}

/** This is supposed to allow the BUE to know in advance how much
	data is coming - but unfortunately there is no way to know this
	at this stage since we don't even know yet what SID is being processed
	So we just answer some number to make the BUE happy. It doesn't
	actually rely on this number so there is no risk - the aFinishedFlag
	indicates the end of data, not the value returned here. It is
	supposed to allow the BUE to optimise its behaviour by know up front
	the data volume.
	@return an arbitrary number
	@param TDrive unused
*/
TUint CSqlBackupClient::GetExpectedDataSize(TDriveNumber aDrive)
	{
	UNUSED_ARG(aDrive);
	// we have no idea at this point - we even don't know who is to be backed up yet
	const TUint KArbitraryNumber = 1024;
	SQL_TRACE_BUR(OstTraceExt3(TRACE_INTERNALS, CSQLBACKUPCLIENT_GETEXPECTEDDATASIZE, "0x%X;CSqlBackupClient::GetExpectedDataSize;aDrive=%d;rc=%u", (TUint)this, (TInt)aDrive, KArbitraryNumber));
	return KArbitraryNumber;
	}

/** This is the backup state machine
	Because the data has to be sent back in sections and the various
	components of the dataflow may straddle chunks, we have to keep
	track of where we are between each transfer - a state machine is
	the simplest and most understandable implementation
	@param TPtr this is where the data will be put to be passed back
	@param TBool set to true when all data has been submitted for backup
	@leave
*/
void CSqlBackupClient::GetBackupDataSectionL(TPtr8& aBuffer, TBool& aFinishedFlag)
	{
	SQL_TRACE_BUR(OstTraceExt3(TRACE_INTERNALS, CSQLBACKUPCLIENT_GETBACKUPDATASECTIONL0, "0x%X;CSqlBackupClient::GetBackupDataSectionL;iState=%d;iFileIndex=%d", (TUint)this, (TInt)iState, iFileIndex));
	// don't assume they set it to false
	aFinishedFlag=EFalse;
	// any files to backup
	if(iFileList.Count()==0)
		{
		SQL_TRACE_BUR(OstTrace1(TRACE_INTERNALS, CSQLBACKUPCLIENT_GETBACKUPDATASECTIONL1, "0x%X;CSqlBackupClient::GetBackupDataSectionL;file count is 0", (TUint)this));
		// nothing to backup - just return the finished flag
		aFinishedFlag=ETrue;
		// clear down the list
		iFileList.Reset();
		// iFileList closed in dtor
		return;
		}

	// run the state machine
	for(TInt bufFreeSpace=aBuffer.MaxSize()-aBuffer.Size(); bufFreeSpace>0; bufFreeSpace=aBuffer.MaxSize()-aBuffer.Size())
		{
		switch(iState)
			{
			case EBackupNoFileOpen: // open a file for processing
				{
				if(iFileIndex>=iFileList.Count())
					{
					SQL_TRACE_BUR(OstTrace1(TRACE_INTERNALS, CSQLBACKUPCLIENT_GETBACKUPDATASECTIONL2, "0x%X;CSqlBackupClient::GetBackupDataSectionL;all files processed", (TUint)this));
					// all files have been processed - send the finished flag
					aFinishedFlag=ETrue;
					// clear down the filelist
					iFileList.Reset();
					return;
					}
				// open the database file to send
				TInt rc=iFile.Open(	iInterface->Fs(), iFileList[iFileIndex].FullName(), EFileRead | EFileShareExclusive);
				__SQLTRACE_BUREXPR(TPtrC fname = iFileList[iFileIndex].FullName());
				SQL_TRACE_BUR(OstTraceExt5(TRACE_INTERNALS, CSQLBACKUPCLIENT_GETBACKUPDATASECTIONL3, "0x%X;CSqlBackupClient::GetBackupDataSectionL;BEGIN;fname=%S;iFileIndex=%d;iFile.SubSessionHandle()=0x%X;rc=%d", (TUint)this, __SQLPRNSTR(fname), iFileIndex, (TUint)iFile.SubSessionHandle(), rc));
				if(KErrNone!=rc)
					{
					// there's nothing we can do if we can't open the file so we just skip it
					++iFileIndex;
					break;
					}
				iState=EBackupOpenNothingSent;
				break;
				}
			case EBackupOpenNothingSent: // nothing sent (so far) for this file - send the header info
				{
				TInt64 fileSize;
				if(KErrNone!=iFile.Size(fileSize) || fileSize==0) // short circuit eval
					{
					// empty or unreadable - skip this file
					iState=EBackupEndOfFile;
					break;
					}
				
				// get the checksum - only grab last 4 bytes - enough to be satisfied that
				// the backup and restore worked ok
				TUint32 checksum = CheckSumL(iFile) & KMaxTUint32;

                // build the header - this is an instance member because it
                // has to persist over multiple calls to this method
				const TDesC& fileName = iFileList[iFileIndex].FullName();
				iBuffer.Format(_L("%8x%8x%4x%16lx%8x%S"),
					checksum,					// %8x
					KMagicNum,					// %8x
					KBackupHeaderVersion,		// %4x
					fileSize,					// %16lx
					fileName.Length(),			// %8x
					&fileName);					// %S
				SQL_TRACE_BUR(OstTraceExt4(TRACE_INTERNALS, CSQLBACKUPCLIENT_GETBACKUPDATASECTIONL5, "0x%X;CSqlBackupClient::GetBackupDataSectionL;fileName=%S;hdrPtr=|%S|;fileSize=%lld", (TUint)this, __SQLPRNSTR(fileName), __SQLPRNSTR(iBuffer), fileSize));
				
				// we need it to look like an 8bit buffer
				TPtr8 hdrPtr8((TUint8*)iBuffer.Ptr(), iBuffer.Size(), iBuffer.Size());
							
				TInt len = Min(hdrPtr8.Size(), bufFreeSpace);
				
				// append the header to the buffer (only till it's full)
				aBuffer.Append(hdrPtr8.Ptr(), len);
				
				// decide what needs to happen next
				// if complete then we need data, otherwise we need to put
				// the rest of the header in the next chunk
				if(hdrPtr8.Size() <= bufFreeSpace)
					{
					iState = EBackupOpenAllHeaderSent;
					}
				else
					{
					// we need to keep track of how much of the header has
					// been sent so that we only send the reminder on the next
					// iteration
					iHeaderSent = len;
					iState = EBackupOpenPartHeaderSent;
					}
				break;
				}
			case EBackupOpenPartHeaderSent: // need to send the rest of the header
				{
				// get back the header - this is already loaded with the necessary info
				// from the previous state we were in - EBackupOpenNothingSent
				
				// we need it to look like an 8bit buffer
				TPtr8 hdrPtr8((TUint8*)iBuffer.Ptr(), iBuffer.Size(), iBuffer.Size());
				
				// how many bytes have we yet to send?
				TInt bytesRemaining = hdrPtr8.Size() - iHeaderSent;
				TInt len = Min(bytesRemaining, bufFreeSpace);
				aBuffer.Append(hdrPtr8.Ptr() + iHeaderSent, len);
				
				if(bytesRemaining <= bufFreeSpace)
					{
					iHeaderSent = 0; // ready for next header
					iState = EBackupOpenAllHeaderSent;
					}
				else
					{
					iHeaderSent += len; // ready to do round again
					//iState=EBackupOpenPartHeaderSent; same state as now!
					}
				break;
				}
			case EBackupOpenAllHeaderSent: // need to send some data
				{
				TPtr8 ptr((TUint8*)aBuffer.Ptr() + aBuffer.Size(), 0, bufFreeSpace);
				__SQLLEAVE_IF_ERROR(iFile.Read(ptr));
				TInt bytesRead = ptr.Size();
				aBuffer.SetLength(aBuffer.Size() + bytesRead);
				// EOF
				if(bytesRead == 0)
					{
					iState = EBackupEndOfFile;
					break;
					}
				break;
				}
			case EBackupEndOfFile:
				{
				SQL_TRACE_BUR(OstTraceExt3(TRACE_INTERNALS, CSQLBACKUPCLIENT_GETBACKUPDATASECTIONL4, "0x%X;CSqlBackupClient::GetBackupDataSectionL;END;iFile.SubSessionHandle()=0x%X;iFileIndex=%d", (TUint)this, (TUint)iFile.SubSessionHandle(), iFileIndex));
				iFile.Close();
				++iFileIndex; // move on to next file
				iState = EBackupNoFileOpen; // go round again
				break;
				}
			default:
				{
				break;
				}
			}//end of the "switch" statement
		}//end of the "for" statement
	}

/** This is called by BUE when the restore has completed
	Nothing to do here except tell the server
	@param TDrive the drive that is being restored (unused)
*/
void CSqlBackupClient::RestoreComplete(TDriveNumber aDrive)
	{
	UNUSED_ARG(aDrive);
	SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_RESTORECOMPLETE, "0x%X;CSqlBackupClient::RestoreComplete;aDrive=%d", (TUint)this, (TInt)aDrive));
	}

/** This is called to let us know that the given SID is to be backed up
	We ask the SQL server for a list of databases that want to be backed
	up - this is because the backup flag is an internal metadata object
	in the database, and to decouple we don't want to have to know how
	this data is stored.
	@param TSecureSid the UID of the application to backup
	@param TDriveNumber the drive to be backed up (unused)
	@leave
*/
void CSqlBackupClient::InitialiseGetProxyBackupDataL(TSecureId aSid, TDriveNumber aDrive)
	{
	UNUSED_ARG(aDrive);
	SQL_TRACE_BUR(OstTraceExt3(TRACE_INTERNALS, CSQLBACKUPCLIENT_INITIALIZEGETPROXYBACKUPDATAL, "0x%X;CSqlBackupClient::InitialiseGetProxyBackupDataL;aSid=0x%X;aDrive=%d", (TUint)this, (TUint)aSid.iId, (TInt)aDrive));
	// get the list of database files to back up - this is provided by the SQL server
	GetBackupListL(aSid);
	// this is the index of the file being processed - point to the beginning
	iFileIndex=0;
	// the first state of the backup state machine
	iState=EBackupNoFileOpen;
	// save the sid for notifying the server when the backup is complete
	iSid=aSid;
	}

/** Called when the BUE wants to start sending data to us
	@param TSecureId the UID of the application that is to be restored
	@param TDriveNumber the drive to restore (unused)
	@leave
*/
void CSqlBackupClient::InitialiseRestoreProxyBaseDataL(TSecureId aSid, TDriveNumber aDrive)
	{
	UNUSED_ARG(aDrive);
	SQL_TRACE_BUR(OstTraceExt3(TRACE_INTERNALS, CSQLBACKUPCLIENT_INITIALIZERESTOREPROXYBASEDATAL, "0x%X;CSqlBackupClient::InitialiseRestoreProxyBaseDataL;aSid=0x%X;aDrive=%d", (TUint)this, (TUint)aSid.iId, (TInt)aDrive));
	iBuffer.Zero();
	// this is the first state of the restore state machine
	iState=ERestoreExpectChecksum;
	iAnyData=EFalse; // to keep track in the state machine whether any data was actually sent
	// save the sid for notifying the server when the restore is done
	iSid=aSid;
	}

/** This is repeatedly called by the BUE to send us chunks of restore data (for the current SID)
    Becuase the data is spread over chunks we need to manage the state across mutiple calls
    to this method so we use a state machine
    @leave KErrCorrupt if the data is incomplete or the checksum fails
    @param TDesc8 the data to be restored
    @param TBool set when there is not more data to restore

Attention!!! This function won't work properly if aInBuffer parameter contains odd number of bytes!!!
(a legacy problem, if it is a problem at all, because the B&R engine probably sends the data in chunks with even size)
*/
void CSqlBackupClient::RestoreBaseDataSectionL(TDesC8& aInBuffer, TBool aFinishedFlag)
	{
	SQL_TRACE_BUR(OstTraceExt4(TRACE_INTERNALS, CSQLBACKUPCLIENT_RESTOREBASEDATASECTONL0, "0x%X;CSqlBackupClient::RestoreBaseDataSectionL;iState=%d;aInBuffer.Length()=%d;aFinishedFlag=%d", (TUint)this, (TInt)iState, aInBuffer.Length(), (TInt)aFinishedFlag));
	// used to walk the buffer
	// got a new buffer - because each time this method is called, we have a
	// fresh chunk of data
	TInt inBufferPos = 0;

	// to mark when the state machine is through
	TBool done = EFalse;
	
	// check whether this is an empty restore
	if(aFinishedFlag && !iAnyData)
		{
		// we have to do this and not rely on aFinishedFlag alone, becuase
		// if aFinished is used, we'll process the last state of the machine
		// which does tidyup, except that if there was no data, no tidyup should
		// be done
		return;
		}
		
	// run the machine
	do
		{
		// how many bytes are there available in the buffer for processing?
		TInt bytesAvailable = aInBuffer.Size() - inBufferPos;
		// the reason why we are testing finishedFlag is because we must
		// make sure we re-enter the machine to do the tidyup
		if(bytesAvailable <= 0 && !aFinishedFlag)
			{
			// ran out of data in the chunk
			// so we return and wait for more data to arrive
			return;
			}
		if(aFinishedFlag && iState != ERestoreComplete && iState != ERestoreExpectData)
			{
			// ran out of data early
			// will be ERestoreComplete if data not aligned on 128
			// will be ERestoreExpectData if data aligned on 128
			__SQLLEAVE(KErrCorrupt);
			}
		// yep there was some data in the chunk if we got here
		if(bytesAvailable > 0)
			{
			iAnyData = ETrue;
			}
		switch(iState)
			{
			case ERestoreExpectChecksum: // 16 bytes (the header is UTF16 encoded, 8 unicode characters for the checksum)
				{
				const TInt KCheckSumStrLen = 8;
				CopyBufData(aInBuffer, inBufferPos, iBuffer, KCheckSumStrLen);
				if(iBuffer.Length() == KCheckSumStrLen)
					{
					iChecksum = ::GetNumUint32L(iBuffer);
					iState = ERestoreExpectOldFileSize;
					iBuffer.Zero();
					}
				break;
				}
			case ERestoreExpectOldFileSize: // 16 bytes (the header is UTF16 encoded, 8 unicode characters for 32-bit old file size)
				{
				const TInt KOldFileSizeStrLen = 8;
				CopyBufData(aInBuffer, inBufferPos, iBuffer, KOldFileSizeStrLen);
				if(iBuffer.Length() == KOldFileSizeStrLen)
					{
					TUint32 oldFileSize = ::GetNumUint32L(iBuffer);
					if(oldFileSize == KMagicNum)
						{
						iState = ERestoreExpectVersion;
						}
					else
						{
						iFileSize = oldFileSize;	
						iState = ERestoreExpectFileNameSize;
						}
					iBuffer.Zero();
					}
				break;
				}	
			case ERestoreExpectVersion:
				{
				const TInt KVersionStrLen = 4;
				CopyBufData(aInBuffer, inBufferPos, iBuffer, KVersionStrLen);
				if(iBuffer.Length() == KVersionStrLen)
					{
					//Ignore the version: ::GetNumUint32L(iBuffer);	
					//At this stage we know that the version is 2+
					iState = ERestoreExpectFileSize;
					iBuffer.Zero();
					}
				break;
				}
			case ERestoreExpectFileSize:
				{
				const TInt KFileSizeStrLen = 16;
				CopyBufData(aInBuffer, inBufferPos, iBuffer, KFileSizeStrLen);
				if(iBuffer.Length() == KFileSizeStrLen)
					{
					iFileSize = GetNumInt64L(iBuffer);	
					iState = ERestoreExpectFileNameSize;
					iBuffer.Zero();
					}
				SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_RESTOREBASEDATASECTONL1, "0x%X;CSqlBackupClient::RestoreBaseDataSectionL;iFileSize=%lld", (TUint)this, iFileSize));
				break;
				}
			case ERestoreExpectFileNameSize: // the size of the file name to restore
				{
				const TInt KFileNameLenStrLen = 8;
				CopyBufData(aInBuffer, inBufferPos, iBuffer, KFileNameLenStrLen);
				if(iBuffer.Length() == KFileNameLenStrLen)
					{
					iFileNameSize = GetNumUint32L(iBuffer);		
					iState = ERestoreExpectFileName;
					iBuffer.Zero();
					}
				break;
				}
			case ERestoreExpectFileName:  // the name of the file to restore
				{
				CopyBufData(aInBuffer, inBufferPos, iBuffer, iFileNameSize);
				SQL_TRACE_BUR(OstTraceExt4(TRACE_INTERNALS, CSQLBACKUPCLIENT_RESTOREBASEDATASECTONL2, "0x%X;CSqlBackupClient::RestoreBaseDataSectionL;BEGIN;iBuffer=%S;iBuffer.Length()=%d;iFileNameSize=%d", (TUint)this, __SQLPRNSTR(iBuffer), iBuffer.Length(), iFileNameSize));
				if(iBuffer.Length() == iFileNameSize)
					{
					iState = ERestoreExpectData;
					iBuffer.Append(KRestoreSuffix);
					// now we start writing the data to the target file
					// write to a temp - double disk space potentially
					// once all the temp files are created, then they are renamed to the
					// real file names in one fell swoop
					__SQLLEAVE_IF_ERROR(iFile.Replace(iInterface->Fs(), iBuffer, EFileWrite | EFileShareExclusive));
					SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_RESTOREBASEDATASECTONL3, "0x%X;CSqlBackupClient::RestoreBaseDataSectionL;iFile.SubSessionHandle()=0x%X", (TUint)this, (TUint)iFile.SubSessionHandle()));
					iBuffer.Zero();
					}
				break;
				}
			case ERestoreExpectData: // now for the data
				{
				TInt len = Min((aInBuffer.Size() - inBufferPos), iFileSize);
				__SQLLEAVE_IF_ERROR(iFile.Write(aInBuffer.Mid(inBufferPos, len)));
				inBufferPos += len;
				iFileSize -= len;
				if(iFileSize == 0)
					{
					iState = ERestoreComplete;
					}
				break;
				}
			case ERestoreComplete: // file completely restored
				{
				// calculate the checksum
				TUint32 cksum = CheckSumL(iFile) & KMaxTUint32;
				
				// done with the file now - has to follow checksum cos it
				// expects ann open file
				SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_RESTOREBASEDATASECTONL4, "0x%X;CSqlBackupClient::RestoreBaseDataSectionL;END;iFile.SubSessionHandle()=0x%X", (TUint)this, (TUint)iFile.SubSessionHandle()));
			    __SQLLEAVE_IF_ERROR(iFile.Flush());
				iFile.Close();

                // validate that the checksum matches
                if(cksum!=iChecksum)
                    {
                    __SQLLEAVE(KErrCorrupt);
                    }
				
				// end of data - or another file to be restored?
				if(aFinishedFlag)
					{
					// we need to rename all the
					// temp rst files to the real database names
					CDir *dir=NULL;
					__SQLLEAVE_IF_ERROR(iInterface->Fs().GetDir(KRestoreFilter,KEntryAttNormal,ESortNone,dir));
					CleanupStack::PushL(dir);
					TInt err2 = KErrNone;
					for(TInt a=0;a<dir->Count();++a)
						{
						TEntry entry=(*dir)[a];
						TPtrC rst=entry.iName.Des();
						TInt len=rst.Length();
						// format <filename>.db.bak.rst
						// just a convenience!
						TPtrC bak(rst.Left(len - 4));//".rst" part excluded
						TPtrC db(rst.Left(len - 8));//".bak.rst" part excluded
						
						// first, rename the orig .db as .bak just in case
						// ok if not found - might have been deleted.
						//the ".bak" file, if exists, will be deleted first.
						(void)iInterface->Fs().Delete(bak);
						TInt err=iInterface->Fs().Rename(db,bak);
						SQL_TRACE_BUR(OstTraceExt4(TRACE_INTERNALS, CSQLBACKUPCLIENT_RESTOREBASEDATASECTONL5, "0x%X;CSqlBackupClient::RestoreBaseDataSectionL;END;bak=%S;db=%S;err=%d", (TUint)this, __SQLPRNSTR(bak), __SQLPRNSTR(db), err));
						if(err!=KErrNone && err!=KErrNotFound)
							{
							__SQLLEAVE(err);
							}
						
						// now, rename the .rst as .db
						err = iInterface->Fs().Rename(rst,db);
						if(err != KErrNone && err2 == KErrNone)
							{
							//The idea here is to not report the error immediatelly by calling LeaveIfError().
							//If we leave here, the next database restore may also fail, for example, if the current database is still open by 
							//its owner. Then "TInt err=iInterface->Fs().Rename(db,bak);" will fail again.
							err2 = err;
							}
						// if we got here, we have a backup of the original database in .db.bak
						// and the new database in .db
						}
					__SQLLEAVE_IF_ERROR(err2);
					
					// clean up dir
					//delete dir;
					CleanupStack::PopAndDestroy(dir);
					dir=NULL;
					
					// now delete all the .bak files
					// we do this here and not part of the earlier loop
					// because we want to make sure that we have a coherent set of database
					// files that belong together and not bits of old and new
					__SQLLEAVE_IF_ERROR(iInterface->Fs().GetDir(KBackupFilter,KEntryAttNormal,ESortNone,dir));
					CleanupStack::PushL(dir);
					SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_RESTOREBASEDATASECTONL6, "0x%X;CSqlBackupClient::RestoreBaseDataSectionL;bak files count=%d", (TUint)this, dir->Count()));
					for(TInt a1=0;a1<dir->Count();++a1)
						{
						TEntry entry=(*dir)[a1];
						TPtrC bak=entry.iName.Des();
						__SQLLEAVE_IF_ERROR(iInterface->Fs().Delete(bak));
						}
					
					// clean up dir
					//delete dir;
					CleanupStack::PopAndDestroy(dir);
					dir=NULL;
					done=ETrue;
					}
				else
					{
					iState=ERestoreExpectChecksum;
					}
					
				break;
				}
			default:
				break;
			}
		} while(!done);
	}

/** The operation was terminated - we should tidyup here (as best we can)
	Nothing needs to be done for a backup. Restore is more
	complicated in the case of an interruption.
	What we need to do here is move all the backup files
	back to being db files....
*/	
void CSqlBackupClient::TerminateMultiStageOperation()
	{
	// backup/restore terminated, try to tidy up! Can't leave, can't Panic!!!!!
	// rename all the .bak files to .db
	CDir *dir=NULL;
	TInt rc=iInterface->Fs().GetDir(KBackupFilter,KEntryAttNormal,ESortNone,dir);
	SQL_TRACE_BUR(OstTraceExt3(TRACE_INTERNALS, CSQLBACKUPCLIENT_TERMINATEMULTISTAGEOPERATION1, "0x%X;CSqlBackupClient::TerminateMultiStageOperation;Fs().GetDir() err=%d;file count=%d", (TUint)this, rc, rc == KErrNone ? dir->Count() : 0));
	if(KErrNone!=rc)
		{
		// can't get a file list - can't do anything
		return;
		}
	for(TInt a=0;a<dir->Count();++a)
		{
		TEntry entry=(*dir)[a];
		TPtrC bak=entry.iName.Des();
		TInt len=bak.Length();
		TPtrC db(bak.Left(len-4));//".bak" part excluded
		rc=iInterface->Fs().Delete(db); // rename does not overwrite
		if(KErrNone == rc)
			{
	        rc = iInterface->Fs().Rename(bak,db);
			}
        //The function cannot leave or return an error. The only thing which could be done here is to print out something
		//and continue with the next file.
		if(KErrNone != rc)
		    {
			SQL_TRACE_BUR(OstTraceExt4(TRACE_INTERNALS, CSQLBACKUPCLIENT_TERMINATEMULTISTAGEOPERATION2, "0x%X;CSqlBackupClient::TerminateMultiStageOperation;Fs().Rename() err=%d;bak=%S;db=%S", (TUint)this, rc, __SQLPRNSTR(bak), __SQLPRNSTR(db)));
		    }
		// backup restored ok
		}
	// cleanup dir
	delete dir;
	}

/** We do our own checksumming so we don't need this
	@return the checksum
	@param TDriveNumber the drive affected (unused)
*/
TUint CSqlBackupClient::GetDataChecksum(TDriveNumber /* aDrive */)
	{
	// not required - not implemented
	const TUint KArbitraryNumber = 1024;
	return KArbitraryNumber;
	}

/** We don't support incremental backup
*/
void CSqlBackupClient::GetSnapshotDataL(TDriveNumber /* aDrive */, TPtr8& /* aBuffer */,
										TBool& /* aFinishedFlag */)
	{
	// incremental backup not supported
	__SQLLEAVE(KErrNotSupported);
	}

/** We don't support incremental backup
*/
void CSqlBackupClient::InitialiseGetBackupDataL(TDriveNumber /* aDrive */)
	{
	// incremental backup not supported
	__SQLLEAVE(KErrNotSupported);
	}

/** We don't support incremental backup
*/
void CSqlBackupClient::InitialiseRestoreBaseDataL(TDriveNumber /* aDrive */)
	{
	// incremental backup not supported
	__SQLLEAVE(KErrNotSupported);
	}

/** We don't support incremental backup
*/
void CSqlBackupClient::InitialiseRestoreIncrementDataL(TDriveNumber /* aDrive */)
	{
	// incremental backup not supported
	__SQLLEAVE(KErrNotSupported);
	}

/** We don't support incremental backup
*/
void CSqlBackupClient::RestoreIncrementDataSectionL(TDesC8& /* aBuffer */, TBool /* aFinishedFlag */)
	{
	// incremental backup not supported
	__SQLLEAVE(KErrNotSupported);
	}

/** We don't support incremental backup
*/
void CSqlBackupClient::AllSnapshotsSuppliedL()
	{
	// incremental backup not supported
	// cannot leave or panic!
	}

/** We don't support incremental backup
*/
void CSqlBackupClient::ReceiveSnapshotDataL(TDriveNumber /* aDrive */, TDesC8& /* aBuffer */,
									TBool /* aFinishedFlag */)
	{
	// incremental backup not supported
	__SQLLEAVE(KErrNotSupported);
	}

/**
	Get a list of database files that need to be backed up
	This is decided by the SQL server on the basis of the UID provided
	and whether the metadata in the database indicates that this data
	should be backed up or not. The list of database files is populated
	into the iFileList array.
	@leave
	@param TSecureSid the UID of the data owner
*/
void CSqlBackupClient::GetBackupListL(TSecureId aSid)
	{
	SQL_TRACE_BUR(OstTraceExt2(TRACE_INTERNALS, CSQLBACKUPCLIENT_GETBACKUPLISTL, "0x%X;CSqlBackupClient::GetBackupListL;aSid=0x%X", (TUint)this, (TUint)aSid.iId));
	// we own the array - the SQL server just populates it
	iInterface->GetBackUpListL(aSid,iFileList);
	}

/** A simple checksumming algorithm to allow a degree
	of trust that the backup and restore worked
	This is visble externally because the test harness
	needs to use it - NOTE the file pointer will be back at the
	start when this function ends.
	@leave
	@param RFile64 an OPEN file to checksum
*/
TUint64 CSqlBackupClient::CheckSumL(const RFile64& aOpenFile) const
	{
	// scoot through the database file building the checksum
	TInt64 seekPos=0; // rewind first
	__SQLLEAVE_IF_ERROR(aOpenFile.Seek(ESeekStart,seekPos));
	TUint64 total=0;
	const TUint KCheckSumBlockSize = 4 * 1024;
	HBufC8* block=HBufC8::NewLC(KCheckSumBlockSize);
	TPtr8 ptr=block->Des();
	for(;;)
		{
		__SQLLEAVE_IF_ERROR(aOpenFile.Read(ptr));
		TInt len=ptr.Length();
		if(len==0)
			{
			break;
			}
		// calculate the checksum
		for(TInt i=0;i<len;++i)
			{
			total = (total << 1) | (total >> 63);
			total += ptr[i];
 			}
		};		
	CleanupStack::PopAndDestroy(block);
	// restore file position
	seekPos=0;
	__SQLLEAVE_IF_ERROR(aOpenFile.Seek(ESeekStart,seekPos));
	return total;
	}

//Reads the content of aInBuf from position aInBufReadPos and stores the data into aOutBuf.
//aDataLen is the length of the data. If the input buffer does not contain all the data, then only the
//available data will be copied to the output buffer.
//
//Attention!!! This function won't work properly if aInBuf parameter contains odd number of bytes!!!
//(a legacy problem, if it is a problem at all, because the B&R engine probably sends the data in chunks with even size)
//
//How the function works. It is called during the restore process and aInBuf parameter contains a block of raw
//data sent by the B&R server. The calling function, RestoreBaseDataSectionL(), uses a state 
//machine to processes the incoming data. At particular moment RestoreBaseDataSectionL() will process the data header 
//and will have to read "aDataLen" 16-bit characters at position "aInBufReadPos". If there are "aDataLen" characters
//at position "aInBufReadPos" and enough free space in "aOutBuf", CopyBufData() will copy all of them,  
//otherwise CopyBufData() will copy as much characters as possible (in which case RestoreBaseDataSectionL() will
//stay in the same state, waiting for more data from the B&R server).
//
void CSqlBackupClient::CopyBufData(const TDesC8& aInBuf, TInt& aInBufReadPos, TDes& aOutBuf, TInt aDataLen)
	{
    __ASSERT_DEBUG(aInBufReadPos >= 0, __SQLPANIC(ESqlPanicBadArgument));
    __ASSERT_DEBUG(aDataLen > 0, __SQLPANIC(ESqlPanicBadArgument));
    __ASSERT_DEBUG(!(aInBuf.Length() & 0x01), __SQLPANIC(ESqlPanicInternalError));
	
	TInt needed = (aDataLen - aOutBuf.Length()) << K8to16bitShift;
	TInt available = aInBuf.Size() - aInBufReadPos;
	TInt len = Min(needed, available);
	TPtrC8 ptr8 = aInBuf.Mid(aInBufReadPos, len);
	aInBufReadPos += len;
	
	len >>= K8to16bitShift;
	aOutBuf.Append((const TUint16*)ptr8.Ptr(), len);
	}
