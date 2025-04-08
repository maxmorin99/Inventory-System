#pragma once

#include "CoreMinimal.h"
#include "IS_ItemTypes.h"
#include "IS_SlotTypes.Generated.h"

/**
 * This struct defines the data associated with an inventory slot.
 */
USTRUCT(BlueprintType)
struct FInventorySlotData
{
	GENERATED_BODY()

	// The data associated with the item stored in the inventory slot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemData ItemData = FItemData();

	// The current number of items stacked in the inventory slot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 0;
	
	bool IsEmpty() const {return StackCount == 0;}

	bool operator==(const FInventorySlotData& Other) const
	{
		return ItemData == Other.ItemData && StackCount == Other.StackCount;
	}
};




