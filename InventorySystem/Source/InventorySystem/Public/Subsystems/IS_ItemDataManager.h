#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IS_ItemDataManager.generated.h"

struct FGameplayTag;
struct FItemData;

/**
 * DataAsset class that stores all essential data related to items that can be stored in an inventory.
 */
UCLASS()
class INVENTORYSYSTEM_API UIS_ItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// DT that defines the data associated with an item tag.
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> ItemsDataTable;

	// DT that defines which classes are associated with an item.
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> ItemClassesDataTable;
};

UCLASS(Config = Game, Defaultconfig, meta = (DisplayName = "Inventory"))
class INVENTORYSYSTEM_API UPI_DeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	UIS_ItemDataAsset* GetItemDataAsset() const;

	UPROPERTY(EditAnywhere, Config, Category = "Inventory|ItemsData")
	TSoftObjectPtr<UIS_ItemDataAsset> InventoryDataAsset;
	
};

/**
* Subsystem responsible for managing and providing access to item data stored in UIS_ItemDataAsset.
* This class acts as a bridge between the inventory system and the item DataAssets, ensuring
* efficient retrieval and management of item-related information at runtime.
*/
UCLASS()
class INVENTORYSYSTEM_API UIS_ItemDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// DT that defines the data associated with an item tag.
	UPROPERTY()
	TObjectPtr<UDataTable> ItemsDataTable;
	
	// DT that defines which classes are associated with an item.
	UPROPERTY()
	TObjectPtr<UDataTable> ItemClassesDataTable;

public:
	virtual void Initialize(FSubsystemCollectionBase& _collection) override;
	
	static UIS_ItemDataManager& Get(const UObject* _worldContextObject);

	UFUNCTION(BlueprintCallable, Category="ItemManager")
	bool GetItemData(const FGameplayTag _itemID, FItemData& Out_Data) const;
};


