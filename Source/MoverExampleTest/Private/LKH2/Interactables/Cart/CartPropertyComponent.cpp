// Fill out your copyright notice in the Description page of Project Settings.

#include "LKH2/Interactables/Cart/CartPropertyComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "LKH2/Interactables/Cart/CartData.h"
#include "LKH2/Interactables/Data/LogicEntityDataBase.h"
#include "LKH2/Interaction/Base/LogicModuleBase.h"
#include "LKH2/Interaction/Component/InteractablePropertyComponent.h"
#include "LKH2/Interaction/Base/LogicContextComponent.h"
#include "LKH2/Interaction/Interface/InteractionContextInterface.h"
#include "LKH2/LKH2GameplayTags.h"

UCartPropertyComponent::UCartPropertyComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);

    // BoxCollision을 생성자에서 자식으로 생성 → 에디터에서 즉시 확인 가능
    DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
    DetectionBox->SetupAttachment(this);
    DetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionBox->SetGenerateOverlapEvents(true);
}

void UCartPropertyComponent::OnRegister()
{
    Super::OnRegister();

    DetectionBox->OnComponentBeginOverlap.AddUniqueDynamic(
        this, &UCartPropertyComponent::OnBoxBeginOverlap);
}

void UCartPropertyComponent::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 로직 모듈 초기화 (Authority)
    if (AActor* Owner = GetOwner())
    {
        if (Owner->HasAuthority())
        {
            InitializeLogic();
        }
    }
}

void UCartPropertyComponent::InitializeLogic()
{
    if (bLogicInitialized || !CartData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CartPropertyComponent] InitializeLogic 무시: bLogicInitialized=%d, CartData=%s"),
            bLogicInitialized,
            CartData ? *CartData->GetName() : TEXT("null"));
        return;
    }

    bLogicInitialized = true;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    TArray<ULogicModuleBase*> Modules = CartData->GetAllModules();
    LogicModules.Empty(Modules.Num());

    for (ULogicModuleBase* Module : Modules)
    {
        if (Module)
        {
            ULogicModuleBase* Instanced = DuplicateObject<ULogicModuleBase>(Module, this);
            if (Instanced)
            {
                LogicModules.Add(Instanced);
                Instanced->InitializeLogic(Owner, CartData);
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("[CartPropertyComponent] InitializeLogic 완료: %d개 모듈 로드 (CartData: %s)"),
        LogicModules.Num(), *CartData->GetName());
}

void UCartPropertyComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 서버에서만 전복 감지 수행
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority())
        return;

    TimeSinceLastOverturn += DeltaTime;
    if (TimeSinceLastOverturn < OverturnCooldown)
        return;

    FVector CartUp = Owner->GetActorUpVector();
    float DotProduct = FVector::DotProduct(CartUp, FVector::UpVector);
    float AngleDegrees = FMath::RadiansToDegrees(
        FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

    if (AngleDegrees > OverturnAngleThreshold)
    {
        TimeSinceLastOverturn = 0.0f;

        FInteractionContext Context;
        Context.Interactor = nullptr;
        Context.InteractionTag = FLKH2GameplayTags::Get().Intent_Cart_Overturn;
        OnInteract(Context);
    }
}

void UCartPropertyComponent::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp,
                                               AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp,
                                               int32 OtherBodyIndex,
                                               bool bFromSweep,
                                               const FHitResult& SweepResult)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority() || !OtherActor)
        return;

    FInteractionContext Context;
    Context.Interactor = nullptr;
    Context.InteractionTag = FLKH2GameplayTags::Get().Intent_Cart_ItemOverlap;
    Context.TargetActor = OtherActor;

    UE_LOG(LogTemp, Log, TEXT("[CartPropertyComponent] ItemOverlap 감지: %s"), *OtherActor->GetName());

    OnInteract(Context);
}

bool UCartPropertyComponent::OnInteract(const FInteractionContext& Context)
{
    // TargetActor가 없으면 소유 액터로 채움 (InteractableComponent와 동일 패턴)
    FInteractionContext ModifiedContext = Context;
    if (!ModifiedContext.TargetActor)
        ModifiedContext.TargetActor = GetOwner();

    // 소유 액터의 PropertyComponent / ContextComponent 자동 주입
    if (AActor* Owner = GetOwner())
    {
        ModifiedContext.InteractableComp = this;
        ModifiedContext.InteractablePropertyComp =
            Owner->FindComponentByClass<UInteractablePropertyComponent>();
        ModifiedContext.ContextComp =
            Owner->FindComponentByClass<ULogicContextComponent>();
    }

    UE_LOG(LogTemp, Log, TEXT("[CartPropertyComponent] OnInteract - Intent: %s, Interactor: %s, LogicModules: %d개"),
        *ModifiedContext.InteractionTag.ToString(),
        ModifiedContext.Interactor ? *ModifiedContext.Interactor->GetName() : TEXT("System(None)"),
        LogicModules.Num());

    // 로직 모듈 순회
    for (ULogicModuleBase* Module : LogicModules)
    {
        if (Module && Module->ExecuteInteraction(ModifiedContext))
            return true;
    }
    return false;
}
