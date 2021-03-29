// Copyright (C) 2019 Kevin Ossia, Joe Finley, and Nicholas Arthur. Unauthorized copying of this file via any medium is strictly prohibited.


#include "AgoraDataSingleton.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"

#include "Agora.h" // Traces
#include "AgoraUITypes.h"
#include "AgoraTypes.h" // Table row

UAgoraDataSingleton::UAgoraDataSingleton(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{

}

USoundMix* UAgoraDataSingleton::GetGlobalSoundMix()
{
	if (!GlobalSoundMix)
	{
		TRACESTATIC(Agora, Error, "No global sound mix set in UAgoraDataSingleton!");
	}

	return GlobalSoundMix;
}

UTexture2D* UAgoraDataSingleton::GetAttributeIcon(const FGameplayAttribute& Attribute)
{

    if(!AttributeDataTable) { TRACESTATIC(Agora, Fatal, "No AttributeDataTable provided") }
   
    // Try and find the row in the table
    FAgoraAttributeDisplayData* AttributeData = AttributeDataTable->FindRow<FAgoraAttributeDisplayData>(*Attribute.GetName(), "GetAttributeIcon", true);
    if (!AttributeData) { return DefaultBrokenImage; }

    // If we have a table, but the table currently doesn't have an icon set up, return the default image
    return AttributeData->Icon ? AttributeData->Icon : DefaultAttributeImage;

}

void UAgoraDataSingleton::GetAttributeDisplayName(const FGameplayAttribute& Attribute, FText& OutAttribudeDisplayName)
{
    if (!AttributeDataTable) { TRACESTATIC(Agora, Fatal, "No AttributeDataTable provided") }

    // Try and find the row in the table
    FAgoraAttributeDisplayData* AttributeData = AttributeDataTable->FindRow<FAgoraAttributeDisplayData>(*Attribute.GetName(), "GetAttributeIcon", true);
    if (!AttributeData) { OutAttribudeDisplayName = FText::FromString(NULLSTRING); }

    // If we have a table, but the table currently doesn't have an icon set up, return the default image
    OutAttribudeDisplayName = AttributeData->DisplayName.IsEmpty() ? FText::FromString(INDEVSTRING) : AttributeData->DisplayName;

}

void UAgoraDataSingleton::GetAttributeDescription(const FGameplayAttribute& Attribute, FText& OutAttributeDescription)
{
    if (!AttributeDataTable) { TRACESTATIC(Agora, Fatal, "No AttributeDataTable provided") }

    // Try and find the row in the table
    FAgoraAttributeDisplayData* AttributeData = AttributeDataTable->FindRow<FAgoraAttributeDisplayData>(*Attribute.GetName(), "GetAttributeIcon", true);
    if (!AttributeData) { OutAttributeDescription = FText::FromString(NULLSTRING); }

    // If we have a table, but the table currently doesn't have an icon set up, return the default image
    OutAttributeDescription = AttributeData->Description.IsEmpty() ? FText::FromString(INDEVSTRING) : AttributeData->Description;

}

FLinearColor UAgoraDataSingleton::GetColorFor(const FName& Element, bool EnsureValid)
{
    ensure(UIColorData);
    FUIColorData* Data = UIColorData->FindRow<FUIColorData>(Element, "ColorSearch", true);
    ensureMsgf(Data, TEXT("No UIColorData found for %s"), *Element.ToString());
    if (Data)
    {
        return Data->IdentifyingColor;
    }
    return FLinearColor::Transparent;

}

