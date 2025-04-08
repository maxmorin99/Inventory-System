#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IS_InventoryInterface.generated.h"

class UIS_InventoryComponent;

UINTERFACE(MinimalAPI)
class UIS_InventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * All actors that possess an inventory component should implement this interface.
 */
class INVENTORYSYSTEM_API IIS_InventoryInterface
{
	GENERATED_BODY()

public:
	virtual UIS_InventoryComponent* GetInventoryComponent() const = 0;
};

