// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited. 


#include "AgoraOptionTypes.h"

float FSoundOptions::GetChannelVolume(ESoundChannel Channel)
{
	switch (Channel)
	{
	case ESoundChannel::Master:
		return MasterVolume;
		break;
	case ESoundChannel::Effects:
		return EffectsVolume;
		break;
	case ESoundChannel::Music:
		return MusicVolume;
		break;
	}

	return -1.f;
}

void FSoundOptions::SetChannelVolume(ESoundChannel Channel, float Volume)
{
	switch (Channel)
	{
	case ESoundChannel::Master:
		MasterVolume = Volume;
		break;
	case ESoundChannel::Effects:
		EffectsVolume = Volume;
		break;
	case ESoundChannel::Music:
		MusicVolume = Volume;
		break;
	}
}
