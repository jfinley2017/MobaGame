// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraGameplayCueManager.h"

bool UAgoraGameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
#if WITH_EDITOR
	// Prevents long hitch on first begin play in editor while developing
    return false;
#endif

	// Load everything upfront for actual game
	return true;
}
