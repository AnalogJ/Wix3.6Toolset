; //-------------------------------------------------------------------------------------------------
; // <copyright file="core.mc" company="Microsoft">
; //    Copyright (c) Microsoft Corporation.  All rights reserved.
; // </copyright>
; //
; // <summary>
; //    Message definitions for the Burn engine.
; // </summary>
; //-------------------------------------------------------------------------------------------------


; // header section

MessageIdTypedef=DWORD

LanguageNames=(English=0x409:MSG00409)


; // message definitions

; // MessageId=#
; // Severity=Success
; // SymbolicName=MSG_SUCCESS
; // Language=English
; // Success %1.
; // .
;
; // MessageId=#
; // Severity=Warning
; // SymbolicName=MSG_WARNING
; // Language=English
; // Warning %1.
; // .
;
; // MessageId=#
; // Severity=Error
; // SymbolicName=MSG_ERROR
; // Language=English
; // Error %1.
; // .

MessageId=1
Severity=Success
SymbolicName=MSG_BURN_INFO
Language=English
Burn v%1!hs!, Windows v%2!d!.%3!d! (Build %4!d!: Service Pack %5!d!), path: %6!ls!, cmdline: '%7!ls!'
.

MessageId=2
Severity=Warning
SymbolicName=MSG_BURN_UNKNOWN_PRIVATE_SWITCH
Language=English
Unknown burn internal command-line switch encountered: '%1!ls!'.
.

MessageId=3
Severity=Success
SymbolicName=MSG_BURN_RUN_BY_RELATED_BUNDLE
Language=English
This bundle is being run by a related bundle as type '%1!hs!'.
.

MessageId=4
Severity=Success
SymbolicName=MSG_BA_REQUESTED_RESTART
Language=English
Bootstrapper application requested restart at shutdown. Planned to restart already: %1!hs!.
.

MessageId=5
Severity=Warning
SymbolicName=MSG_RESTARTING
Language=English
Restarting computer...
=======================================
.

MessageId=6
Severity=Success
SymbolicName=MSG_BA_REQUESTED_RELOAD
Language=English
Bootstrapper application requested to be reloaded.
.

MessageId=7
Severity=Success
SymbolicName=MSG_EXITING
Language=English
Exit code: 0x%1!x!, restarting: %2!hs!
.

MessageId=51
Severity=Error
SymbolicName=MSG_FAILED_PARSE_CONDITION
Language=English
Error %1!hs!. Failed to parse condition %2!ls!. Unexpected symbol at position %3!hs!
.

MessageId=52
Severity=Success
SymbolicName=MSG_CONDITION_RESULT
Language=English
Condition '%1!ls!' evaluates to %2!hs!.
.

MessageId=53
Severity=Error
SymbolicName=MSG_FAILED_CONDITION_CHECK
Language=English
Bundle global condition check didn't succeed - aborting without loading application.
.

MessageId=54
Severity=Error
SymbolicName=MSG_PAYLOAD_FILE_NOT_PRESENT
Language=English
Failed to resolve source for file: %2!ls!, error: %1!ls!.
.

MessageId=55
Severity=Warning
SymbolicName=MSG_CANNOT_LOAD_STATE_FILE
Language=English
Could not load or read state file: %2!ls!, error: 0x%1!x!.
.

MessageId=56
Severity=Error
SymbolicName=MSG_USER_CANCELED
Language=English
Application canceled operation: %2!ls!, error: %1!ls!
.

MessageId=100
Severity=Success
SymbolicName=MSG_DETECT_BEGIN
Language=English
Detect %1!u! packages
.

MessageId=101
Severity=Success
SymbolicName=MSG_DETECTED_PACKAGE
Language=English
Detected package: %1!ls!, state: %2!hs!, cached: %3!hs!
.

MessageId=102
Severity=Success
SymbolicName=MSG_DETECTED_RELATED_BUNDLE
Language=English
Detected related bundle: %1!ls!, type: %2!hs!, scope: %3!hs!, version: %4!hs!, operation: %5!hs!
.

MessageId=103
Severity=Success
SymbolicName=MSG_DETECTED_RELATED_PACKAGE
Language=English
Detected related package: %1!ls!, scope: %2!hs!, version: %3!hs!, language: %4!u! operation: %5!hs!
.

MessageId=104
Severity=Success
SymbolicName=MSG_DETECTED_MSI_FEATURE
Language=English
Detected package: %1!ls!, feature: %2!ls!, state: %3!hs!
.

MessageId=105
Severity=Success
SymbolicName=MSG_DETECTED_MSP_TARGET
Language=English
Detected package: %1!ls! target: %2!ls!, state: %3!hs!
.

MessageId=120
Severity=Warning
SymbolicName=MSG_DETECT_PACKAGE_NOT_FULLY_CACHED
Language=English
Detected partially cached package: %1!ls!, invalid payload: %2!ls!, reason: 0x%3!x!
.

MessageId=151
Severity=Error
SymbolicName=MSG_FAILED_DETECT_PACKAGE
Language=English
Detect failed for package: %2!ls!, error: %1!ls!
.

MessageId=152
Severity=Error
SymbolicName=MSG_FAILED_READ_RELATED_PACKAGE_LANGUAGE
Language=English
Detected related package: %2!ls!, but failed to read language: %3!hs!, error: 0x%1!x!
.

MessageId=170
Severity=Warning
SymbolicName=MSG_DETECT_BAD_PRODUCT_CONFIGURATION
Language=English
Detected bad configuration for product: %1!ls!
.

MessageId=199
Severity=Success
SymbolicName=MSG_DETECT_COMPLETE
Language=English
Detect complete, result: 0x%1!x!
.

MessageId=200
Severity=Success
SymbolicName=MSG_PLAN_BEGIN
Language=English
Plan %1!u! packages, action: %2!hs!
.

MessageId=201
Severity=Success
SymbolicName=MSG_PLANNED_PACKAGE
Language=English
Planned package: %1!ls!, state: %2!hs!, default requested: %3!hs!, ba requested: %4!hs!, execute: %5!hs!, rollback: %6!hs!, cache: %7!hs!, uncache: %8!hs!, dependency: %9!hs!
.

MessageId=202
Severity=Success
SymbolicName=MSG_PLANNED_BUNDLE_UX_CHANGED_REQUEST
Language=English
Planned bundle: %1!ls!, ba requested state: %2!hs! over default: %3!hs!
.

MessageId=203
Severity=Success
SymbolicName=MSG_PLANNED_MSI_FEATURE
Language=English
Planned feature: %1!ls!, state: %2!hs!, default requested %3!hs!, ba requested: %4!hs!, execute action: %5!hs!, rollback action: %6!hs!
.

MessageId=204
Severity=Success
SymbolicName=MSG_PLAN_MSI_FEATURES
Language=English
Plan %1!u! msi features for package: %2!ls!
.

MessageId=205
Severity=Warning
SymbolicName=MSG_PLAN_SKIP_PATCH_ACTION
Language=English
Plan %5!hs! skipped patch: %1!ls!, action: %2!hs! because chained target package: %3!ls! being uninstalled
.

MessageId=206
Severity=Warning
SymbolicName=MSG_PLAN_SKIP_SLIPSTREAM_ACTION
Language=English
Plan %5!hs! skipped patch: %1!ls!, action: %2!hs! because slipstreamed into chained target package: %3!ls!, action: %4!hs!
.

MessageId=207
Severity=Success
SymbolicName=MSG_PLANNED_RELATED_BUNDLE
Language=English
Planned related bundle: %1!ls!, type: %2!hs!, default requested: %3!hs!, ba requested: %4!hs!, execute: %5!hs!, rollback: %6!hs!, dependency: %7!hs!
.

MessageId=299
Severity=Success
SymbolicName=MSG_PLAN_COMPLETE
Language=English
Plan complete, result: 0x%1!x!
.

MessageId=300
Severity=Success
SymbolicName=MSG_APPLY_BEGIN
Language=English
Apply begin
.

MessageId=301
Severity=Success
SymbolicName=MSG_APPLYING_PACKAGE
Language=English
Applying %1!hs! package: %2!ls!, action: %3!hs!, path: %4!ls!, arguments: '%5!ls!'
.

MessageId=302
Severity=Success
SymbolicName=MSG_ACQUIRED_PAYLOAD
Language=English
Acquired payload: %1!ls! to working path: %2!ls! from: %4!ls!.
.

MessageId=304
Severity=Success
SymbolicName=MSG_VERIFIED_EXISTING_PAYLOAD
Language=English
Verified existing payload: %1!ls! at path: %2!ls!.
.

MessageId=305
Severity=Success
SymbolicName=MSG_VERIFIED_ACQUIRED_PAYLOAD
Language=English
Verified acquired payload: %1!ls! at path: %2!ls!, %3!hs! to: %4!ls!.
.

MessageId=306
Severity=Success
SymbolicName=MSG_APPLYING_PATCH_PACKAGE
Language=English
Applying package: %1!ls!, target: %5!ls!, action: %2!hs!, path: %3!ls!, arguments: '%4!ls!'
.

MessageId=307
Severity=Warning
SymbolicName=MSG_ATTEMPTED_UNINSTALL_ABSENT_PACKAGE
Language=English
Attempted to uninstall absent package: %1!ls!. Continuing...
.

MessageId=308
Severity=Warning
SymbolicName=MSG_FAILED_PAUSE_AU
Language=English
Automatic updates could not be paused due to error: 0x%1!x!. Continuing...
.

MessageId=309
Severity=Warning
SymbolicName=MSG_APPLY_SKIPPED_FAILED_CACHED_PACKAGE
Language=English
Skipping apply of package: %1!ls! due to cache error: 0x%2!x!. Continuing...
.

MessageId=310
Severity=Error
SymbolicName=MSG_FAILED_VERIFY_PAYLOAD
Language=English
Failed to verify payload: %2!ls! at path: %3!ls!, error: %1!ls!. Deleting file.
.

MessageId=311
Severity=Error
SymbolicName=MSG_FAILED_ACQUIRE_CONTAINER
Language=English
Failed to acquire container: %2!ls! to working path: %3!ls!, error: %1!ls!.
.

MessageId=312
Severity=Error
SymbolicName=MSG_FAILED_EXTRACT_CONTAINER
Language=English
Failed to extract payloads from container: %2!ls! to working path: %3!ls!, error: %1!ls!.
.

MessageId=313
Severity=Error
SymbolicName=MSG_FAILED_ACQUIRE_PAYLOAD
Language=English
Failed to acquire payload: %2!ls! to working path: %3!ls!, error: %1!ls!.
.

MessageId=314
Severity=Error
SymbolicName=MSG_FAILED_CACHE_PAYLOAD
Language=English
Failed to cache payload: %2!ls! from working path: %3!ls!, error: %1!ls!.
.

MessageId=315
Severity=Error
SymbolicName=MSG_FAILED_LAYOUT_BUNDLE
Language=English
Failed to layout bundle: %2!ls! to layout directory: %3!ls!, error: %1!ls!.
.

MessageId=316
Severity=Error
SymbolicName=MSG_FAILED_LAYOUT_PAYLOAD
Language=English
Failed to layout payload: %2!ls! to layout directory: %3!ls!, error: %1!ls!.
.

MessageId=317
Severity=Success
SymbolicName=MSG_ROLLBACK_PACKAGE_SKIPPED
Language=English
Skipped rollback of package: %1!ls!, action: %2!hs!, already: %3!hs!
.

MessageId=320
Severity=Success
SymbolicName=MSG_DEPENDENCY_BUNDLE_REGISTER
Language=English
Registering bundle dependency provider: %1!ls!, version: %2!ls!
.

MessageId=321
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_SKIP_NOPROVIDERS
Language=English
Skipping dependency registration on package with no dependency providers: %1!ls!
.

MessageId=322
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_SKIP_WRONGSCOPE
Language=English
Skipping cross-scope dependency registration on package: %1!ls!, bundle scope: %2!hs!, package scope: %3!hs!
.

MessageId=323
Severity=Success
SymbolicName=MSG_DEPENDENCY_PACKAGE_REGISTER
Language=English
Registering package dependency provider: %1!ls!, version: %2!ls!, package: %3!ls!
.

MessageId=324
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_SKIP_MISSING
Language=English
Skipping dependency registration on missing package provider: %1!ls!, package: %2!ls!
.

MessageId=325
Severity=Success
SymbolicName=MSG_DEPENDENCY_PACKAGE_REGISTER_DEPENDENCY
Language=English
Registering dependency: %1!ls! on package provider: %2!ls!, package: %3!ls!
.

MessageId=326
Severity=Success
SymbolicName=MSG_DEPENDENCY_PACKAGE_UNREGISTER_DEPENDENCY
Language=English
Removing dependency: %1!ls! on package provider: %2!ls!, package %3!ls!
.

MessageId=327
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_HASDEPENDENTS
Language=English
Will not uninstall package: %1!ls!, found dependents: %2!d!
.

MessageId=328
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_DEPENDENT
Language=English
Found dependent: %1!ls!, name: %2!ls!
.

MessageId=329
Severity=Success
SymbolicName=MSG_DEPENDENCY_PACKAGE_UNREGISTER
Language=English
Removing package dependency provider: %1!ls!, package: %2!ls!
.

MessageId=330
Severity=Success
SymbolicName=MSG_DEPENDENCY_BUNDLE_UNREGISTER
Language=English
Removing bundle dependency provider: %1!ls!
.

MessageId=331
Severity=Success
SymbolicName=MSG_SYSTEM_RESTORE_POINT_STARTING
Language=English
Creating a system restore point.
.

MessageId=332
Severity=Success
SymbolicName=MSG_SYSTEM_RESTORE_POINT_SUCCEEDED
Language=English
Created a system restore point.
.

MessageId=333
Severity=Success
SymbolicName=MSG_SYSTEM_RESTORE_POINT_DISABLED
Language=English
System restore disabled, system restore point not created.
.

MessageId=334
Severity=Warning
SymbolicName=MSG_SYSTEM_RESTORE_POINT_FAILED
Language=English
Could not create system restore point, error: 0x%1!x!. Continuing...
.

MessageId=335
Severity=Success
SymbolicName=MSG_ACQUIRE_BUNDLE_PAYLOAD
Language=English
Acquiring bundle payload: %2!ls!, %3!hs! from: %4!ls!
.

MessageId=336
Severity=Success
SymbolicName=MSG_ACQUIRE_CONTAINER
Language=English
Acquiring container: %1!ls!, %3!hs! from: %4!ls!
.

MessageId=337
Severity=Success
SymbolicName=MSG_ACQUIRE_CONTAINER_PAYLOAD
Language=English
Acquiring container: %1!ls!, payload: %2!ls!, %3!hs! from: %4!ls!
.

MessageId=338
Severity=Success
SymbolicName=MSG_ACQUIRE_PACKAGE_PAYLOAD
Language=English
Acquiring package: %1!ls!, payload: %2!ls!, %3!hs! from: %4!ls!
.

MessageId=340
Severity=Warning
SymbolicName=MSG_PROMPT_BUNDLE_PAYLOAD_SOURCE
Language=English
Prompt for source of bundle payload: %2!ls!, path: %3!ls!
.

MessageId=341
Severity=Warning
SymbolicName=MSG_PROMPT_CONTAINER_SOURCE
Language=English
Prompt for source of container: %1!ls!, path: %3!ls!
.

MessageId=342
Severity=Warning
SymbolicName=MSG_PROMPT_CONTAINER_PAYLOAD_SOURCE
Language=English
Prompt for source of container: %1!ls!, payload: %2!ls!, path: %3!ls!
.

MessageId=343
Severity=Warning
SymbolicName=MSG_PROMPT_PACKAGE_PAYLOAD_SOURCE
Language=English
Prompt for source of package: %1!ls!, payload: %2!ls!, path: %3!ls!
.

MessageId=348
Severity=Warning
SymbolicName=MSG_APPLY_RETRYING_PACKAGE
Language=English
Application requested retry of package: %1!ls!, encountered error: 0x%2!x!. Retrying...
.

MessageId=349
Severity=Warning
SymbolicName=MSG_APPLY_RETRYING_PAYLOAD
Language=English
Application requested retry of payload: %2!ls!, encountered error: %1!ls!. Retrying...
.

MessageId=350
Severity=Warning
SymbolicName=MSG_APPLY_CONTINUING_NONVITAL_PACKAGE
Language=English
Applying non-vital package: %1!ls!, encountered error: 0x%2!x!. Continuing...
.

MessageId=351
Severity=Success
SymbolicName=MSG_UNCACHE_PACKAGE
Language=English
Removing cached package: %1!ls!, from path: %2!ls!
.

MessageId=352
Severity=Success
SymbolicName=MSG_UNCACHE_BUNDLE
Language=English
Removing cached bundle: %1!ls!, from path: %2!ls!
.

MessageId=353
Severity=Warning
SymbolicName=MSG_UNABLE_UNCACHE_PACKAGE
Language=English
Unable to remove cached package: %1!ls!, from path: %2!ls!, reason: 0x%3!x!. Continuing...
.

MessageId=354
Severity=Warning
SymbolicName=MSG_UNABLE_UNCACHE_BUNDLE
Language=English
Unable to remove cached bundle: %1!ls!, from path: %2!ls!, reason: 0x%3!x!. Continuing...
.

MessageId=355
Severity=Warning
SymbolicName=MSG_SOURCELIST_REGISTER
Language=English
Unable to register source directory: %1!ls!, product: %2!ls!, reason: 0x%3!x!. Continuing...
.

MessageId=399
Severity=Success
SymbolicName=MSG_APPLY_COMPLETE
Language=English
Apply complete, result: 0x%1!x!, restart: %2!hs!, ba requested restart:  %3!hs!
.

MessageId=400
Severity=Success
SymbolicName=MSG_SYSTEM_SHUTDOWN
Language=English
Received system request to shut down the process: critical: %1!hs!, elevated: %2!hs!, allowed: %3!hs!
.

MessageId=410
Severity=Success
SymbolicName=MSG_VARIABLE_DUMP
Language=English
Variable: %1!ls!
.

MessageId=500
Severity=Success
SymbolicName=MSG_QUIT
Language=English
Shutting down, exit code: 0x%1!x!
.

MessageId=501
Severity=Warning
SymbolicName=MSG_STATE_NOT_SAVED
Language=English
The state file could not be saved, error: 0x%1!x!. Continuing...
.
