#pragma once

#include "CoreMinimal.h"
#include "IS_SortTypes.Generated.h"

UENUM(BlueprintType)
enum class ESortType : uint8
{
	EST_None UMETA(DisplayName = "None"),
	EST_Alphabetical UMETA(DisplayName = "Alphabetical"),
	EST_Weight UMETA(DisplayName = "Weight")
};

