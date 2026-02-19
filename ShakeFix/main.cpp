#include "nvse/PluginAPI.h"
#include "GameData.hpp"

static NiPoint3 kOrgPlayerPositions[2] = {};

static void __fastcall HandleCameraTranslate(PlayerCharacter* apPlayer) {
	NiAVObject* pPlayerNode = apPlayer->spPlayerNode;

	// Save original player position
	kOrgPlayerPositions[0] = pPlayerNode->GetLocalTranslate();
	kOrgPlayerPositions[1] = pPlayerNode->GetWorldTranslate();

	// Reset player 
	const NiPoint3 kZero = { 0.f, 0.f, 0.f };
	pPlayerNode->SetWorldTranslate(kZero);
	pPlayerNode->SetLocalTranslate(kZero);

	// Update player scenegraph
	NiUpdateData kUpdateData;
	pPlayerNode->Update(kUpdateData);

	// Set camera position
	if (apPlayer->pIronSightNode && !apPlayer->IsInReloadAnim() && !TESMain::GetVATSCameraData()->HasMode())
		TESMain::GetSingleton()->spFirstPersonCamera->SetWorldTranslate(apPlayer->pIronSightNode->GetWorldTranslate());
}

static void __fastcall RestorePlayerPosition(NiAVObject* apNode, void*, NiUpdateData& arData) {
	apNode->SetLocalTranslate(kOrgPlayerPositions[0]);
	apNode->SetWorldTranslate(kOrgPlayerPositions[1]);
	apNode->Update(arData);
}

static __declspec(naked) void CameraHookJmp(PlayerCharacter* apPlayer) {
	// Thanks lStewieAl
	__asm {
		push 0x874F66
		jmp HandleCameraTranslate
	}
}

static void MessageHandler(NVSEMessagingInterface::Message* msg) {
	if (msg->type == NVSEMessagingInterface::kMessage_DeferredInit) {
		if (*reinterpret_cast<uint32_t*>(0x875957) == 0x001E4305) {
			WriteRelJump(0x874EBF, CameraHookJmp);
			WriteRelCall(0x875956, RestorePlayerPosition);
		}
	}
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Viewmodel Shake Fix";
	info->version = 121;
	return !nvse->isEditor;
}


EXTERN_DLL_EXPORT bool NVSEPlugin_Load(NVSEInterface* nvse) {
	static_cast<NVSEMessagingInterface*>(nvse->QueryInterface(kInterface_Messaging))->RegisterListener(nvse->GetPluginHandle(), "NVSE", MessageHandler);
	return true;
}