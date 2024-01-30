#include "nvse/PluginAPI.h"
#include "SafeWrite.h"
#include "GameData.hpp"

NVSEInterface* g_nvseInterface{};
IDebugLog	   gLog("logs\\ViewmodelShakeFix.log");

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Viewmodel Shake Fix";
	info->version = 110;
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

static void __cdecl ResetColorWriteEnable(BOOL abValue) {
	// Restore player position
	NiAVObject* pPlayerNode = PlayerCharacter::GetSingleton()->spPlayerNode;
	pPlayerNode->SetLocalTranslate(kOrgPlayerPositions[0]);
	pPlayerNode->SetWorldTranslate(kOrgPlayerPositions[1]);

	CdeclCall(0x4ECED0, abValue);
}

static _declspec(naked) void CameraHookJmp(PlayerCharacter* apPlayer) {
	// Thanks lStewieAl
	_asm {
		push 0x874F66
		jmp HandleCameraTranslate
	}
}

static void InitHooks() {
	if (*(UInt32*)0x8755FF != 0xFFC778CD) {
		return;
	}
	WriteRelJump(0x874EBF, CameraHookJmp);
	ReplaceCall(0x8755FE, ResetColorWriteEnable);
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