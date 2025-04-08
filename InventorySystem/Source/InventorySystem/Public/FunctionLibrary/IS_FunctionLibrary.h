// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/IS_SlotTypes.h"
#include "IS_FunctionLibrary.generated.h"

class UIS_InventoryComponent;
struct FGameplayTag;

/**
 * A Blueprint Function Library providing utility functions for managing and interacting with inventory systems.
 * This class offers a collection of static methods that can be called from Blueprints or C++ to perform operations
 * such as adding, removing, and querying items in an actor's inventory, managing quick access slots, and retrieving
 * inventory data.
 */
UCLASS()
class INVENTORYSYSTEM_API UIS_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the inventory component associated with the given actor.
	 *
	 * @param _inActor The actor from which to retrieve the inventory component.
	 *
	 * @return A pointer to the inventory component of the actor,
	 * or nullptr if the actor is not valid or does not have one.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static UIS_InventoryComponent* GetInventoryOfActor(const AActor* _inActor);

	/**
	 * Adds a specified quantity of an item to the inventory of the given actor.
	 *
	 * @param _ownerActor The actor whose inventory will receive the item.
	 * @param _itemId The gameplay tag identifying the item to add.
	 * @param _qty The quantity of the item to add. Defaults to 1 if not specified.
	 *
	 * @return The total quantity of the item successfully added to the inventory.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static void AddToInventoryOfActor(AActor* _ownerActor, const FGameplayTag& _itemId, const int32 _qty = 1);

	/**
	 * Removes a specified quantity of an item from the inventory slot at the given index for the provided actor.
	 *
	 * @param _ownerActor The actor whose inventory will be modified.
	 * @param _slotIndex The index of the inventory slot from which to remove the item.
	 * @param _qty The quantity of the item to remove. Defaults to 1 if not specified.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static void RemoveFromInventoryOfActor(AActor* _ownerActor, const int32& _slotIndex, const int32& _qty = 1);

	/**
	 * Checks if the inventory of the given actor contains an item identified by the specified gameplay tag.
	 *
	 * @param _ownerActor The actor whose inventory will be checked.
	 * @param _itemId The gameplay tag identifying the item to search for.
	 *
	 * @return True if the inventory contains the item, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static bool DoesInventoryContainItem(AActor* _ownerActor, const FGameplayTag& _itemId);

	/**
	 * Retrieves the current and maximum weight of the inventory for the given actor.
	 *
	 * @param _ownerActor The actor whose inventory weights will be retrieved.
	 * @param Out_CurrentWeight Output parameter that will contain the current weight of the inventory.
	 * @param Out_MaxWeight Output parameter that will contain the maximum weight capacity of the inventory.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static void GetInventoryWeights(AActor* _ownerActor, int32& Out_CurrentWeight, int32& Out_MaxWeight);
};
