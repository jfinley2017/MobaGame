// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#pragma once

#include "CoreMinimal.h"

#include "Engine/Engine.h" // GEngine for logging macros

// Global Variables
#define NULLSTRING FString(TEXT("None"))
#define INDEVSTRING FString(TEXT("In Development"))

// Custom Collision Channels

#define ECC_Damage ECC_GameTraceChannel1
#define ECC_Sensor ECC_GameTraceChannel2
#define ECC_Projectile ECC_GameTraceChannel3


/*******************************************************************************************************************************/
/*                                                       Debugging Macros.                                                     */
/*******************************************************************************************************************************/

// Logs are split into two categories:
// Log- General logs for a category. Not much chance of spamming output, an ability activation might be here
// LogVerbose - Verbose logs for a category. High chance of this spamming the output log. Tracking a DOT tick effect would go here, or some other variable that changes often.

// The general idea of why we don't just use the verbose verbosity (like UE_LOG(LogTemp, VERBOSE, ..)) is that logs tagged as verbose in UnrealEngine do not appear in the editor.
// There are ways to make them appear, however enabling verbose enables it for EVERY log type. Some engine verbose logs really are hard on the spam.
// With Verbose logs as a separate category, it's easy to filter them and they show by default when they're needed.

// Current concerns:
// The game likes to flood the logs when a session begins, we might want to log when a session starts with Agora category (in the GameInstance class)?
// This would make it easier to grab a session when debugging.
// Could just hide startup related categories though

// Declare the log with verbose version at same time
#if defined(__clang__) //If compiling for Linux
#define DECLARE_LOG_WITH_VERBOSE(Name) \
DECLARE_LOG_CATEGORY_EXTERN(Name, Log, All) \
DECLARE_LOG_CATEGORY_EXTERN(Name##Verbose, Log, All)
#else //If compiling for Windows
#define DECLARE_LOG_WITH_VERBOSE(Name) \
DECLARE_LOG_CATEGORY_EXTERN(##Name, Log, All) \
DECLARE_LOG_CATEGORY_EXTERN(Name##Verbose, Log, All)
#endif


DECLARE_LOG_CATEGORY_EXTERN(LogAgoraStartup, Log, All)

// General log for things which don't really belong in the other more specified logs
DECLARE_LOG_WITH_VERBOSE(Agora)

// General Character Log
DECLARE_LOG_WITH_VERBOSE(AgoraCharacter)

DECLARE_LOG_WITH_VERBOSE(AgoraMode)

// General Pawn Log
DECLARE_LOG_WITH_VERBOSE(AgoraPawn)

DECLARE_LOG_WITH_VERBOSE(AgoraAI)

// Used for errors in libraries/other utility based code
DECLARE_LOG_WITH_VERBOSE(UtilityLog)

// Logs which shouldnt remain through commits. 
DECLARE_LOG_CATEGORY_EXTERN(DebugLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(DebugLogVerbose, Log, All);


// Print enum macro. !!ENUM MUST BE MARKED AS UENUM TO BE PRINTED!!
// Usage: GETENUMSTRING("YourEnumClass", <Value(can be variable)>)
#define GETENUMSTRING(InEnumType, InEnumValue) ( \
    (FindObject<UEnum>(ANY_PACKAGE, TEXT(InEnumType), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(InEnumType), true)->GetNameStringByIndex((uint8)InEnumValue) \
                                                                    : FString("Invalid - are you sure enum uses UENUM() macro?") )

// Macros from https://wiki.unrealengine.com/Log_Macro_with_Netmode_and_Colour
// Modified slightly to allow for LogCategories/Verbosity to be specified in codebase, rather than hardcoded here
// Net logging macro
#define NETMODE_WORLD (((GEngine == nullptr) || (GetWorld() == nullptr)) ? TEXT("") \
        : (GEngine->GetNetMode(GetWorld()) == NM_Client) ? TEXT("[Client] ") \
        : (GEngine->GetNetMode(GetWorld()) == NM_ListenServer) ? TEXT("[ListenServer] ") \
        : (GEngine->GetNetMode(GetWorld()) == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
        : TEXT("[Standalone] "))

#define NETMODE_WORLDCONTEXT(WorldContextObject) (((GEngine == nullptr) || (WorldContextObject->GetWorld() == nullptr)) ? TEXT("") \
        : (GEngine->GetNetMode(WorldContextObject->GetWorld()) == NM_Client) ? TEXT("[Client] ") \
        : (GEngine->GetNetMode(WorldContextObject->GetWorld()) == NM_ListenServer) ? TEXT("[ListenServer] ") \
        : (GEngine->GetNetMode(WorldContextObject->GetWorld()) == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
        : TEXT("[Standalone] "))

#if _MSC_VER
#define FUNC_NAME    *FString(__FUNCTION__)
#else // FIXME - GCC?
#define FUNC_NAME    *FString(__func__)
#endif



//////////////////////////////////////////////////////////////////////////
// Fast Debugging,
// Less verbose, less detailed, less permanent, much faster.

/**
 * Debug trace.
 * Convenience trace which is not meant to be permanent but rather be a supplement to breakpoints.
 * Avoids having to type a formal logging category + verbosity for simple "do I make it here" checks.
 */
#define TRACEDEBUG(Format, ...)\
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (Msg == "") \
    { \
        UE_LOG(DebugLog, Warning, TEXT("%s%s() : %s"),  NETMODE_WORLD, FUNC_NAME, *GetNameSafe(this));\
    } \
    else \
    { \
        UE_LOG(DebugLog, Warning, TEXT("%s%s() : %s"),  NETMODE_WORLD, FUNC_NAME, *Msg);\
    } \
}

 /**
  * Debug trace.
  * Convenience trace which is not meant to be permanent but rather be a supplement to breakpoints.
  * Avoids having to type a formal logging category + verbosity for simple "do I make it here" checks.
  * Not intended to remain through commits.
  */
#define TRACESTATICDEBUG(Format, ...)\
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (Msg == "") \
    { \
        UE_LOG(DebugLog, Warning, TEXT("%s%s() : %s"),  NETMODE_WORLD, FUNC_NAME, *GetNameSafe(this));\
    } \
    else \
    { \
        UE_LOG(DebugLog, Warning, TEXT("%s%s() : %s"),  NETMODE_WORLD, FUNC_NAME, *Msg);\
    } \
}

/**
 * Quick SCREENDEBUG macro.
 * Convenience trace which is not meant to be permanent but rather be a supplement to breakpoints.
 * Not intended to remain through commits.
 */
#define SCREENDEBUG(Format, ...)\
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (Msg == "") \
    { \
        TCHAR StdMsg[MAX_SPRINTF] = TEXT(""); \
        FCString::Sprintf(StdMsg, TEXT("%s%s() : %s"), NETMODE_WORLD, FUNC_NAME, *GetNameSafe(this)); \
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, StdMsg); \
    } \
    else \
    { \
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, Msg); \
    } \
}

//////////////////////////////////////////////////////////////////////////
// Traces
// Meant to remain through commits as a way of logging. Should be frequently used

/* General logging macro. Will attempt to grab the netmode in addition to whatever logging is provided. */
#define TRACE(LogCategory, Verbosity, Format, ...) \
{ \
    static_assert((ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask) < ELogVerbosity::NumVerbosity && ELogVerbosity::Verbosity > 0, "Verbosity must be constant and in range."); \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (Msg == "") \
    { \
        UE_LOG(LogCategory, Verbosity, TEXT("%s%s() : %s"),  NETMODE_WORLD, FUNC_NAME, *GetNameSafe(this));\
    } \
    else \
    { \
        UE_LOG(LogCategory, Verbosity, TEXT("%s%s() : %s"),  NETMODE_WORLD, FUNC_NAME, *Msg);\
    } \
}

/* Static equivalent to the above, use when you don't have a world context from which to grab the netmode */
#define TRACESTATIC(LogCategory, Verbosity, Format, ...) \
{ \
    static_assert((ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask) < ELogVerbosity::NumVerbosity && ELogVerbosity::Verbosity > 0, "Verbosity must be constant and in range."); \
    SET_WARN_COLOR(COLOR_CYAN);\
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    UE_LOG(LogCategory, Verbosity, TEXT("%s() : %s"),  FUNC_NAME, *Msg);\
    CLEAR_WARN_COLOR();\
}

//////////////////////////////////////////////////////////////////////////
// On-Screen debug drawing
// Maybe its possible to push this into a function library where colors are easier to specify, having to find GEngine everytime you wana debug is a pain in the ass

/** Prints to screen (...). DOES NOT PRINT TO LOG. */
#define SCREENDEBUG_WHITE(Duration, Format, ...) \
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (Msg == "") \
    { \
        TCHAR StdMsg[MAX_SPRINTF] = TEXT(""); \
        FCString::Sprintf(StdMsg, TEXT("%s%s() : %s"), NETMODE_WORLD, FUNC_NAME, *GetNameSafe(this)); \
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::White, StdMsg); \
    } \
    else \
    { \
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::White, Msg); \
    } \
}

/** Prints to screen (...). DOES NOT PRINT TO LOG. */
#define SCREENDEBUG_RED(Duration, Format, ...) \
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (Msg == "") \
    { \
        TCHAR StdMsg[MAX_SPRINTF] = TEXT(""); \
        FCString::Sprintf(StdMsg, TEXT("%s%s() : %s"), NETMODE_WORLD, FUNC_NAME, *GetNameSafe(this)); \
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Red, StdMsg); \
    } \
    else \
    { \
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Red, Msg); \
    } \
}

/** Prints to screen (...). DOES NOT PRINT TO LOG. */
#define SCREENDEBUG_BLUE(Duration, Format, ...) \
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (Msg == "") \
    { \
        TCHAR StdMsg[MAX_SPRINTF] = TEXT(""); \
        FCString::Sprintf(StdMsg, TEXT("%s%s() : %s"), NETMODE_WORLD, FUNC_NAME, *GetNameSafe(this)); \
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Blue, StdMsg); \
    } \
    else \
    { \
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Blue, Msg); \
    } \
}

/** Prints to screen (...). DOES NOT PRINT TO LOG. */
#define SCREENDEBUG_GREEN(Duration, Format, ...) \
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (Msg == "") \
    { \
        TCHAR StdMsg[MAX_SPRINTF] = TEXT(""); \
        FCString::Sprintf(StdMsg, TEXT("%s%s() : %s"), NETMODE_WORLD, FUNC_NAME, *GetNameSafe(this)); \
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Green, StdMsg); \
    } \
    else \
    { \
        GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Green, Msg); \
    } \
}
