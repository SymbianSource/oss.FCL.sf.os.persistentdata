// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
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




// INCLUDE FILES
#include <ecom/ecom.h>
#include <e32uid.h>
#include "featmgrconfiguration.h"
#include "featmgrserver.h"
#include "featmgrsession.h"
#include "featmgrpluginhandler.h"
#include "featmgrdebug.h"
#include "featmgrsecuritypolicy.h"
#include <f32file.h>
#include <s32file.h>

// LOCAL CONSTANTS AND MACROS
#ifdef EXTENDED_FEATURE_MANAGER_TEST
_LIT( KPanicCategory, "EnhancedFeatMgrServer" );
#else
_LIT( KPanicCategory, "FeatMgrServer" );
#endif // EXTENDED_FEATURE_MANAGER_TEST

// LOCAL FUNCTION PROTOTYPES
TInt E32Main(); // Process entry point

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFeatMgrServer::CFeatMgrServer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFeatMgrServer::CFeatMgrServer( const TInt aPriority, const TServerType aType  )
    :
    CPolicyServer( aPriority, KFeatMgrPlatSecPolicy, aType ), iBurState(this),
    iBackupInProgress(EFalse),
    iPluginsReady(EFalse), 
    iPluginsDeleted( EFalse ),
    iPendingRequests( ETrue ),
    iFeaturesReady(EFalse)
    
    {
    // Nothing to do
    }

// -----------------------------------------------------------------------------
// CFeatMgrServer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeatMgrServer::ConstructL()
    {
    FUNC_LOG
    
#ifndef EXTENDED_FEATURE_MANAGER_TEST
    // Set server process system critical
    User::SetProcessCritical(User::ESystemCritical);
#else
    iShutdown.ConstructL();
    // ensure the server still exits even if the 1st client fails to connect
    if( !iShutdown.IsActive() )
        {
        iShutdown.Start();
        }
#endif
    
    // Add server to active scheduler
    StartL( KServerProcessName );
    
    // Connect to file server.
    User::LeaveIfError( iFs.Connect() );
    
    // register BUR
    iBackupNotification = CBaBackupSessionWrapper::NewL();
    
    // register the file 
	TFileName temp( iRegistry->GetFeaturesFilePathAndName() );
    iBackupNotification->RegisterFileL( temp, *this );
	
    // feature registry
    iRegistry = CFeatMgrFeatureRegistry::NewL( iFs, *this );
    
    TRAPD(err,iRegistry->ReadFeatureFilesL());
    switch (err)
    	{
    	case KErrNotFound:
    		ERROR_LOG( "CFeatMgrServer::ConstructL() - no feature files found in ROM - going to panic");
    		User::Panic( KPanicCategory, EPanicNoFeatureFiles );
    		break;
    	case KErrCorrupt:
    	    ERROR_LOG( "CFeatMgrServer::ConstructL() - feature information in ROM is invalid - going to panic");
    	    User::Panic( KPanicCategory, EPanicInvalidFeatureInfo );
    	    break;
    	default:
    		User::LeaveIfError(err);
    		break;
    	}
	
    // List and load plugins and feature info.
    TBool found = LoadPluginsL();
    
    if( found )
        {
        // Timer for deleting plugins and for comparing feature lists.
        iTimer = CFeatMgrTimer::NewL( *this );
       }
    else
        {
        iRegistry->ReadRuntimeFeaturesL( iFeaturesReady );
        }
    }

// -----------------------------------------------------------------------------
// CFeatMgrServer::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeatMgrServer* CFeatMgrServer::NewLC(const TInt aPriority)
    {
    FUNC_LOG

    CFeatMgrServer* self = new( ELeave ) CFeatMgrServer(aPriority, EUnsharableSessions);
    
    CleanupStack::PushL( self );
    self->ConstructL();

    return self;
    }

#ifdef EXTENDED_FEATURE_MANAGER_TEST
// -----------------------------------------------------------------------------
// A new session is being created
// Cancel the shutdown timer if it was running
// -----------------------------------------------------------------------------
//
void CFeatMgrServer::AddSession()
    {
    FUNC_LOG
    
    ++iSessionCount;
    iShutdown.Cancel();
    }

// -----------------------------------------------------------------------------
// A session is being destroyed
// Start the shutdown timer if it is the last session.
// -----------------------------------------------------------------------------
//
void CFeatMgrServer::DropSession()
    {
    FUNC_LOG
    
    if (--iSessionCount==0)
        {
        if( !iShutdown.IsActive() )
            {
            iShutdown.Start();
            }
        }
    }

// ---------------------------------------------------------------------------
// CShutDown
// ---------------------------------------------------------------------------
inline CShutdown::CShutdown()
    :CTimer(-1)
    {
    CActiveScheduler::Add(this);
    }
    
inline void CShutdown::ConstructL()
    {
    CTimer::ConstructL();
    }

inline void CShutdown::Start()
    {
    FUNC_LOG
    After(KShutdownTimeout);
    }

void CShutdown::RunL()
    {
    FUNC_LOG
    CActiveScheduler::Stop();
    }

#endif // EXTENDED_FEATURE_MANAGER_TEST
    
// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CFeatMgrServer::~CFeatMgrServer()
    {
    FUNC_LOG

    // Close all open sessions
	CFeatMgrSession* session=NULL;
	iSessionIter.SetToFirst();
	
	while( (session = static_cast<CFeatMgrSession*>(iSessionIter++)) != NULL )
	    {
		delete session;
	    }

    // Delete plugin handlers    
    if ( !iPluginsDeleted )
        {
        iPendingRequests = EFalse; // No need to serve pending requests
        iFeaturesReady = ETrue; // No need to read files anymore
        DeletePlugins();
        } 
    
    delete iTimer;
    iFs.Close();
    iPluginList.Close();
    
    // De register Backup and Restore and cleanup memory
    if( iBackupNotification ) 
    	{
		TFileName temp( iRegistry->GetFeaturesFilePathAndName() );
    	iBackupNotification->DeregisterFile( temp );
    	delete iBackupNotification;
    	iBackupNotification = NULL;
    	}
    
    delete iRegistry;
    }


// -----------------------------------------------------------------------------
// CFeatMgrServer::NewSessionL
// Creates a new CSession2
// -----------------------------------------------------------------------------
//
CSession2* CFeatMgrServer::NewSessionL( const TVersion& aVersion,
                                     const RMessage2& /*aMessage*/ ) const
    {
    FUNC_LOG
    
    if ( !User::QueryVersionSupported( TVersion( KServerVersionMajor, 
                                                 KServerVersionMinor, 
                                                 KServerVersionBuild ), 
                                                 aVersion ) )
        {
        User::Leave( KErrNotSupported );
        }
    
    CSession2* session = CFeatMgrSession::NewL(*const_cast<CFeatMgrServer*>(this), *iRegistry );

    return( session );
    }

// -----------------------------------------------------------------------------
// CFeatMgrServer::TimerFired
// Handles timer firing. 
// -----------------------------------------------------------------------------
//
void CFeatMgrServer::TimerFired()
    {
    FUNC_LOG

    // Delete plugin handlers. Plugin handler deletes the plugin.    
    if ( !iPluginsDeleted )
        {
        DeletePlugins();
        } 
    }

// -----------------------------------------------------------------------------
// CFeatMgrServer::HandleFeatureChange()
// -----------------------------------------------------------------------------
//
void CFeatMgrServer::HandleFeatureChange( TFeatureServerEntry& aFeature, TFeatureChangeType aType )
    {
    FUNC_LOG

    CFeatMgrSession* session=NULL;
	TDblQueIter<CSession2> local_iSessionIter = iSessionIter;
	local_iSessionIter.SetToFirst();
	    
	while( (session = static_cast<CFeatMgrSession*>(local_iSessionIter++)) != NULL )
	    {
		session->ServiceNotifications( aFeature, aType );
	    }
    }

// ---------------------------------------------------------------------------
// CFeatMgrServer::ResetAndDestroyArray
// ---------------------------------------------------------------------------
// 
static void ResetAndDestroyArray( TAny* aPtr )
    {
    RImplInfoPtrArray* array = static_cast< RImplInfoPtrArray* >( aPtr );
    array->ResetAndDestroy();
    array->Close();
    }

// -----------------------------------------------------------------------------
// CFeatMgrServer::LoadPluginsL()
// -----------------------------------------------------------------------------
//
TBool CFeatMgrServer::LoadPluginsL()
    {
    FUNC_LOG
    
    RImplInfoPtrArray implInfoArray;
    TBool ret( EFalse );

    // Use ECom to read information about existing interface implementations
    _LIT8( KEmptyString, "" );
    TEComResolverParams resolverParams;
    resolverParams.SetDataType (KEmptyString);
    resolverParams.SetWildcardMatch (ETrue);

    TCleanupItem cleanupItem( ResetAndDestroyArray, &implInfoArray );
    CleanupStack::PushL( cleanupItem );
    
    TIMESTAMP( "CFeatMgrServer::LoadPluginsL - ListImplementationsL start: " )
    REComSession::ListImplementationsL( KFeatureInfoPluginInterfaceUid,
#ifndef EXTENDED_FEATURE_MANAGER_TEST                                         
                                        resolverParams,

                                        KRomOnlyResolverUid, 
#endif                                        
                                        implInfoArray);
    TIMESTAMP( "CFeatMgrServer::LoadPluginsL - ListImplementationsL end: " )
    
    // Check if any plugin was found. 
    TInt count = implInfoArray.Count();
    
    if ( count > 0 )
        {
        
        for(TInt i = 0; i < count; i ++)
            {
            CFeatMgrPluginHandler* pluginHandler = NULL;
            TInt err( KErrNone );
        
            // Create handler
            TRAP( err, pluginHandler = CFeatMgrPluginHandler::NewL( 
                  implInfoArray[i]->ImplementationUid(), *this) );
                
            LOG_IF_ERROR1( err, "CFeatMgrServer::LoadPluginsL() - pluginHandler creation err %d", err );
            
            // Apply first request for plugin to process
            if ( err == KErrNone )
                {
                TRAP( err, pluginHandler->SendCommandL( 
                    FeatureInfoCommand::ELoadFeatureInfoCmdId ) );
                }            
            
            // Panic if error sth else than not supported        
            if ( err != KErrNone && err != KErrNotSupported )
                {
                ERROR_LOG2( "CFeatMgrServer::LoadPluginsL() - implementationUid: 0x%x, error: %d - going to panic",
                                        implInfoArray[i]->ImplementationUid(), err );
                User::Panic( KPanicCategory, EPanicLoadPluginError );                           
                }
            // If simple features are not supported by the plugin search for enhanced ones
            else if ( err == KErrNotSupported )
            	{
            	// Reset error code
            	err = KErrNone;
                TRAP( err, pluginHandler->SendCommandL(
                    FeatureInfoCommand::ELoadEnhancedFeatureInfoCmdId ) );

                // Panic if error since in this case the plugin does not support any feature
            	if ( err != KErrNone )
	            	{
	                ERROR_LOG2( "CFeatMgrServer::LoadPluginsL() - implementationUid: 0x%x, error: %d - going to panic",
	                        implInfoArray[i]->ImplementationUid(), err );
	                User::Panic( KPanicCategory, EPanicLoadPluginError );
	            	}
	            }
            // If a simple or enhanced feature is supported by the plugin then append the plugin to the list
            if ( err == KErrNone )
            	{
                INFO_LOG1( "CFeatMgrServer::LoadPluginsL() - Add info of implementationUid: 0x%x",
                                        implInfoArray[i]->ImplementationUid() );
            
                // Add information of the plugin to the plugin list
                SFeatMgrPluginInfo plugin;
                plugin.iPluginHandler = pluginHandler;
                // Set all plugins as not ready initially
                plugin.iPluginReady = EFalse;

                TInt err = iPluginList.Append(plugin);
            
                if ( err != KErrNone )
                    {
                    ERROR_LOG2( "CFeatMgrServer::LoadPluginsL() - Saving plugin info of implementationUid: 0x%x, err %d",
                                                  implInfoArray[i]->ImplementationUid(), err );
                    User::Leave( err );
                    }
                }
            }
        INFO_LOG1( "CFeatMgrServer::LoadPluginsL - interfaceUid.iUid == 0x%x, return plugins found",
                           KFeatureInfoPluginInterfaceUid );
        ret = ETrue;
        }
    
    else
        {
        iPluginsReady = ETrue; // Plugins not found.
        INFO_LOG1( "CFeatMgrServer::LoadPluginsL - interfaceUid.iUid == 0x%x, return plugins not found",
                           KFeatureInfoPluginInterfaceUid );
        ret = EFalse;
        }
    
    CleanupStack::PopAndDestroy(&implInfoArray);
    
    return ret;
    }
    
    
// -----------------------------------------------------------------------------
// CFeatMgrServer::PluginsReady()
// -----------------------------------------------------------------------------

TBool CFeatMgrServer::PluginsReady() const
    {
    FUNC_LOG
    return(iPluginsReady);
   	}

// -----------------------------------------------------------------------------
// CFeatMgrServer::BackupIsInProgress()
// -----------------------------------------------------------------------------
TBool CFeatMgrServer::BackupIsInProgress() const
    {
    FUNC_LOG
    return(iBackupInProgress);
    }
   	
// -----------------------------------------------------------------------------
// CFeatMgrServer::ServicePendingRequests()
// -----------------------------------------------------------------------------

void CFeatMgrServer::ServicePendingRequests()
    {
    FUNC_LOG
    
    CFeatMgrSession* session=NULL;
	iSessionIter.SetToFirst();
	    
	while( (session = static_cast<CFeatMgrSession*>(iSessionIter++)) != NULL )
	    {
		TRAPD( err, session->ServicePendingRequestsL() );
        LOG_IF_ERROR1( err, "CFeatMgrServer::ServicePendingRequests - err %d", err );
	    }
   	
   	iPendingRequests = EFalse;
   	}

// -----------------------------------------------------------------------------
// CFeatMgrServer::DeletePlugins()
// -----------------------------------------------------------------------------

void CFeatMgrServer::DeletePlugins()
    {
    FUNC_LOG
    
    // Set iPluginsReady to ETrue because plugins will be deleted.
    iPluginsReady = ETrue;
    TInt err( KErrNone );

    // Load runtimefeatures.txt if not loaded yet.
    if ( !iFeaturesReady )
        { 
        TRAP( err, iRegistry->ReadRuntimeFeaturesL( iFeaturesReady ) );
        
        LOG_IF_ERROR1( err, "CFeatMgrServer::DeletePlugins() - ReadRuntimeFeaturesL err %d", err ); 
        }
    
    // Service pending requests before deleting plugins if not serviced yet.
    if ( iPendingRequests )
        {
        ServicePendingRequests();
        }
          
    // Delete plugin handlers
    TInt count = iPluginList.Count();

    for ( TInt i = 0 ; i < count; i++ )
        {
        delete iPluginList[i].iPluginHandler;
        iPluginList[i].iPluginHandler = NULL;
        }

    // Reset list of plugins
    iPluginList.Reset();
        
    // Signal final closure of ecom session
	REComSession::FinalClose();
	    
	// All plugin handlers deleted
	iPluginsDeleted = ETrue;
	}
    
// -----------------------------------------------------------------------------
// CFeatMgrServer::FeatureInfoL()
// -----------------------------------------------------------------------------

void CFeatMgrServer::FeatureInfoL( RArray<FeatureInfoCommand::TFeature>& aFeatureList, 
        CFeatMgrPluginHandler* aPluginHandler )
    {
    FUNC_LOG
     
    // Save feature info received from the plugin
    TInt pluginCount = iPluginList.Count();

    for ( TInt i = 0; i < pluginCount; i ++ )
        {
        if ( iPluginList[i].iPluginHandler == aPluginHandler )
            {
            iRegistry->MergePluginFeatures( aFeatureList );
            // Send command to load enhanced feature info
            TRAPD( err, iPluginList[i].iPluginHandler->SendCommandL( 
                FeatureInfoCommand::ELoadEnhancedFeatureInfoCmdId ) );
            // Panic if error sth else than not supported
            if ( err != KErrNone && err != KErrNotSupported )
                {
                ERROR_LOG1( "CFeatMgrServer::FeatureInfoL() - panicing due error %d", err );
                User::Panic( KPanicCategory, EPanicLoadPluginError );
                }
            // At this point we have simple feature supported by the plugin.
            // If no enhanced feature is supported, but a simple one is, then
            // the plugin is ready to be used. Otherwise continue to add the
            // enhanced features.
            else if( err == KErrNotSupported )
            	{
            	iPluginList[i].iPluginReady = ETrue;
                }
            }
        else
            {
            INFO_LOG( "CFeatMgrServer::FeatureInfo - unknown plugin handler") ;
            }
        }   

    CheckPluginsReadyL();
    }

// -----------------------------------------------------------------------------
// CFeatMgrServer::FeatureInfoL()
// -----------------------------------------------------------------------------

void CFeatMgrServer::FeatureInfoL( RFeatureArray& aFeatureList, 
        CFeatMgrPluginHandler* aPluginHandler )
    {
    FUNC_LOG
     
    // Save feature info received from the plugin
    TInt pluginCount = iPluginList.Count();

    for ( TInt i = 0; i < pluginCount; i ++ )
        {
        if ( iPluginList[i].iPluginHandler == aPluginHandler )
            {
            iRegistry->MergePluginFeatures( aFeatureList );
            // Send another command if something left to process
            iPluginList[i].iPluginReady = ETrue;
            }
        else
            {
            INFO_LOG( "CFeatMgrServer::FeatureInfo - unknown plugin handler") ;
            }
        }   

    CheckPluginsReadyL();
    }

// -----------------------------------------------------------------------------
// CFeatMgrServer::CheckPluginsReadyL()
// -----------------------------------------------------------------------------

void CFeatMgrServer::CheckPluginsReadyL()
    {
    FUNC_LOG

    // If all plugins has responsed iPluginsReady is ETrue
    // else iPluginsReady is EFalse;
    TInt pluginCount = iPluginList.Count();
    
    for ( TInt i = 0; i < pluginCount; i ++ )
        {
        if ( iPluginList[i].iPluginReady )
            {
            iPluginsReady = ETrue;
            }
        else
            {   
            iPluginsReady = EFalse;
            break;
            }
        }
    
    // If plugins ready, read runtime features and call ServicePendingRequests of sessions.
    if ( iPluginsReady )
        {
        // Load runtimefeatures.txt
        if ( !iFeaturesReady )
            {
            iRegistry->ReadRuntimeFeaturesL( iFeaturesReady );
            }

        if ( iPendingRequests )
            {
            ServicePendingRequests();    
            }
        }      
    }
    
/**
 * Implementation of the pure virtual function.
 */
void CFeatMgrServer::HandleBackupOperationEventL( const TBackupOperationAttributes& /* aBackupOperationAttributes */ )
	{
	return;
	}

/**
 * There is a flag state variable being passed into Feature Manager as type MBackupObserver::TFileLockFlags.
 * Only certain combinations of these flags are valid for Feature Manager and if the combination is 
 * not valid, then because there is no way of returning an error it will just set the internal state 
 * machine into an "error" state.
 * Given our current state and our goto state (i.e. where we are at the moment and where we want to goto)
 * the state machine checks that this is a valid path in our state machine and then perform the correct 
 * operations to get us to the next valid state. It also calls the correct Feature manager functions to 
 * accomplish this operation.
 */
void CFeatMgrServer::ChangeFileLockL( const TDesC& /* aFileName */, TFileLockFlags aFlags )
	{
	// There is only one file we are concerned with, so
	// don't check the filename matches
	// Default TOperationType type to ENone (we are ignoring this value)
	TBackupOperationAttributes attribs( aFlags, ENone );
	iBurState.BUROperationL( attribs );

	return;
	}


/**
 * Generic return function, to return the state of the machine back to normal.
 * This can only occur from certain states.
 */
BURStatus CFeatMgrServer::Goto_NormalState( BURStatus /* aCurrent */  )
	{
	return EFeatMgrBURState_None;   // all roads lead to normal state
	}

/**
 * This function handles error recovery from an invalid state. It will return the feature 
 * maneger back to normal, in both it's internal state machine state, and it's internal 
 * variables.
 */
BURStatus CFeatMgrServer::Goto_ErrorState( BURStatus aCurrent  )
	{
	BURStatus aNewState = EFeatMgrBURState_None;   // fail safe: all roads lead to normal state
	TInt err( KErrNone );
	
	switch( aCurrent )
		{
		case( EFeatMgrBURState_BackupStarted ) :
			iBackupInProgress = EFalse;
			ServicePendingRequests();
			break;
		case( EFeatMgrBURState_RestoreStarted ) :
		    iPluginsReady    = EFalse;
		    iPluginsDeleted  = EFalse;
		    iPendingRequests = ETrue;
		    iFeaturesReady   = EFalse;
			// re-read the new features;
			ClearFeatures();
			TRAP( err, LoadFeaturesL()  );
			// Stop queuing
			iPluginsReady = ETrue;
			// commit the pending change requests
			ServicePendingRequests();
			break;
		case( EFeatMgrBURState_BackupEnded ) :
		case( EFeatMgrBURState_RestoreEnded ) :
			break;
		default :
			break;
		}
	
	aNewState = EFeatMgrBURState_Error;  // state++
	return aNewState;
	}

BURStatus CFeatMgrServer::Goto_StartBackupState( BURStatus /* aCurrent */ )
	{
	BURStatus aNewState = EFeatMgrBURState_BackupStarted;  // state++
	iBackupInProgress = ETrue;
	return aNewState;
	}

BURStatus CFeatMgrServer::Goto_EndBackupState( BURStatus /* aCurrent */  )
	{
	BURStatus aNewState = EFeatMgrBURState_BackupEnded;   // state++
	
	iBackupInProgress = EFalse;
	ServicePendingRequests();
	// no error from ServicePendingRequests() is possible
	
	return aNewState;
	}

BURStatus CFeatMgrServer::Goto_StartRestoreState( BURStatus /* aCurrent */  )
	{
	// remarkably like Goto_StartBackupState
	BURStatus aNewState = EFeatMgrBURState_RestoreStarted;  // state++
	iPluginsReady = EFalse;
	return aNewState;
	}

BURStatus CFeatMgrServer::Goto_EndRestoreState( BURStatus /* aCurrent */  )
	{
	BURStatus aNewState = EFeatMgrBURState_Error;   // fail safe
	TInt err( KErrNone );
	iPluginsReady    = EFalse;
    iPluginsDeleted  = EFalse;
    iPendingRequests = ETrue;
    iFeaturesReady   = EFalse;

	// re-read the new features
    // Only call the next function if the previous one was
    // successfully completed.
	ClearFeatures();
	TRAP( err, LoadFeaturesL()  );
	if( err == KErrNone ) 
		{
		TRAP( err, HandleRestoredNotificationsL() );
		}

	if( err != KErrNone )
		{
		// error condition
		aNewState = EFeatMgrBURState_Error;
		}
	else
		{
		// Stop queuing
		iPluginsReady = ETrue;
		
		// commit the pending change requests
		ServicePendingRequests();
		
		// Change state machine
		aNewState = EFeatMgrBURState_RestoreEnded;
		}
	
	return aNewState;
	}

/**
 * This function will load features from files and plugins 
 */
void CFeatMgrServer::LoadFeaturesL( void )
	{
    // Load ROM (Z)
	TRAPD( err, iRegistry->ReadFeatureFilesL() );
	if( KErrNotFound == err)
		{
		ERROR_LOG( "CFeatMgrServer::ConstructL() & CallReadFeatureFilesL() - no feature files found in ROM - going to panic");
	    User::Panic( KPanicCategory, EPanicNoFeatureFiles );
		}
	else
		{
		User::LeaveIfError(err);
		}

    // Load RAM (C)
	// List and load plugins and feature info.
	TBool found = LoadPluginsL();
	
	if( found )
	    {
	    // Timer for deleting plugins and for comparing feature lists.
	    iTimer = CFeatMgrTimer::NewL( *this );
	    }
	else
	    {
	    TRAPD( err, iRegistry->ReadRuntimeFeaturesL( iFeaturesReady ) );
		if( KErrNotFound == err)
			{
		    User::Panic( KPanicCategory, EPanicNoFeatureFiles );
			}
		else
			{
			User::LeaveIfError(err);
			}
	    }

	return;
}


/**
 * Discover which features have changes wrt the restore operation
 */
void CFeatMgrServer::HandleRestoredNotificationsL( void )
	{
	iRegistry->HandleRestoredFeatureNotificationsL();
	
	return;
	}

/**
 * This function will clear features from RAM
 */
void CFeatMgrServer::ClearFeatures( void )
	{
	// Delete plugin handlers    
    if ( !iPluginsDeleted )
        {
        // Delete plugin handlers
        TInt count = iPluginList.Count();
        for ( TInt i = 0 ; i < count; i++ )
            {
            delete iPluginList[i].iPluginHandler;
            iPluginList[i].iPluginHandler = NULL;
            }

        // Reset list of plugins
        iPluginList.Reset();
        iPluginList.Close();        
    	    
    	// All plugin handlers deleted
    	iPluginsDeleted = EFalse;
        } 

    if( NULL != iRegistry )
    	{
        iRegistry->ResetFeatures();
        }

    return;
}


// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// Function that starts the FeatMgrServer.
// -----------------------------------------------------------------------------
//
static void RunServerL()
    {
    FUNC_LOG

    // Naming the server thread after the startup helps to debug panics
    User::LeaveIfError( User::RenameProcess( KServerProcessName ) );
    
    User::LeaveIfError( User::RenameThread( KServerProcessName ) );
     
    // Create and install the active scheduler we need
    CActiveScheduler* scheduler = new(ELeave) CActiveScheduler;
    CleanupStack::PushL( scheduler );
    
    CActiveScheduler::Install( scheduler );
    
    // Now we are ready to instantiate the actual CServer2 instance
    CFeatMgrServer* server = CFeatMgrServer::NewLC( KServerCActivePriority );

	// Initialisation complete, now signal the client
	RProcess::Rendezvous(KErrNone);

    INFO_LOG( "RunServerL() - Starting scheduler..." );

	// Ready to run
	CActiveScheduler::Start();

    INFO_LOG( "RunServerL() - Scheduler stopped" );

	// Cleanup the server and scheduler
	CleanupStack::PopAndDestroy( server );
	CleanupStack::PopAndDestroy( scheduler );
    }

// -----------------------------------------------------------------------------
// Main function
// -----------------------------------------------------------------------------
//
TInt E32Main()
    {
    FUNC_LOG

	__UHEAP_MARK;

	CTrapCleanup* cleanup = CTrapCleanup::New();
	TInt ret = KErrNoMemory;

	if ( cleanup )
		{
		TRAP( ret, RunServerL() );
		delete cleanup;
		}

	__UHEAP_MARKEND;

	return ret;
    }

//  End of File  
