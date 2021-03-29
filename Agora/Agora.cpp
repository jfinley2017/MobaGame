// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.

#include "Agora.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Agora, "Agora");

// Defines the verbose version of the log at same time
#if defined(__clang__) //If compiling for Linux
#define DEFINE_LOG_CATEGORY_WITH_VERBOSE(Name) \
DEFINE_LOG_CATEGORY(Name) \
DEFINE_LOG_CATEGORY(Name##Verbose)
#else //If compiling for Windows
#define DEFINE_LOG_CATEGORY_WITH_VERBOSE(Name) \
DEFINE_LOG_CATEGORY(##Name) \
DEFINE_LOG_CATEGORY(Name##Verbose)
#endif

DEFINE_LOG_CATEGORY(LogAgoraStartup);

DEFINE_LOG_CATEGORY_WITH_VERBOSE(Agora);
DEFINE_LOG_CATEGORY_WITH_VERBOSE(AgoraMode);
DEFINE_LOG_CATEGORY_WITH_VERBOSE(AgoraCharacter);
DEFINE_LOG_CATEGORY_WITH_VERBOSE(AgoraPawn);
DEFINE_LOG_CATEGORY_WITH_VERBOSE(AgoraAI);
DEFINE_LOG_CATEGORY_WITH_VERBOSE(UtilityLog);
DEFINE_LOG_CATEGORY_WITH_VERBOSE(DebugLog);