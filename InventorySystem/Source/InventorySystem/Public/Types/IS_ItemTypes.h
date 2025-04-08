#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IS_ItemTypes.Generated.h"

/**
 * This struct defines all the properties related to an Item.
 */
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

	FItemData()
		:ItemID(FGameplayTag::EmptyTag)
		,Name(FString(""))
		,Description(FString(""))
		,Icon(nullptr)
		,Weight(0)
		,bCanBeStacked(false)
		,bCanBeDropped(false)
	{}

	void Empty()
	{
		ItemID = FGameplayTag::EmptyTag;
		Name = FString("");
		Description = FString("");
		Icon = nullptr;
		Weight = 0;
		bCanBeStacked = false;
		bCanBeDropped = false;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "bCanHaveMultiple"))
	bool bCanBeStacked;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanBeDropped;

	bool operator==(const FItemData& other) const
	{
		return ItemID == other.ItemID;
	}
};

