#include "Subsystems/IS_ItemDataManager.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Types/IS_ItemTypes.h"

UIS_ItemDataAsset* UPI_DeveloperSettings::GetItemDataAsset() const
{
	if (InventoryDataAsset.ToSoftObjectPath().IsValid())
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		return Cast<UIS_ItemDataAsset>(Streamable.LoadSynchronous(InventoryDataAsset.ToSoftObjectPath()));
	}

	return nullptr;
}

void UIS_ItemDataManager::Initialize(FSubsystemCollectionBase& _collection)
{
	Super::Initialize(_collection);

	if (const UPI_DeveloperSettings* DevSettings = GetDefault<UPI_DeveloperSettings>())
	{
		if (const UIS_ItemDataAsset* ItemDataAsset = DevSettings->GetItemDataAsset())
		{
			ItemsDataTable = ItemDataAsset->ItemsDataTable;
			ItemClassesDataTable = ItemDataAsset->ItemClassesDataTable;
		}
	}
}

UIS_ItemDataManager& UIS_ItemDataManager::Get(const UObject* _worldContextObject)
{
	check(_worldContextObject);
	
	const UGameInstance* GameInstance = _worldContextObject->GetWorld()->GetGameInstance();
	check(GameInstance);
	
	return *GameInstance->GetSubsystem<UIS_ItemDataManager>();
}

bool UIS_ItemDataManager::GetItemData(const FGameplayTag _itemID, FItemData& Out_Data) const
{
	if (ItemsDataTable == nullptr) return false;

	Out_Data.Empty();

	TArray<FItemData*> Rows;
	ItemsDataTable->GetAllRows("", Rows);

	for (const FItemData* Row : Rows)
	{
		if (Row->ItemID == _itemID)
		{
			Out_Data = *Row;
			return true;
		}
	}

	return false;
}
