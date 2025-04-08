#pragma once

#include "CoreMinimal.h"
#include "Types/IS_SlotTypes.h"
#include "IS_InventorySlot.generated.h"

/**
 * Represents a single slot in the inventory system.
 * 
 * The `InventorySlot` class is responsible for managing the data and stack count of an item stored in it.
 * It provides methods to set and retrieve the item data and stack count, as well as to check if the slot is empty
 * or to clear its contents. 
 */
UCLASS()
class INVENTORYSYSTEM_API UIS_InventorySlot : public UObject
{
	GENERATED_BODY()

private:
	FInventorySlotData SlotData;

public:
	bool operator<(const UIS_InventorySlot*& _other) const
	{
		if (_other == nullptr) return false;
		return GetSlotData().ItemData.Name < _other->GetSlotData().ItemData.Name;
	}
	
	/**
	 * Clears the data and resets the stack count for this inventory slot.
	 */
	void ClearData();

	/**
	 * Sets the slot data for this inventory slot.
	 *
	 * @param _itemData The new item data to assign to this slot.
	 * @param _stackCount The new stack count assign to this slot.
	 */
	FORCEINLINE void SetSlotData(const FItemData& _itemData, const int32 _stackCount)
	{
		SlotData.ItemData = _itemData;
		SlotData.StackCount = _stackCount;
	}

	FORCEINLINE void SetSlotData(const FInventorySlotData& _slotData) {SlotData = _slotData;}
	

	/**
	 * Sets the item data for this inventory slot.
	 *
	 * @param _newData The new item data to assign to this slot.
	 */
	FORCEINLINE void SetItemData(const FItemData& _newData) { SlotData.ItemData = _newData; }

	/**
	 * Sets the stack count for this inventory slot.
	 *
	 * @param _newStackCount The new stack count to assign to this slot.
	 */
	FORCEINLINE void SetStackCount(const int32 _newStackCount) { SlotData.StackCount = _newStackCount; }

	/**
	 * Retrieves the slot data stored in this inventory slot.
	 *
	 * @return The slot data stored in this slot.
	 */
	FORCEINLINE FInventorySlotData GetSlotData() const { return SlotData; }

	/**
	 * Checks whether the inventory slot is empty.
	 *
	 * @return Returns true if the slot is empty, otherwise false.
	 */
	FORCEINLINE bool IsEmpty() const { return SlotData.StackCount == 0; }

	/**
	 * This is for debugging purpose only. This is meant to be removed in the futur.
	 */
	void AddToImGui(const FString& _context) const;
	
};
