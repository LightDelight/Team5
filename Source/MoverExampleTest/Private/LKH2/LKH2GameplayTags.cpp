#include "LKH2/LKH2GameplayTags.h"

// ─── [Internal Native Tags] ───
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeStat_CurrentProgress, "Stat.Common.CurrentProgress", "Common progress stat stored on items.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeStat_MaxProgress, "Stat.Common.MaxProgress", "Common max progress stat stored on items.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeTime_StartTime, "Time.Common.StartTime", "Start timestamp for UI progress.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeTime_EndTime, "Time.Common.EndTime", "End timestamp for UI progress.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeType_Container, "Type.Item.Container", "Tag indicating that an item acts as a container for other items.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeIntent_ItemAdd, "Intent.Workstation.ItemAdd", "Intent issued when adding an item to a workstation.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeIntent_ItemRemove, "Intent.Workstation.ItemRemove", "Intent issued when removing an item from a workstation.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeIntent_Cart_ItemOverlap, "Intent.Cart.ItemOverlap", "Intent from CartPropertyComponent when an item enters the detection box.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeIntent_Cart_Overturn, "Intent.Cart.Overturn", "Intent from CartPropertyComponent when cart overturn is detected.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(NativeIntent_Cart_SpillCleanup, "Intent.Cart.SpillCleanup", "Intent sent to cleanup workstation to register a spilled item UID.");

FLKH2GameplayTags FLKH2GameplayTags::Instance;

FLKH2GameplayTags::FLKH2GameplayTags()
{
	Stat_Common_CurrentProgress = NativeStat_CurrentProgress.GetTag();
	Stat_Common_MaxProgress = NativeStat_MaxProgress.GetTag();
	Time_Common_StartTime = NativeTime_StartTime.GetTag();
	Time_Common_EndTime = NativeTime_EndTime.GetTag();
	Type_Item_Container = NativeType_Container.GetTag();
	Intent_Workstation_ItemAdd = NativeIntent_ItemAdd.GetTag();
	Intent_Workstation_ItemRemove = NativeIntent_ItemRemove.GetTag();
	Intent_Cart_ItemOverlap = NativeIntent_Cart_ItemOverlap.GetTag();
	Intent_Cart_Overturn = NativeIntent_Cart_Overturn.GetTag();
	Intent_Cart_SpillCleanup = NativeIntent_Cart_SpillCleanup.GetTag();
}

void FLKH2GameplayTags::Initialize()
{
	// NativeGameplayTags는 선언만으로 등록되지만, 구조적 명시를 위해 유지
}
