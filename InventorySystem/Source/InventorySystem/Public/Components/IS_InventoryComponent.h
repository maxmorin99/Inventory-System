#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IS_InventoryComponent.generated.h"

class UIS_InventorySlot;

struct FGameplayTag;
struct FItemData;
struct FInventorySlotData;

enum class ESortType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedSlotChangedSignature, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotDataChangedSignature, const FInventorySlotData&, Data, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAddedSignature, const FGameplayTag&, ItemID, int32, Qty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDroppedSignature, const FGameplayTag&, ItemID, int32, Qty);

/**
 * A component that manages the inventory system for an actor.
 * 
 * The `UPI_InventoryComponent` class provides functionality for managing an inventory system, 
 * including adding, removing, and organizing items. It supports inventory slots, quick access 
 * slots, weight management, and item stacking. This class is designed to work as a component 
 * of an actor, such as a player character, and supports network replication for multiplayer scenarios.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORYSYSTEM_API UIS_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Delegate called when the selected slot changes. Called on client only.
	UPROPERTY(BlueprintAssignable, Category = "Inventory Delegates")
	FOnSelectedSlotChangedSignature OnSelectedSlotChangedDelegate;
	
	// Delegate called when the data of an inventory slot changes. Called on client only.
	UPROPERTY(BlueprintAssignable, Category = "Inventory Delegates")
	FOnSlotDataChangedSignature OnSlotDataChangedDelegate;

	// Delegate Called when one or multiple items are added. Called on client only.
	UPROPERTY(BlueprintAssignable, Category = "Inventory Delegates")
	FOnItemAddedSignature OnItemAddedDelegate;

	// Delegate Called when one or multiple items are dropped. Called on Server only.
	UPROPERTY(BlueprintAssignable, Category = "Inventory Delegates")
	FOnItemDroppedSignature OnItemDroppedDelegate;
	
	UIS_InventoryComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Clears the entire inventory, removing all items and resetting it to an empty state. */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Empty();

	/**
	 * Determines whether the inventory contains an item with the specified ID.
	 * Should only be called on server since the slots data are not replicated.
	 * @param _itemID The unique identifier of the item to search for, represented by a gameplay tag.
	 *
	 * @return true if the item is found in the inventory, otherwise false.
	 */
	UFUNCTION(BlueprintCallable)
	bool DoesContainItem(const FGameplayTag& _itemID) const;
	
	/**
	 * Attempts to add the specified item to the inventory.
	 *
	 * @param _itemID The unique identifier of the item to be added, represented by a gameplay tag.
	 * @param _count The number of instances of the item to add. Defaults to 1 if not specified.
	 *
	 * @return The number of items successfully added to the inventory.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TryAddItem(const FGameplayTag& _itemID, int32 _count = 1);

	/**
	 * Attempts to drop a specified quantity of the item stored in the inventory slot at the given index.
	 *
	 * @param _slotIndex The index of the inventory slot containing the item to drop.
	 * @param _quantity The number of items to drop from the slot. Defaults to 1 if not specified.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TryDropItem(const int32 _slotIndex, int32 _quantity = 1);

	/**
	 * Sorts the items in the inventory based on the specified sorting criteria.
	 *
	 * @param _sortType The type of sorting to apply, defined by the ESortType enumeration.
	 * This determines the order in which items are arranged (e.g., by name, rarity, quantity, etc.).
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Sort(const ESortType& _sortType);
	
	/** @return The max weight of the inventory.  */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetMaxWeight() const { return MaxWeight; }

	/** @return The current weight of this inventory */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetCurrentWeight() const { return CurrentWeight; }

	/** @return The size of this inventory. */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetSize() const { return InventorySize; }

	/**
	 * Sets the selected slot index.
	 *
	 * @param _slotIndex The index of the slot to select.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetSelectedSlot(int32 _slotIndex);

	/** @return The selected slot index. */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetSelectedSlot() const { return SelectedSlot; }

	/** ImGui */
	void AddToImGui(const FString& _context);
	void AddAddItemToImGui();
	void AddRemoveItemToImGui();
	void AddSortItemsToImGui();
	FString EnumToString(const ESortType& _sortType) const;
	/** ~ImGui */

protected:
	virtual void BeginPlay() override;

private:
	// The pawn that owns this component.
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;

	// Array containing all the inventory slots.
	UPROPERTY()
	TArray<UIS_InventorySlot*> Slots;

	UPROPERTY(Replicated)
	int32 SelectedSlot = 0;

	// The total number of slots available in the inventory.
	UPROPERTY(EditDefaultsOnly)
	int32 InventorySize = 5;

	// The maximum weight the inventory can hold.
	UPROPERTY(EditDefaultsOnly)
	int32 MaxWeight = 30;
	
	// The current total weight of items in the inventory.
	UPROPERTY(Replicated)
	int32 CurrentWeight = 0;

	/**
	 * Initializes the inventory system.
	 * This function validates the consistency of the inventory parameters, such as
	 * `InventorySize` and `QuickAccessSize`, ensuring they meet the required constraints.
	 * It also allocates memory for the inventory slots based on the specified size,
	 * preparing the inventory for use.
	 */
	void Initialize();

	/** @return The number of empty slots. */
	UFUNCTION(BlueprintCallable)
	int32 GetEmptySlotsCount() const;

	/** @return true if the inventory is full (CurrentWeight == MaxWeight), false otherwise. */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsFull() const {return CurrentWeight >= MaxWeight;}

	/**
	* Attempts to stack an item into an existing inventory slot.
	*
	* @param _itemData The data of the item to stack.
	* @param _count A reference to the number of items to add. This value will be updated depending on whether the item was successfully stacked or not.
	*/
	void TryStackItem(const FItemData& _itemData, int32& _count);

	/**
	* Attempts to add an item to the currently selected inventory slot.  
	* The item will be added only if the selected slot is empty or if it contains the same item and it is stackable.  
	*
	* @param _itemData The data of the item to add.  
	* @param _count A reference to the number of items to add. This value will be updated depending on whether the item was successfully added to the selected slot or not.  
	*/
	void TryAddToSelectedSlot(const FItemData& _itemData, int32& _count);

	/**
	* Attempts to add an item to the first available empty inventory slot.  
	*
	* @param _itemData The data of the item to add.  
	* @param _count A reference to the number of items to add. This value will be updated depending on whether the item was successfully added to an empty slot or not.  
	*/
	void TryAddToEmptySlot(const FItemData& _itemData, int32& _count);

	/**
	* Replaces the content of the currently selected inventory slot with the specified item data and quantity. 
	*
	* @param _itemData The data of the new item to place in the selected slot.  
	* @param _count A reference to the number of items to add. This value will be updated to reflect the actual number of items placed in the slot.  
	*/
	void ReplaceSelectedSlotContent(const FItemData& _itemData, int32& _count);

	/**
	 * Adds an item to the first available empty slot in the inventory.
	 * This function searches for the first empty inventory slot and places the specified item in it.
	 * If a stack count is provided, it determines how many items are added to the slot. 
	 * If no empty slot is available, the function does nothing.
	 *
	 * @param _itemID The unique identifier for the item being added, represented by a gameplay tag.
	 * @param _stackCount The number of items to add to the slot. Defaults to 1 if not specified.
	 *
	 * @return The index of the first empty slot found, or -1 if no slot was found.
	 */
	int32 AddItemToFirstEmptySlot(const FGameplayTag& _itemID, const int32 _stackCount = 1);
	
	/**
	 * Finds and returns the index of the inventory slot that contains an item matching the specified ID.
	 *
	 * @param _itemID The unique identifier of the item to search for, represented by a gameplay tag.
	 *
	 * @return The index of the slot containing the item, or -1 if no matching slot is found.
	 */
	int32 FindFirstSlotWithItem(const FGameplayTag& _itemID) const;

	/**
	 * Updates the stack count for a specified inventory slot.
	 *
	 * @param _slotIndex The index of the inventory slot whose stack count is to be updated.
	 * @param _stackCount The new stack count to assign to the slot. Defaults to 1 if not specified.
	 */
	void StackItem(const int32 _slotIndex, const int32 _stackCount = 1);

	/**
	 * Sorts the items in the inventory alphabetically based on their names.
	 */
	void HandleAlphabeticalSort();

	/**
	* Sorts the items in the inventory based on their weight, arranging them from lightest to heaviest.
	*/
	void HandleWeightSort();

	/**
	 * Determines the change state of a slot by comparing its old and new data.
	 * Called after sorting the inventory.
	 *
	 * @param _oldData The slot data before sorting.
	 * @param _newData The slot data after sorting.
	 */
	bool GetSlotDataChangedState(const FInventorySlotData& _oldData, const FInventorySlotData& _newData) const;

	/**
	 * Remaps inventory slots using the provided mapping of sorted slot data.
	 * Called after sorting the inventory.
	 *
	 * @param _sortedSlotsMap A sorted map of slot indices to their corresponding inventory slot data.
	 */
	void RemapSlots(const TMap<int32, FInventorySlotData>& _sortedSlotsMap);

	/**
	 * Notifies the client about changes in the slot data.
	 * Called when the data of a specific inventory slot has been updated.
	 *
	 * @param _data The updated inventory slot data.
	 * @param _slotIndex The index of the slot that was changed.
	 */
	UFUNCTION(Client, Reliable)
	void ClientOnSlotDataChanged(const FInventorySlotData& _data, int32 _slotIndex);

	/**
	 * Notifies the client about a change in the selected slot.
	 * Called when the currently selected inventory slot has been changed.
	 *
	 * @param _index The index of the newly selected slot.
	 */
	UFUNCTION(Client, Reliable)
	void ClientOnSelectedSlotChanged(int32 _index);

	/**
	 * Notifies the client that items have been added to the inventory.
	 * Called when a specific quantity of an item has been successfully added.
	 *
	 * @param _itemID The unique identifier of the item added.
	 * @param _qty The quantity of the item that was added.
	 */
	UFUNCTION(Client, Reliable)
	void ClientOnItemsAdded(const FGameplayTag& _itemID, int32 _qty);
};








