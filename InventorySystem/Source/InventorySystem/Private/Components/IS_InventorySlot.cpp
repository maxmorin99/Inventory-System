#include "Components/IS_InventorySlot.h"

#include "imgui.h"
#include "Net/UnrealNetwork.h"

void UIS_InventorySlot::ClearData()
{
	SlotData.ItemData = FItemData();
	SlotData.StackCount = 0;
}

void UIS_InventorySlot::AddToImGui(const FString& _context) const
{
	const FItemData ItemData = SlotData.ItemData;
	ImGui::Text("%s", TCHAR_TO_UTF8(*ItemData.ItemID.ToString()));
	ImGui::TableNextColumn();
	ImGui::Text("%s", TCHAR_TO_UTF8(*ItemData.Name));
	ImGui::TableNextColumn();
	ImGui::Text("%d", ItemData.Weight);
	ImGui::TableNextColumn();
	ImGui::Text("%d", SlotData.StackCount);
}
