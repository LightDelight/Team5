#include "PhysicsDoor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h" 

APhysicsDoor::APhysicsDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. 문틀 생성 (고정)
	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	RootComponent = DoorFrame;
	DoorFrame->SetMobility(EComponentMobility::Static);

	// 2. 문짝 생성 (움직임)
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorFrame);
	DoorMesh->SetMobility(EComponentMobility::Movable);

	// 물리 활성화
	DoorMesh->SetSimulatePhysics(true);
	DoorMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	DoorMesh->SetMassOverrideInKg(NAME_None, 50.0f, true);

	// 3. 경첩 생성
	DoorHinge = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("DoorHinge"));
	DoorHinge->SetupAttachment(DoorFrame);

	

	// A. 위치 이동 잠금 (문이 떨어지지 않게 꽉 잡음)
	
	DoorHinge->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.0f);
	DoorHinge->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.0f);
	DoorHinge->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.0f);

	// B. 회전 제한 (여닫이 문 설정)
	

	// Swing1: 문이 열리는 방향 (90도까지만 열리게 제한)
	DoorHinge->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 90.0f);

	// Swing2: 문이 위아래로 흔들리는 것 (잠금)
	DoorHinge->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);

	// Twist: 문이 꽈배기처럼 비틀리는 것 (잠금)
	DoorHinge->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);

	// 경첩 위치 이동
	DoorHinge->SetRelativeLocation(FVector(0.0f, 50.0f, 100.0f));
}

void APhysicsDoor::BeginPlay()
{
	Super::BeginPlay();

	// 4. 컴포넌트 연결 
	DoorHinge->SetConstrainedComponents(DoorFrame, NAME_None, DoorMesh, NAME_None);

	// 5. 각도 업데이트 
	DoorHinge->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, MaxOpenAngle);
}