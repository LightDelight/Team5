// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Marker/MarkerBase.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "LKH2/Interactables/Marker/MarkerData.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interaction/Component/InteractableComponent.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "LKH2/Interactables/Data/ItemStatValue.h"

AMarkerBase::AMarkerBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // ─── 루트 ───
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    // ─── 감지 Sphere ───
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(Root);
    DetectionSphere->SetSphereRadius(200.f);
    // InteractorComponent의 채널과 일치시켜야 Overlap 감지 가능
    // (실제 채널 값은 프로젝트 설정에 맞게 BP에서 변경)
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // ─── Niagara 이펙트 ───
    EffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectComponent"));
    EffectComponent->SetupAttachment(Root);
    EffectComponent->SetAutoActivate(false);

    // ─── 상호작용 컴포넌트 ───
    InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
    InteractableComponent->SetupAttachment(Root);

    // ─── PropertyComponent (결합도 높은 데이터 보관 + 내장 진행 위젯) ───
    PropertyComponent = CreateDefaultSubobject<UInteractablePropertyComponent>(TEXT("PropertyComponent"));
    PropertyComponent->SetupAttachment(Root);

    // ─── 블랙보드 ───
    BlackboardComponent = CreateDefaultSubobject<ULogicContextComponent>(TEXT("BlackboardComponent"));
}

void AMarkerBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMarkerBase, MarkerData);
}

void AMarkerBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("[MarkerBase] BeginPlay: %s (Authority=%s)"),
        *GetName(), HasAuthority() ? TEXT("Server") : TEXT("Client"));

    // SetMarkerDataAndApply에서 이미 적용됐으면 스킵
    if (HasAuthority() && MarkerData && !bDataApplied)
    {
        ApplyMarkerData();
    }
    else if (HasAuthority() && !MarkerData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MarkerBase] BeginPlay: MarkerData 없음 → ApplyMarkerData 생략"));
    }
    else if (HasAuthority())
    {
        UE_LOG(LogTemp, Log, TEXT("[MarkerBase] BeginPlay: 이미 ApplyMarkerData 완료됨 → 스킵"));
    }
}

void AMarkerBase::OnRep_MarkerData()
{
    ApplyMarkerData();
}

void AMarkerBase::SetMarkerDataAndApply(UMarkerData* InData)
{
    if (!InData) return;
    MarkerData = InData;
    bDataApplied = false; // 강제 재적용 허용
    ApplyMarkerData();
}

void AMarkerBase::ApplyMarkerData()
{
    if (!MarkerData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MarkerBase] ApplyMarkerData: MarkerData 없음"));
        return;
    }
    if (bDataApplied)
    {
        UE_LOG(LogTemp, Verbose, TEXT("[MarkerBase] ApplyMarkerData: 이미 적용됨 → 스킵 (%s)"), *GetName());
        return;
    }
    bDataApplied = true;

    UE_LOG(LogTemp, Log, TEXT("[MarkerBase] ApplyMarkerData 시작: %s (DA: %s)"),
        *GetName(), *MarkerData->GetName());

    // 1. DetectionSphere 반경 갱신
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(MarkerData->DetectionRadius);
        UE_LOG(LogTemp, Log, TEXT("[MarkerBase]   DetectionSphere 반경: %.1f"), MarkerData->DetectionRadius);
    }

    // 2. Niagara 이펙트 할당 및 활성화
    if (EffectComponent && MarkerData->MarkerEffect)
    {
        EffectComponent->SetAsset(MarkerData->MarkerEffect);
        EffectComponent->Activate(true);
        UE_LOG(LogTemp, Log, TEXT("[MarkerBase]   Niagara 이펙트 활성화: %s"), *MarkerData->MarkerEffect.GetFName().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[MarkerBase]   Niagara 이펙트 없음 (스킵)"));
    }

    // 3. PropertyComponent에 위젯 클래스 설정
    if (PropertyComponent && MarkerData->MarkerWidgetClass)
    {
        if (PropertyComponent->ProgressWidgetComponent)
        {
            PropertyComponent->ProgressWidgetComponent->SetWidgetClass(MarkerData->MarkerWidgetClass);
            PropertyComponent->ProgressWidgetComponent->SetVisibility(true);
            UE_LOG(LogTemp, Log, TEXT("[MarkerBase]   위젯 클래스 설정: %s"), *MarkerData->MarkerWidgetClass->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[MarkerBase]   위젯 클래스 없음 (스킵)"));
    }

    // 4. 로직 모듈 초기화
    if (InteractableComponent)
    {
        InteractableComponent->InitializeLogic(MarkerData, this);
        UE_LOG(LogTemp, Log, TEXT("[MarkerBase]   로직 모듈 초기화 완료 (%d개)"),
            MarkerData->GetAllModules().Num());
    }

    UE_LOG(LogTemp, Log, TEXT("[MarkerBase] ApplyMarkerData 완료"));
}

// ─── ILogicContextInterface ───

UInteractableComponent* AMarkerBase::GetInteractableComponent() const
{
    return InteractableComponent.Get();
}

UInteractablePropertyComponent* AMarkerBase::GetPropertyComponent() const
{
    return PropertyComponent.Get();
}

const FItemStatValue* AMarkerBase::FindStat(const FGameplayTag& Tag) const
{
    if (BlackboardComponent)
    {
        if (const FItemStatValue* Val = BlackboardComponent->FindStat(Tag))
            return Val;
    }
    if (MarkerData)
    {
        if (const FItemStatValue* Val = MarkerData->EntityStats.Find(Tag))
            return Val;
    }
    return nullptr;
}

void AMarkerBase::SetStat(const FGameplayTag& Tag, const FItemStatValue& Value)
{
    if (BlackboardComponent)
        BlackboardComponent->SetStat(Tag, Value);
}

FGameplayTag AMarkerBase::ResolveKey(const FGameplayTag& Key) const
{
    return Key;
}

TArray<ULogicModuleBase*> AMarkerBase::GetLogicModules() const
{
    if (InteractableComponent)
        return InteractableComponent->GetLogicModules();
    return {};
}

// ─── IInteractionContextInterface ───

bool AMarkerBase::OnInteract_Implementation(const FInteractionContext& Context)
{
    UE_LOG(LogTemp, Log, TEXT("[MarkerBase] OnInteract: %s → Intent: %s (Interactor: %s)"),
        *GetName(),
        *Context.InteractionTag.ToString(),
        Context.Interactor ? *Context.Interactor->GetName() : TEXT("System(None)"));

    if (InteractableComponent)
        return InteractableComponent->OnInteract(Context);

    UE_LOG(LogTemp, Error, TEXT("[MarkerBase] OnInteract: InteractableComponent 없음!"));
    return false;
}

void AMarkerBase::SetOutlineEnabled_Implementation(bool bEnabled, int32 StencilValue)
{
    TArray<UPrimitiveComponent*> PrimitiveComps;
    GetComponents<UPrimitiveComponent>(PrimitiveComps);
    for (UPrimitiveComponent* Comp : PrimitiveComps)
    {
        if (Comp)
        {
            Comp->SetRenderCustomDepth(bEnabled);
            Comp->SetCustomDepthStencilValue(1);
        }
    }
}
