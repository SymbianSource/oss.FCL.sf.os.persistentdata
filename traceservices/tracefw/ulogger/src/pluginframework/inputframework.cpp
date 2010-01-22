// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// CInputFramework.cpp
// 
//

#include "inputframework.h"
#include "uloggershared.h"
//#include "uloggertools.h"

#if defined(__LIGHTLOGGER_ENABLED)
#include "lightlogger.h" 
#endif

namespace Ulogger
{

CInputFramework::CInputFramework(MInputPlugin *aInputPlugin, MInputFrameworkObserver *aObserver)
:CActive(CActive::EPriorityHigh),
iInputPlugin(aInputPlugin),
iDataPtr(NULL, 0),
iObserver(aObserver)
	{
	CActiveScheduler::Add(this);
	iData = NULL;
	iInputData = NULL;
	iContinueReading = EFalse;
	}

EXPORT_C CInputFramework::~CInputFramework()
	{
	if(this->IsActive())
		this->Cancel();

	if(iInputPlugin)
		iInputPlugin->CloseInputPlugin();
		
	if(iData)
		{
		delete iData;
		iData = NULL;
		}
	if(iInputData)
		{
		delete iInputData;
		iInputData = NULL;
		}
	
	delete iCommand;
	iCommand = NULL;
	
	iControlData.Close();
	}


EXPORT_C CInputFramework* CInputFramework::NewL(MInputPlugin *aInputPlugin, const RPointerArray<TPluginConfiguration>& aInputSettings, MInputFrameworkObserver *aObserver)
	{
	CInputFramework* obj = CInputFramework::NewLC(aInputPlugin, aInputSettings, aObserver);
	CleanupStack::Pop(); //obj
	return obj;
	}


EXPORT_C CInputFramework* CInputFramework::NewLC(MInputPlugin *aInputPlugin, const RPointerArray<TPluginConfiguration>& aInputSettings, MInputFrameworkObserver *aObserver)
	{
	CInputFramework *obj = new (ELeave) CInputFramework(aInputPlugin, aObserver);
	CleanupStack::PushL(obj);
	obj->ConstructL(aInputSettings);
	return obj;
	}


void CInputFramework::ConstructL(const RPointerArray<TPluginConfiguration>& aInputSettings)
	{
	#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
	__MARK_METHOD("CInputFramework::ConstructL")
	#endif	
	
	if(!iInputPlugin)
		return;
	
	//copy settings
	for(TInt i=0; i<aInputSettings.Count(); ++i)
		{
		TPluginConfiguration* pluginConfig = new TPluginConfiguration();
		pluginConfig->SetKey(aInputSettings[i]->Key());
		pluginConfig->SetValue(aInputSettings[i]->Value());
		iInputSettings.AppendL(pluginConfig);
		}

	//configure control plugin
  	User::LeaveIfError(iInputPlugin->ConfigureInputPlugin(iInputSettings));
	iData = HBufC8::NewL(1024);
	iDataPtr.Set(iData->Des());
	User::LeaveIfError(iControlData.Create(1024)); //can be reallocated when needed
	
	//create control data manager
	iInputData = new (ELeave) CInputData();
	
	//create command parser
	iCommand = CCommand::NewL(this);
	}


void CInputFramework::DoCancel()
	{
	iContinueReading = EFalse;
	if(iInputPlugin)
		iInputPlugin->CancelReadData();
	}

void CInputFramework::RunL()
	{
	#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
	__LOG("CInputFramework::RunL, errCode:")
	__LOGNUM(iStatus.Int())
	#endif
	
	if(iStatus.Int() == KErrNone)
		{
		//process control data
		#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
		__LOG("starting to parse recieved control data")
		#endif
		
		//read control data and merge chunks if nesessary
		if(iControlData.MaxSize() < iControlData.Length()+iDataPtr.Length())
			User::LeaveIfError(iControlData.ReAlloc(iControlData.Length()+iDataPtr.Length()+8));
		iControlData.Append(iDataPtr);
		
		//check is we have full control data package
		TBuf8<2> eol;
		eol.Copy((const unsigned char*)&CD_EOL, SIZE_OF_EOL);
		TInt eolExist = iControlData.Find(eol);
		if(eolExist == KErrNotFound)
			{
			iControlData.Zero();
			this->StartReading();
			return;
			}
		
		//create control data raw format
		const ControlData *cData =  (const ControlData*)iControlData.Ptr();
		
		#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
		__LOG("completed control data:")
		__LOGBUF(iControlData)
		User::InfoPrint(_L("processing control data"));
		#endif
		
		//parse control data
		unsigned long chunks = iInputData->GetChunksCount(cData);
		
		#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
		__LOG("number of chunks in data:")
		__LOGNUM(chunks)
		#endif
		
		//unpack control data payload into array of strings
		RArray<TPtrC> array;
		RPointerArray<HBufC> chunksArray;
		long i;
		for(i=0; i<chunks; i++)
			{
			unsigned long currentChunkSize=0;
			const void* payloadChunk = iInputData->GetChunk(cData, i, currentChunkSize);
			if(payloadChunk && currentChunkSize>0)
				{
				HBufC8 *des8 = HBufC8::NewLC(currentChunkSize+1);
				des8->Des().Copy((unsigned char const*)payloadChunk, currentChunkSize);
				HBufC *des = HBufC::NewLC(des8->Length());
				des->Des().Copy(*des8);
				CleanupStack::Pop();//des
				CleanupStack::PopAndDestroy();//des8
				chunksArray.AppendL(des);
				array.Append(des->Des());
				
				#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
				__LOGBUF(array[array.Count()-1])
				#endif
				}
			}
		
		User::After(5000); //small delay to let usb on pc flush all buffers
		
		//parse command and send back acknowledgment
		iCommand->HandleCommandL(array);
		
		//cleanup
		chunksArray.ResetAndDestroy();
		chunksArray.Close();
		array.Close();
		}
	
	//continue reading
	if(iContinueReading)
		this->StartReading();
	}


EXPORT_C TInt CInputFramework::StartReading()
	{
	#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
	__MARK_METHOD("CInputFramework::StartReading")
	#endif	
	
	TInt errCode = KErrNone;
	if(iInputPlugin!=NULL && !this->IsActive())
		{		
		iContinueReading = ETrue;
		iDataPtr.Zero();
		errCode = iInputPlugin->ReadData(iStatus, iDataPtr);
		if(errCode == KErrNone)			
			SetActive();
		}
	else if(!iInputPlugin)
		errCode = KErrNotReady;
	
	return errCode;
	}


EXPORT_C void CInputFramework::StopReading()
	{
	#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
	__MARK_METHOD("CInputFramework::StopReading")
	#endif	
	
	if(IsActive())
		Cancel();
	iContinueReading = EFalse;
	}


EXPORT_C TInt CInputFramework::DoCommandL(TCommand aCommand, const RArray<TPtrC>& aValues)
	{
	TInt errCode = KErrNotSupported;
	
	//create new array with TPtrC8 values
	RPointerArray<HBufC8> values8;
	//CleanupStack::PushL( TCleanupItem(ResetAndDestroyPtrArray8, &values8) );
	RArray<TPtrC8> array8;
	CleanupClosePushL(array8);
	for(TInt i=0; i<aValues.Count(); ++i)
		{
		HBufC8* des8 = HBufC8::NewLC(aValues[i].Length());
		des8->Des().Copy(aValues[i]);
		values8.AppendL(des8);
		array8.AppendL( values8[values8.Count()-1]->Des() ); //get last TPtrC8
		CleanupStack::Pop(); //des8
		}
	
	//process command in server
	ControlData* ack = iObserver->ProcessCommandL(aCommand, array8);

	//cleanup
	CleanupStack::PopAndDestroy(); //array8
	values8.ResetAndDestroy();
	values8.Close();
	
	//prepare acknowledgment
	TInt ackSize = iInputData->GetSize(ack);
	RBuf8 ackBuf;
	ackBuf.Create(ackSize);
	ackBuf.Copy((unsigned char const*)ack, ackSize);
	
	#if defined(__LIGHTLOGGER_ENABLED) && defined(__VERBOSE_MODE)
	__LOG("ack size:")
	__LOGNUM(ackSize);
	__LOG("ack")
	__LOGBUF(ackBuf)
	#endif
	
	if(iInputPlugin)
		iInputPlugin->SendAcknowledgment(ackBuf); //send back acknowledgment
	ackBuf.Close();
	delete [] ack;
	iControlData.Zero();
	iObserver->DoPostProcessing(aCommand);

	if(!iContinueReading || !iInputPlugin)
		return KErrGeneral;
		
	/*
	during 'restart' input framework will be started by observer and we don'e need to restart
	it in RunL method, this flag will prevent of calling StartReading again.
	 */
	if(aCommand == ERestart) 
		iContinueReading = EFalse;
		
	return errCode;
	}

} //</Ulogger>

