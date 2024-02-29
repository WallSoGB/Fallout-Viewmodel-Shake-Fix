#include "nvse/PluginAPI.h"
#include "SafeWrite.h"
#include "GameData.hpp"

NVSEInterface* g_nvseInterface{};
IDebugLog	   gLog("logs\\ViewmodelShakeFix.log");

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Viewmodel Shake Fix";
	info->version = 120;
	return true;
}

NiUpdateData NiUpdateData::kDefaultUpdateData = NiUpdateData();
static NiPoint3 kOrgPlayerPositions[2] = {};
static NiPoint3 ZERO = { 0.0f, 0.0f, 0.0f };

static void __fastcall HandleCameraTranslate(PlayerCharacter* apPlayer) {
	NiAVObject* pPlayerNode = apPlayer->spPlayerNode;

	// Save original player position
	kOrgPlayerPositions[0] = pPlayerNode->GetLocalTranslate();
	kOrgPlayerPositions[1] = pPlayerNode->GetWorldTranslate();

	// Reset player position
	pPlayerNode->SetWorldTranslate(ZERO);
	pPlayerNode->SetLocalTranslate(ZERO);

	// Update player scenegraph
	pPlayerNode->Update(NiUpdateData::kDefaultUpdateData);

	// Set camera position
	if (apPlayer->pIronSightNode && !apPlayer->IsInReloadAnim() && !TESMain::GetVATSCameraData()->HasMode())
		TESMain::GetSingleton()->spFirstPersonCamera->SetWorldTranslate(apPlayer->pIronSightNode->GetWorldTranslate());
}

static void __fastcall RestorePlayerPosition(NiAVObject* apNode, void*, NiUpdateData& arData) {
	apNode->SetLocalTranslate(kOrgPlayerPositions[0]);
	apNode->SetWorldTranslate(kOrgPlayerPositions[1]);
	apNode->Update(arData);
}

static _declspec(naked) void CameraHookJmp(PlayerCharacter* apPlayer) {
	// Thanks lStewieAl
	_asm {
		push 0x874F66
		jmp HandleCameraTranslate
	}
}

static void InitHooks() {
	if (*(UInt32*)0x875957 != 0x001E4305) {
		return;
	}
	WriteRelJump(0x874EBF, CameraHookJmp);
	WriteRelCall(0x875956, UInt32(RestorePlayerPosition));
}

static void MessageHandler(NVSEMessagingInterface::Message* msg) {
    switch (msg->type) {
    case NVSEMessagingInterface::kMessage_DeferredInit:
		InitHooks();
        break;
    }
}

bool NVSEPlugin_Load(NVSEInterface* nvse) {
	
	if (!nvse->isEditor) {
        ((NVSEMessagingInterface*)nvse->QueryInterface(kInterface_Messaging))->RegisterListener(nvse->GetPluginHandle(), "NVSE", MessageHandler);
	}

	return true;
}