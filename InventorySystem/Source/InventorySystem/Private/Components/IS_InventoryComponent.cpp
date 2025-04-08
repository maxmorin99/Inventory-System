#include "Components/IS_InventoryComponent.h"

#include "GameplayTagsManager.h"
#include "Components/IS_InventorySlot.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/IS_ItemDataManager.h"
#include "imgui.h"
#include "Types/IS_SortTypes.h"

UIS_InventoryComponent::UIS_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UIS_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UIS_InventoryComponent, SelectedSlot, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIS_InventoryComponent, CurrentWeight, COND_OwnerOnly);
}

void UIS_InventoryComponent::Empty_Implementation()
{
	for (int32 i = 0; i < InventorySize; i++)
	{
		Slots[i]->ClearData();
		ClientOnSlotDataChanged(FInventorySlotData(), i);
	}
}

void UIS_InventoryComponent::TryAddItem_Implementation(const FGameplayTag& _itemID, int32 _count)
{
	checkf(_count > 0, TEXT("You must add at least one item. Called in %s::TryAddItem"), *GetName());

	if (IsFull()) return;

	FItemData ItemData = FItemData();
	if (UIS_ItemDataManager::Get(this).GetItemData(_itemID, ItemData) == false) return;

	const int32 InitialQtyToAdd = _count;

	TryStackItem(ItemData, _count);

	if (_count > 0)
	{
		TryAddToSelectedSlot(ItemData, _count);
	}

	if (_count > 0)
	{
		TryAddToEmptySlot(ItemData, _count);
	}

	if (_count > 0)
	{
		ReplaceSelectedSlotContent(ItemData, _count);
	}

	CurrentWeight += (InitialQtyToAdd - _count) * ItemData.Weight;

	ClientOnItemsAdded(_itemID, InitialQtyToAdd - _count);
}

void UIS_InventoryComponent::TryDropItem_Implementation(const int32 _slotIndex, int32 _quantity)
{
	checkf(_slotIndex < InventorySize && _slotIndex >= 0, TEXT("Can't remove item from invalid inventory slot. Called from %s::TryDropItem"), *GetName());
	checkf(_quantity > 0, TEXT("Cant remove negative or null quantity of an item from the inventory. Called from %s::TryDropItem"), *GetName());

	UIS_InventorySlot* Slot = Slots[_slotIndex];

	if (Slot->IsEmpty()) return;

	const FInventorySlotData SlotItemData = Slot->GetSlotData();

	const int32 CurrSlotStackCnt = Slot->GetSlotData().StackCount;
	_quantity = FMath::Min(_quantity, CurrSlotStackCnt);
	const uint32 NewSlotStackCnt = CurrSlotStackCnt - _quantity;

	if (NewSlotStackCnt == 0)
	{
		Slot->ClearData();
		ClientOnSlotDataChanged(Slot->GetSlotData(), _slotIndex);
	}
	else
	{
		Slot->SetStackCount(NewSlotStackCnt);
		ClientOnSlotDataChanged(Slot->GetSlotData(), _slotIndex);
	}

	CurrentWeight -= _quantity * SlotItemData.ItemData.Weight;

	OnItemDroppedDelegate.Broadcast(SlotItemData.ItemData.ItemID, _quantity);
}

void UIS_InventoryComponent::Sort_Implementation(const ESortType& _sortType)
{
	switch (_sortType)
	{
	case ESortType::EST_None:
		break;
	case ESortType::EST_Alphabetical:
		HandleAlphabeticalSort();
		break;
	case ESortType::EST_Weight:
		HandleWeightSort();
		break;
	}
}

void UIS_InventoryComponent::SetSelectedSlot_Implementation(int32 _slotIndex)
{
	checkf(_slotIndex >= 0, TEXT("Index must be >= 0."))
	
	const int32 CurrentSelectedSlotIndex = GetSelectedSlot();
	const bool bSelectedSlotChanged = CurrentSelectedSlotIndex != _slotIndex;
	SelectedSlot = _slotIndex;
	
	if (bSelectedSlotChanged)
	{
		ClientOnSelectedSlotChanged(_slotIndex);
	}
}

int32 UIS_InventoryComponent::GetEmptySlotsCount() const
{
	int32 EmptySlotsCount = 0;
	
	for (int32 i = 0; i < InventorySize; i++)
	{
		if (Slots[i]->IsEmpty()) EmptySlotsCount++;
	}
	
	return EmptySlotsCount;
}

bool UIS_InventoryComponent::DoesContainItem(const FGameplayTag& _itemID) const
{
	for (int32 i = 0; i < InventorySize; i++)
	{
		const FInventorySlotData SlotData = Slots[i]->GetSlotData();
		const FItemData ItemData = SlotData.ItemData;
		if (ItemData.ItemID == _itemID)
		{
			return true;
		}
	}

	return false;
}

void UIS_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Initialize();
}

void UIS_InventoryComponent::Initialize()
{
	OwnerPawn = Cast<APawn>(GetOwner());
	check(OwnerPawn);

	// We want to initialize the inventory on the server.
	if (!OwnerPawn->HasAuthority()) return;

	InventorySize = InventorySize > 0 ? InventorySize : 0;
	
	CurrentWeight = 0;
	Slots.Empty();
	
	// Create all inventory slots in memory.
	for (int32 i = 0; i < InventorySize; i++)
	{
		UIS_InventorySlot* NewSlot = NewObject<UIS_InventorySlot>(this);
		Slots.Add(NewSlot);
	}
}

void UIS_InventoryComponent::TryStackItem(const FItemData& _itemData, int32& _count)
{
	if (!_itemData.bCanBeStacked) return;
	
	for (int32 i = 0; i < InventorySize; i++)
	{
		const FInventorySlotData CurrSlotData = Slots[i]->GetSlotData();
		if (CurrSlotData.ItemData.ItemID == _itemData.ItemID)
		{
			const int32 CurrStackCnt = CurrSlotData.StackCount;
			StackItem(i, _count + CurrStackCnt);
			_count = 0;
		}
	}
}

void UIS_InventoryComponent::TryAddToSelectedSlot(const FItemData& _itemData, int32& _count)
{
	if (Slots[SelectedSlot]->IsEmpty())
	{
		const int32 QtyToAdd = _itemData.bCanBeStacked ? _count : 1;
		_count -= QtyToAdd;

		Slots[SelectedSlot]->SetSlotData(_itemData, QtyToAdd);
		ClientOnSlotDataChanged(Slots[SelectedSlot]->GetSlotData(), SelectedSlot);
	}
	else
	{
		const FInventorySlotData SelectedSlotData = Slots[SelectedSlot]->GetSlotData();
		if (SelectedSlotData.ItemData.ItemID == _itemData.ItemID && _itemData.bCanBeStacked)
		{
			StackItem(SelectedSlot, _count);
			_count = 0;
		}
	}
}

void UIS_InventoryComponent::TryAddToEmptySlot(const FItemData& _itemData, int32& _count)
{
	if (GetEmptySlotsCount() == 0) return;
		
	if (_itemData.bCanBeStacked)
	{
		AddItemToFirstEmptySlot(_itemData.ItemID, _count);
		_count = 0;
	}
	else
	{
		while (GetEmptySlotsCount() > 0 && _count > 0)
		{
			AddItemToFirstEmptySlot(_itemData.ItemID, 1);
			_count--;
		}
	}
}

void UIS_InventoryComponent::ReplaceSelectedSlotContent(const FItemData& _itemData, int32& _count)
{
	checkf(_count > 0, TEXT("Can't replace SelectedSlot's content with a negative item stack value."));

	const int32 SelectedSlotOldStackCnt = Slots[SelectedSlot] ? Slots[SelectedSlot]->GetSlotData().StackCount : 0;
	
	if (SelectedSlotOldStackCnt > 0)
	{
		TryDropItem(SelectedSlot, SelectedSlotOldStackCnt);
	}

	Slots[SelectedSlot]->SetSlotData(_itemData, _count);
	ClientOnSlotDataChanged(Slots[SelectedSlot]->GetSlotData(), SelectedSlot);
	_count = 0;
}

int32 UIS_InventoryComponent::AddItemToFirstEmptySlot(const FGameplayTag& _itemID, const int32 _stackCount)
{
	FItemData ItemData = FItemData();
	if (UIS_ItemDataManager::Get(this).GetItemData(_itemID, ItemData) == false)
	{
		return -1;
	}
	
	for (int32 i = 0; i < InventorySize; i++)
	{
		UIS_InventorySlot* CurrentSlot = Slots[i];
		
		if (CurrentSlot->IsEmpty())
		{
			CurrentSlot->SetItemData(ItemData);
			CurrentSlot->SetStackCount(_stackCount);
			ClientOnSlotDataChanged(CurrentSlot->GetSlotData(), i);
	
			return i;
		}
	}

	return -1;
}

int32 UIS_InventoryComponent::FindFirstSlotWithItem(const FGameplayTag& _itemID) const
{
	for (int32 i = 0; i < InventorySize; i++)
	{
		const UIS_InventorySlot* Slot = Slots[i];
		FItemData SlotData = Slot->GetSlotData().ItemData;

		if (SlotData.ItemID == _itemID)
		{
			return i;
		}
	}

	return -1;
}

void UIS_InventoryComponent::StackItem(const int32 _slotIndex, const int32 _stackCount)
{
	UIS_InventorySlot* Slot = Slots[_slotIndex];
	if (Slot == nullptr || _stackCount < 0) return;

	Slot->SetStackCount(_stackCount);
	ClientOnSlotDataChanged(Slot->GetSlotData(), _slotIndex);
}

void UIS_InventoryComponent::HandleAlphabeticalSort()
{
	TMap<int32, FInventorySlotData> SlotsMap;
	
	for (int32 i = 0; i < InventorySize; i++)
	{
		if (Slots[i]->IsEmpty()) continue;
		SlotsMap.Add(i, Slots[i]->GetSlotData());
	}
	
	SlotsMap.ValueSort([](const FInventorySlotData& A, const FInventorySlotData& B)
	{
		return A.ItemData.Name < B.ItemData.Name;
	});
	
	for (int32 i = 0; i < InventorySize; i++)
	{
		Slots[i]->ClearData();
	}

	RemapSlots(SlotsMap);
}

void UIS_InventoryComponent::HandleWeightSort()
{
	TMap<int32, FInventorySlotData> SlotsMap;
	
	for (int32 i = 0; i < InventorySize; i++)
	{
		if (Slots[i]->IsEmpty()) continue;
		SlotsMap.Add(i, Slots[i]->GetSlotData());
	}
	
	SlotsMap.ValueSort([](const FInventorySlotData& A, const FInventorySlotData& B)
	{
		return A.ItemData.Weight < B.ItemData.Weight;
	});
	
	for (int32 i = 0; i < InventorySize; i++)
	{
		Slots[i]->ClearData();
	}

	RemapSlots(SlotsMap);
}

bool UIS_InventoryComponent::GetSlotDataChangedState(const FInventorySlotData& _oldData, const FInventorySlotData& _newData) const
{
	if (_oldData.IsEmpty() && !_newData.IsEmpty() || !_oldData.IsEmpty() && _newData.IsEmpty())
	{
		return true;
	}
	return false;
}

void UIS_InventoryComponent::RemapSlots(const TMap<int32, FInventorySlotData>& _sortedSlotsMap)
{
	// Key: Old slot index; Value: New slot index
	TMap<int32, int32> RemapIndices;
	int32 i = 0;
	
	for (const auto& Pair : _sortedSlotsMap)
	{
		const int32 Index = Pair.Key;
		const FInventorySlotData OldSlotData = Slots[i]->GetSlotData();
		const FInventorySlotData NewSlotData = Pair.Value;
		
		RemapIndices.Add(Index, i);
		Slots[i]->SetSlotData(NewSlotData);

		if (Index != i)
		{
			if (GetSlotDataChangedState(OldSlotData, NewSlotData))
			{
				ClientOnSlotDataChanged(Slots[i]->GetSlotData(), i);
			}
		}
		
		i++;
	}
}

void UIS_InventoryComponent::ClientOnSlotDataChanged_Implementation(const FInventorySlotData& _data, int32 _slotIndex)
{
	OnSlotDataChangedDelegate.Broadcast(_data, _slotIndex);
}

void UIS_InventoryComponent::ClientOnSelectedSlotChanged_Implementation(int32 _index)
{
	OnSelectedSlotChangedDelegate.Broadcast(_index);
}

void UIS_InventoryComponent::ClientOnItemsAdded_Implementation(const FGameplayTag& _itemID, int32 _qty)
{
	OnItemAddedDelegate.Broadcast(_itemID, _qty);
}

void UIS_InventoryComponent::AddToImGui(const FString& _context)
{
	ImGui::Spacing();
	ImGui::Text("Inventory of %s", TCHAR_TO_UTF8(*_context));
	ImGui::SameLine();
	ImGui::Text("\tSize: %d", GetSize());
	ImGui::SameLine();
	ImGui::Text("\tCurrent Weight: %d/%d", GetCurrentWeight(), GetMaxWeight());

	static FString PopupId = FString();
	if (ImGui::BeginCombo("##OptionsCombo", "--Select option--"))
	{
		if (ImGui::Selectable("Add Item"))
		{
			PopupId = FString("AddItemPU");
		}

		if (ImGui::Selectable("Remove Item"))
		{
			PopupId = FString("RemoveItemPU");
		}

		if (ImGui::Selectable("Sort Items"))
		{
			PopupId = FString("SortItems");
		}

		ImGui::EndCombo();
	}

	if (!PopupId.IsEmpty())
	{
		ImGui::OpenPopup(TCHAR_TO_UTF8(*PopupId));
		PopupId = FString();
	}

	if (ImGui::BeginPopup("AddItemPU"))
	{
		AddAddItemToImGui();
		ImGui::EndPopup();
	}
	
	if (ImGui::BeginPopup("RemoveItemPU"))
	{
		AddRemoveItemToImGui();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("SortItems"))
	{
		AddSortItemsToImGui();
		ImGui::EndPopup();
	}

	FString TableName = FString::Printf(TEXT("##%s Slot Table"), *_context);
	if (ImGui::BeginTable(TCHAR_TO_UTF8(*TableName), 4, ImGuiTableFlags_Borders))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(168, 50, 50, 255));
		ImGui::Text("Item ID");
		ImGui::TableNextColumn();
		ImGui::Text("Item Name");
		ImGui::TableNextColumn();
		ImGui::Text("Weight");
		ImGui::TableNextColumn();
		ImGui::Text("Stack Count");
		
		for (const UIS_InventorySlot* Slot : Slots)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			Slot->AddToImGui(_context);
		}

		ImGui::EndTable();
	}
}

void UIS_InventoryComponent::AddAddItemToImGui()
{
	static FString ItemID = FString("--Select Item--");
	static int32 Qty = 1;

	ImGui::Text("Item ID");
	ImGui::SameLine();

	if (ImGui::BeginCombo("##ItemIDCombo", TCHAR_TO_UTF8(*ItemID)))
	{
		if (ImGui::Selectable("Shovel"))
		{
			ItemID = TEXT("Item.Shovel");
		}
		else if (ImGui::Selectable("Flashlight"))
		{
			ItemID = TEXT("Item.Flashlight");
		}
		else if (ImGui::Selectable("Health Potion"))
		{
			ItemID = TEXT("Item.HealthPotion");
		}
		else if (ImGui::Selectable("Key"))
		{
			ItemID = TEXT("Item.Key");
		}
		else if (ImGui::Selectable("Injection"))
		{
			ItemID = TEXT("Item.Injection");
		}

		ImGui::EndCombo();
	}

	ImGui::Text("Quantity: ");
	ImGui::SameLine();
	ImGui::InputInt("", &Qty);

	if (ImGui::Button("Add"))
	{
		const FGameplayTag ItemTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(ItemID));
		ImGui::CloseCurrentPopup();
		TryAddItem(ItemTag, Qty);
		ItemID = FString("--Select Item--");
		Qty = 1;
	}
		
	if (ImGui::Button("Cancel"))
	{
		ItemID = FString("--Select Item--");
		Qty = 1;
		ImGui::CloseCurrentPopup();
	}
}

void UIS_InventoryComponent::AddRemoveItemToImGui()
{
	static int32 SlotIndex = 0;
	static int32 Qty = 1;

	ImGui::Text("Slot Index: ");
	ImGui::SameLine();
	ImGui::InputInt("##SlotIndex", &SlotIndex);
	ImGui::Text("Quantity: ");
	ImGui::SameLine();
	ImGui::InputInt("", &Qty);
	
	if (ImGui::Button("Remove"))
	{
		TryDropItem(SlotIndex, Qty);
		SlotIndex = 0;
		Qty = 1;
		ImGui::CloseCurrentPopup();
	}
	
	if (ImGui::Button("Cancel"))
	{
		SlotIndex = 0;
		Qty = 1;
		ImGui::CloseCurrentPopup();
	}
}

void UIS_InventoryComponent::AddSortItemsToImGui()
{
	ImGui::Text("Sort type: ");
	ImGui::SameLine();

	static ESortType ChosenSortType = ESortType::EST_None;
	const FString ChosenSortTypeString = EnumToString(ChosenSortType).RightChop(4);
	
	if (ImGui::BeginCombo("##SortTypeCombo", TCHAR_TO_UTF8(*ChosenSortTypeString)))
	{
		if (ImGui::Selectable("Alphabetical"))
		{
			ChosenSortType = ESortType::EST_Alphabetical;
		}
		else if (ImGui::Selectable("Weight"))
		{
			ChosenSortType = ESortType::EST_Weight;
		}

		ImGui::EndCombo();
	}

	if (ImGui::Button("Sort"))
	{
		Sort(ChosenSortType);
		ChosenSortType = ESortType::EST_None;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel"))
	{
		ChosenSortType = ESortType::EST_None;
		ImGui::CloseCurrentPopup();
	}
}

FString UIS_InventoryComponent::EnumToString(const ESortType& _sortType) const
{
	const UEnum* EnumPtr = StaticEnum<ESortType>();
	if (!EnumPtr)
	{
		return FString("Invalid");
	}

	return EnumPtr->GetNameStringByValue(static_cast<int64>(_sortType));
}