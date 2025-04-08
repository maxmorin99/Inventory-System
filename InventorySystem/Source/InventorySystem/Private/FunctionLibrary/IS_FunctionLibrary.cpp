#include "FunctionLibrary/IS_FunctionLibrary.h"

#include "Components/IS_InventoryComponent.h"
#include "Components/IS_InventorySlot.h"
#include "Interfaces/IS_InventoryInterface.h"

UIS_InventoryComponent* UIS_FunctionLibrary::GetInventoryOfActor(const AActor* _inActor)
{
	if (const IIS_InventoryInterface* InventoryInterface = Cast<IIS_InventoryInterface>(_inActor))
	{
		return InventoryInterface->GetInventoryComponent();
	}
	return nullptr;
}

void UIS_FunctionLibrary::AddToInventoryOfActor(AActor* _ownerActor, const FGameplayTag& _itemId, const int32 _qty)
{
	if (UIS_InventoryComponent* InventoryComponent = GetInventoryOfActor(_ownerActor))
	{
		InventoryComponent->TryAddItem(_itemId, _qty);
	}
}

void UIS_FunctionLibrary::RemoveFromInventoryOfActor(AActor* _ownerActor, const int32& _slotIndex, const int32& _qty)
{
	if (UIS_InventoryComponent* InventoryComponent = GetInventoryOfActor(_ownerActor))
	{
		InventoryComponent->TryDropItem(_slotIndex, _qty);
	}
}

bool UIS_FunctionLibrary::DoesInventoryContainItem(AActor* _ownerActor, const FGameplayTag& _itemId)
{
	if (const UIS_InventoryComponent* InventoryComponent = GetInventoryOfActor(_ownerActor))
	{
		return InventoryComponent->DoesContainItem(_itemId);
	}

	return false;
}

void UIS_FunctionLibrary::GetInventoryWeights(AActor* _ownerActor, int32& Out_CurrentWeight, int32& Out_MaxWeight)
{
	if (const UIS_InventoryComponent* InventoryComponent = GetInventoryOfActor(_ownerActor))
	{
		Out_CurrentWeight = InventoryComponent->GetCurrentWeight();
		Out_MaxWeight = InventoryComponent->GetMaxWeight();
	}
	else
	{
		Out_CurrentWeight = Out_MaxWeight = 0;
	}
}
