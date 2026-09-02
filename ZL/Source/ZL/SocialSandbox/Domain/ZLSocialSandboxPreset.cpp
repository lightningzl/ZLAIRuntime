#include "SocialSandbox/Domain/ZLSocialSandboxPreset.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
constexpr float MaxLocationMagnitude = 100000.0f;
constexpr float MaxRotationMagnitude = 3600.0f;
constexpr float MaxInitialHealth = 1000.0f;
constexpr int32 MaxNpcCount = 4;

bool Fail(FText& OutError, const TCHAR* Message)
{
	OutError = FText::FromString(Message);
	return false;
}

bool HasOnlyFields(const TSharedPtr<FJsonObject>& Object, const TSet<FString>& AllowedFields, FText& OutError)
{
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
	{
		if (!AllowedFields.Contains(Pair.Key))
		{
			return Fail(OutError, TEXT("拒绝：配置包含不允许的字段"));
		}
	}
	return true;
}

bool ReadString(const TSharedPtr<FJsonObject>& Object, const FString& Name, FString& OutValue, const int32 MaxLength, FText& OutError)
{
	if (!Object->TryGetStringField(Name, OutValue) || OutValue.TrimStartAndEnd().IsEmpty() || OutValue.Len() > MaxLength)
	{
		return Fail(OutError, TEXT("拒绝：配置字符串字段无效"));
	}
	return true;
}

bool ReadNumber(const TSharedPtr<FJsonObject>& Object, const FString& Name, float& OutValue, const float Minimum, const float Maximum, FText& OutError)
{
	double Value = 0.0;
	if (!Object->TryGetNumberField(Name, Value) || !FMath::IsFinite(Value) || Value < Minimum || Value > Maximum)
	{
		return Fail(OutError, TEXT("拒绝：配置数值字段超出范围"));
	}
	OutValue = static_cast<float>(Value);
	return true;
}

bool ReadStringArray(const TSharedPtr<FJsonObject>& Object, const FString& Name, TArray<FString>& OutItems, const int32 MaximumItems, const int32 MaximumLength, const bool bAllowEmpty, FText& OutError)
{
	const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
	if (!Object->TryGetArrayField(Name, Values) || Values == nullptr || (!bAllowEmpty && Values->IsEmpty()) || Values->Num() > MaximumItems)
	{
		return Fail(OutError, TEXT("拒绝：配置数组字段无效"));
	}
	OutItems.Reset();
	for (const TSharedPtr<FJsonValue>& Value : *Values)
	{
		FString Item;
		if (!Value.IsValid() || !Value->TryGetString(Item) || Item.TrimStartAndEnd().IsEmpty() || Item.Len() > MaximumLength)
		{
			return Fail(OutError, TEXT("拒绝：配置数组内容无效"));
		}
		OutItems.Add(Item);
	}
	return true;
}

bool ReadColor(const TSharedPtr<FJsonObject>& Object, FLinearColor& OutColor, FText& OutError)
{
	const TSharedPtr<FJsonObject>* Color = nullptr;
	if (!Object->TryGetObjectField(TEXT("color"), Color) || Color == nullptr || !HasOnlyFields(*Color, {TEXT("r"), TEXT("g"), TEXT("b"), TEXT("a")}, OutError))
	{
		return false;
	}
	return ReadNumber(*Color, TEXT("r"), OutColor.R, 0.0f, 1.0f, OutError)
		&& ReadNumber(*Color, TEXT("g"), OutColor.G, 0.0f, 1.0f, OutError)
		&& ReadNumber(*Color, TEXT("b"), OutColor.B, 0.0f, 1.0f, OutError)
		&& ReadNumber(*Color, TEXT("a"), OutColor.A, 0.0f, 1.0f, OutError);
}

bool ReadTransform(const TSharedPtr<FJsonObject>& Object, FTransform& OutTransform, FText& OutError)
{
	const TSharedPtr<FJsonObject>* Transform = nullptr;
	if (!Object->TryGetObjectField(TEXT("spawn_transform"), Transform) || Transform == nullptr || !HasOnlyFields(*Transform, {TEXT("location"), TEXT("rotation")}, OutError))
	{
		return false;
	}
	const TSharedPtr<FJsonObject>* Location = nullptr;
	const TSharedPtr<FJsonObject>* Rotation = nullptr;
	if (!(*Transform)->TryGetObjectField(TEXT("location"), Location) || Location == nullptr
		|| !(*Transform)->TryGetObjectField(TEXT("rotation"), Rotation) || Rotation == nullptr
		|| !HasOnlyFields(*Location, {TEXT("x"), TEXT("y"), TEXT("z")}, OutError)
		|| !HasOnlyFields(*Rotation, {TEXT("pitch"), TEXT("yaw"), TEXT("roll")}, OutError))
	{
		return false;
	}
	float X = 0.0f, Y = 0.0f, Z = 0.0f, Pitch = 0.0f, Yaw = 0.0f, Roll = 0.0f;
	if (!ReadNumber(*Location, TEXT("x"), X, -MaxLocationMagnitude, MaxLocationMagnitude, OutError)
		|| !ReadNumber(*Location, TEXT("y"), Y, -MaxLocationMagnitude, MaxLocationMagnitude, OutError)
		|| !ReadNumber(*Location, TEXT("z"), Z, -MaxLocationMagnitude, MaxLocationMagnitude, OutError)
		|| !ReadNumber(*Rotation, TEXT("pitch"), Pitch, -MaxRotationMagnitude, MaxRotationMagnitude, OutError)
		|| !ReadNumber(*Rotation, TEXT("yaw"), Yaw, -MaxRotationMagnitude, MaxRotationMagnitude, OutError)
		|| !ReadNumber(*Rotation, TEXT("roll"), Roll, -MaxRotationMagnitude, MaxRotationMagnitude, OutError))
	{
		return false;
	}
	OutTransform = FTransform(FRotator(Pitch, Yaw, Roll), FVector(X, Y, Z));
	return true;
}

bool ParseCharacter(const TSharedPtr<FJsonObject>& Object, const bool bPlayer, FZLSocialSandboxPlayerPreset& OutPlayer, FZLSocialSandboxNpcPreset& OutNpc, FText& OutError)
{
	const TSet<FString> Common = {TEXT("stable_id"), TEXT("type"), TEXT("display_name"), TEXT("color"), TEXT("spawn_transform"), TEXT("initial_health")};
	TSet<FString> Allowed = Common;
	if (!bPlayer)
	{
		Allowed.Add(TEXT("role"));
		Allowed.Add(TEXT("personality"));
		Allowed.Add(TEXT("speaking_style"));
		Allowed.Add(TEXT("goals"));
		Allowed.Add(TEXT("relationship"));
		Allowed.Add(TEXT("instant_state"));
	}
	if (!HasOnlyFields(Object, Allowed, OutError)) { return false; }
	FString StableId;
	FString Type;
	FString DisplayName;
	FLinearColor Color = FLinearColor::White;
	FTransform SpawnTransform;
	float Health = 0.0f;
	if (!ReadString(Object, TEXT("stable_id"), StableId, 128, OutError)
		|| !ReadString(Object, TEXT("type"), Type, 16, OutError)
		|| !ReadString(Object, TEXT("display_name"), DisplayName, 64, OutError)
		|| !ReadColor(Object, Color, OutError)
		|| !ReadTransform(Object, SpawnTransform, OutError)
		|| !ReadNumber(Object, TEXT("initial_health"), Health, 1.0f, MaxInitialHealth, OutError)
		|| (bPlayer && !Type.Equals(TEXT("player"), ESearchCase::CaseSensitive))
		|| (!bPlayer && !Type.Equals(TEXT("npc"), ESearchCase::CaseSensitive)))
	{
		return Fail(OutError, TEXT("拒绝：角色类型或基础字段无效"));
	}
	if (bPlayer)
	{
		OutPlayer.StableId = FName(*StableId);
		OutPlayer.DisplayName = FText::FromString(DisplayName);
		OutPlayer.BodyColor = Color;
		OutPlayer.SpawnTransform = SpawnTransform;
		OutPlayer.InitialHealth = Health;
		return OutPlayer.IsValid() ? true : Fail(OutError, TEXT("拒绝：玩家配置无效"));
	}
	FZLSocialSandboxNpcProfile Profile;
	FString Role;
	if (!ReadString(Object, TEXT("role"), Role, 128, OutError)
		|| !ReadStringArray(Object, TEXT("personality"), Profile.Personality, 8, 64, false, OutError)
		|| !ReadString(Object, TEXT("speaking_style"), Profile.SpeakingStyle, 256, OutError)
		|| !ReadStringArray(Object, TEXT("goals"), Profile.Goals, 8, 128, true, OutError))
	{
		return false;
	}
	const TSharedPtr<FJsonObject>* Relationship = nullptr;
	const TSharedPtr<FJsonObject>* InstantState = nullptr;
	if (!Object->TryGetObjectField(TEXT("relationship"), Relationship) || Relationship == nullptr
		|| !Object->TryGetObjectField(TEXT("instant_state"), InstantState) || InstantState == nullptr
		|| !HasOnlyFields(*Relationship, {TEXT("trust"), TEXT("affinity"), TEXT("fear"), TEXT("familiarity")}, OutError)
		|| !HasOnlyFields(*InstantState, {TEXT("fear"), TEXT("anger"), TEXT("curiosity"), TEXT("alert")}, OutError))
	{
		return false;
	}
	Profile.StableId = FName(*StableId);
	Profile.DisplayName = FText::FromString(DisplayName);
	Profile.Role = Role;
	Profile.BodyColor = Color;
	if (!ReadNumber(*Relationship, TEXT("trust"), Profile.Trust, -1.0f, 1.0f, OutError)
		|| !ReadNumber(*Relationship, TEXT("affinity"), Profile.Affinity, -1.0f, 1.0f, OutError)
		|| !ReadNumber(*Relationship, TEXT("fear"), Profile.RelationshipFear, 0.0f, 1.0f, OutError)
		|| !ReadNumber(*Relationship, TEXT("familiarity"), Profile.Familiarity, 0.0f, 1.0f, OutError)
		|| !ReadNumber(*InstantState, TEXT("fear"), Profile.Fear, 0.0f, 1.0f, OutError)
		|| !ReadNumber(*InstantState, TEXT("anger"), Profile.Anger, 0.0f, 1.0f, OutError)
		|| !ReadNumber(*InstantState, TEXT("curiosity"), Profile.Curiosity, 0.0f, 1.0f, OutError)
		|| !ReadNumber(*InstantState, TEXT("alert"), Profile.Alert, 0.0f, 1.0f, OutError))
	{
		return false;
	}
	OutNpc.Profile = Profile;
	OutNpc.SpawnTransform = SpawnTransform;
	OutNpc.InitialHealth = Health;
	return OutNpc.IsValid() ? true : Fail(OutError, TEXT("拒绝：NPC 配置无效"));
}
}

bool FZLSocialSandboxPlayerPreset::IsValid() const
{
	return !StableId.IsNone() && !DisplayName.IsEmptyOrWhitespace()
		&& FMath::IsFinite(BodyColor.R) && FMath::IsFinite(BodyColor.G) && FMath::IsFinite(BodyColor.B) && FMath::IsFinite(BodyColor.A)
		&& SpawnTransform.IsValid() && FMath::IsWithinInclusive(InitialHealth, 1.0f, MaxInitialHealth);
}

bool FZLSocialSandboxNpcPreset::IsValid() const
{
	return Profile.IsValid() && SpawnTransform.IsValid() && FMath::IsWithinInclusive(InitialHealth, 1.0f, MaxInitialHealth);
}

bool FZLSocialSandboxPreset::IsValid() const
{
	if (PresetId.TrimStartAndEnd().IsEmpty() || PresetId.Len() > 64 || !Player.IsValid() || Npcs.Num() < 2 || Npcs.Num() > MaxNpcCount)
	{
		return false;
	}
	TSet<FName> Ids = {Player.StableId};
	for (const FZLSocialSandboxNpcPreset& Npc : Npcs)
	{
		if (!Npc.IsValid() || Ids.Contains(Npc.Profile.StableId)) { return false; }
		Ids.Add(Npc.Profile.StableId);
	}
	return true;
}

FString FZLSocialSandboxPresetCodec::GetPresetDirectory()
{
	return FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("SocialSandbox/Presets"));
}

bool FZLSocialSandboxPresetCodec::IsSafePresetName(const FString& PresetName)
{
	if (PresetName.IsEmpty() || PresetName.Len() > 48) { return false; }
	for (const TCHAR Character : PresetName)
	{
		if (!(FChar::IsAlnum(Character) || Character == TEXT('_') || Character == TEXT('-'))) { return false; }
	}
	return true;
}

bool FZLSocialSandboxPresetCodec::LoadNamedPreset(const FString& PresetName, FZLSocialSandboxPreset& OutPreset, FText& OutError)
{
	if (!IsSafePresetName(PresetName)) { return Fail(OutError, TEXT("拒绝：预设名称无效")); }
	return LoadFromFile(FPaths::Combine(GetPresetDirectory(), PresetName + TEXT(".json")), OutPreset, OutError);
}

bool FZLSocialSandboxPresetCodec::LoadFromFile(const FString& FilePath, FZLSocialSandboxPreset& OutPreset, FText& OutError)
{
	const FString FullPath = FPaths::ConvertRelativePathToFull(FilePath);
	const FString AllowedDirectory = FPaths::ConvertRelativePathToFull(GetPresetDirectory());
	if (!FullPath.StartsWith(AllowedDirectory) || !FullPath.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase)) { return Fail(OutError, TEXT("拒绝：配置路径不在受控目录")); }
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *FullPath) || Text.Len() > 65536) { return Fail(OutError, TEXT("拒绝：无法读取受控配置文件")); }
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()
		|| !HasOnlyFields(Root, {TEXT("schema_version"), TEXT("preset_id"), TEXT("player"), TEXT("npcs")}, OutError)) { return Fail(OutError, TEXT("拒绝：配置 JSON 无效")); }
	double Version = 0.0;
	FString PresetId;
	const TSharedPtr<FJsonObject>* PlayerObject = nullptr;
	const TArray<TSharedPtr<FJsonValue>>* NpcValues = nullptr;
	if (!Root->TryGetNumberField(TEXT("schema_version"), Version) || Version != FZLSocialSandboxPreset::SchemaVersion
		|| !ReadString(Root, TEXT("preset_id"), PresetId, 64, OutError)
		|| !Root->TryGetObjectField(TEXT("player"), PlayerObject) || PlayerObject == nullptr
		|| !Root->TryGetArrayField(TEXT("npcs"), NpcValues) || NpcValues == nullptr) { return Fail(OutError, TEXT("拒绝：配置根字段无效")); }
	FZLSocialSandboxPreset Candidate;
	Candidate.PresetId = PresetId;
	FZLSocialSandboxNpcPreset Ignored;
	if (!ParseCharacter(*PlayerObject, true, Candidate.Player, Ignored, OutError)) { return false; }
	for (const TSharedPtr<FJsonValue>& Value : *NpcValues)
	{
		const TSharedPtr<FJsonObject>* NpcObject = nullptr;
		if (!Value.IsValid() || !Value->TryGetObject(NpcObject) || NpcObject == nullptr) { return Fail(OutError, TEXT("拒绝：NPC 配置对象无效")); }
		FZLSocialSandboxNpcPreset Npc;
		FZLSocialSandboxPlayerPreset IgnoredPlayer;
		if (!ParseCharacter(*NpcObject, false, IgnoredPlayer, Npc, OutError)) { return false; }
		Candidate.Npcs.Add(MoveTemp(Npc));
	}
	if (!Candidate.IsValid()) { return Fail(OutError, TEXT("拒绝：配置角色数量或稳定 ID 无效")); }
	OutPreset = MoveTemp(Candidate);
	OutError = FText::GetEmpty();
	return true;
}

bool FZLSocialSandboxPresetCodec::ExportToSaved(const FZLSocialSandboxPreset& Preset, FString& OutFilePath, FText& OutError)
{
	if (!Preset.IsValid() || !IsSafePresetName(Preset.PresetId)) { return Fail(OutError, TEXT("拒绝：无法导出无效配置")); }
	auto MakeColor = [](const FLinearColor& Color)
	{
		TSharedRef<FJsonObject> Value = MakeShared<FJsonObject>();
		Value->SetNumberField(TEXT("r"), Color.R); Value->SetNumberField(TEXT("g"), Color.G);
		Value->SetNumberField(TEXT("b"), Color.B); Value->SetNumberField(TEXT("a"), Color.A);
		return Value;
	};
	auto MakeTransform = [](const FTransform& Transform)
	{
		TSharedRef<FJsonObject> Location = MakeShared<FJsonObject>();
		const FVector Position = Transform.GetLocation();
		Location->SetNumberField(TEXT("x"), Position.X); Location->SetNumberField(TEXT("y"), Position.Y); Location->SetNumberField(TEXT("z"), Position.Z);
		TSharedRef<FJsonObject> Rotation = MakeShared<FJsonObject>();
		const FRotator Rotator = Transform.Rotator();
		Rotation->SetNumberField(TEXT("pitch"), Rotator.Pitch); Rotation->SetNumberField(TEXT("yaw"), Rotator.Yaw); Rotation->SetNumberField(TEXT("roll"), Rotator.Roll);
		TSharedRef<FJsonObject> Value = MakeShared<FJsonObject>();
		Value->SetObjectField(TEXT("location"), Location); Value->SetObjectField(TEXT("rotation"), Rotation);
		return Value;
	};
	auto MakeCommon = [&MakeColor, &MakeTransform](const FName StableId, const FText& DisplayName, const FLinearColor& Color, const FTransform& Transform, const float Health, const TCHAR* Type)
	{
		TSharedRef<FJsonObject> Value = MakeShared<FJsonObject>();
		Value->SetStringField(TEXT("stable_id"), StableId.ToString()); Value->SetStringField(TEXT("type"), Type);
		Value->SetStringField(TEXT("display_name"), DisplayName.ToString()); Value->SetObjectField(TEXT("color"), MakeColor(Color));
		Value->SetObjectField(TEXT("spawn_transform"), MakeTransform(Transform)); Value->SetNumberField(TEXT("initial_health"), Health);
		return Value;
	};
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("schema_version"), FZLSocialSandboxPreset::SchemaVersion);
	Root->SetStringField(TEXT("preset_id"), Preset.PresetId);
	Root->SetObjectField(TEXT("player"), MakeCommon(Preset.Player.StableId, Preset.Player.DisplayName, Preset.Player.BodyColor, Preset.Player.SpawnTransform, Preset.Player.InitialHealth, TEXT("player")));
	TArray<TSharedPtr<FJsonValue>> Npcs;
	for (const FZLSocialSandboxNpcPreset& Npc : Preset.Npcs)
	{
		TSharedRef<FJsonObject> Value = MakeCommon(Npc.Profile.StableId, Npc.Profile.DisplayName, Npc.Profile.BodyColor, Npc.SpawnTransform, Npc.InitialHealth, TEXT("npc"));
		auto MakeStringArray = [](const TArray<FString>& Strings)
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FString& String : Strings) { Values.Add(MakeShared<FJsonValueString>(String)); }
			return Values;
		};
		Value->SetStringField(TEXT("role"), Npc.Profile.Role); Value->SetArrayField(TEXT("personality"), MakeStringArray(Npc.Profile.Personality));
		Value->SetStringField(TEXT("speaking_style"), Npc.Profile.SpeakingStyle); Value->SetArrayField(TEXT("goals"), MakeStringArray(Npc.Profile.Goals));
		TSharedRef<FJsonObject> Relationship = MakeShared<FJsonObject>();
		Relationship->SetNumberField(TEXT("trust"), Npc.Profile.Trust); Relationship->SetNumberField(TEXT("affinity"), Npc.Profile.Affinity);
		Relationship->SetNumberField(TEXT("fear"), Npc.Profile.RelationshipFear); Relationship->SetNumberField(TEXT("familiarity"), Npc.Profile.Familiarity);
		TSharedRef<FJsonObject> InstantState = MakeShared<FJsonObject>();
		InstantState->SetNumberField(TEXT("fear"), Npc.Profile.Fear); InstantState->SetNumberField(TEXT("anger"), Npc.Profile.Anger);
		InstantState->SetNumberField(TEXT("curiosity"), Npc.Profile.Curiosity); InstantState->SetNumberField(TEXT("alert"), Npc.Profile.Alert);
		Value->SetObjectField(TEXT("relationship"), Relationship); Value->SetObjectField(TEXT("instant_state"), InstantState);
		Npcs.Add(MakeShared<FJsonValueObject>(Value));
	}
	Root->SetArrayField(TEXT("npcs"), Npcs);
	FString Text;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);
	if (!FJsonSerializer::Serialize(Root, Writer)) { return Fail(OutError, TEXT("拒绝：配置导出序列化失败")); }
	const FString Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SocialSandbox/Exports"));
	IFileManager::Get().MakeDirectory(*Directory, true);
	OutFilePath = FPaths::Combine(Directory, Preset.PresetId + TEXT(".json"));
	if (!FFileHelper::SaveStringToFile(Text, *OutFilePath)) { return Fail(OutError, TEXT("拒绝：无法写入配置导出文件")); }
	OutError = FText::GetEmpty();
	return true;
}
