#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UCharacterActionGameplayAbility;
class UCharacterDataAsset;
class IAssetTypeActions;
class FAssetTypeActions_Base;

class FKrzyweKarty2EditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

protected:
    TArray<TSharedRef<IAssetTypeActions>> AssetTypeActions;
};
