


InteractorComponent
플레이어에게 부착되는 컴포넌트
Tag 기반 Intent와 Context를 담아 상호작용 요청 메시지를 보낸다.
씬 컴포넌트로서 위치를 가짐. 아이템의 스냅 위치를 담당함.
SphereCollision로 탐색한 Actor와 GridManager로부터 받아온 Actor 두 가지 대상을 갖는다.

InteractionContextInterface
Interaction/Interface에 있음.
상호작용 메시지 중계 인터페이스. 외곽선 온오프 요청 전달도 포함한다.
액터가 상속받는다. 스스로 컴포넌트를 찾게 하기 위함.

InteractableComponent
액터에게 부착되는 컴포넌트
수신받은 요청에 자신의 Context를 추가해 LogicContextComponent에게 전달한다.
씬 컴포넌트로서 위치를 가짐. 아이템의 스냅 위치를 담당함.
외곽선 온오프 함수의 구체적 정의를 가진다.
경쟁 상태 관리를 맡는다.
로직의 생명주기를 관리한다.

InteractorPropertyComponent / InteractablePropertyComponent
결합도가 강한 데이터를 모아두는 컴포넌트. 또한 스냅 등 관련 기능을 가진다.

LogicContextComponent
액터에게 부착되는 컴포넌트
런타임 스탯을 LogicBlackboard로 관리한다.

LogicBlackboard
로직을 Stateless로 유지하면서 컴포넌트에 변수가 지속적으로 늘어나는 것을 방지하기 위해 도입한 동적 저장소.
태그를 통해 데이터를 연결한다.

LogicContextInterface
로직이 필요한 정보를 받기 위해 액터를 중재자로 만드는 인터페이스.
Context Payload를 적용한 후에는 불필요하나, Context를 적용하기 어려운 경우를 대비해 남겨둠.

InteractionManager
실제 행동을 정의하는 곳. (예 : ExecuteDrop = 아이템을 플레이어에게서 분리하고, 땅에 떨어뜨린다.)
게임에 필요한 모든 실제 행동은 매니저에 존재해야 한다. 그러나 반드시 헬퍼 함수를 통해서만 이루어져야 한다.
InteractionManager가 전달받는 인자는 할 수 있는 가장 최소 단위어야 한다.



LogicModuleBase
로직 모듈의 베이스. 상속받아 로직을 작성한다.
사전, 핵심, 사후 로직을 가진다. 외부 호출용 통합 실행 함수를 가진다.
사전, 핵심, 사후 로직은 각각 별도로 구현할 수 있다. 그러나 항상 통합 실행 함수만이 호출되어야 한다.

로직 모듈은 수행되지 않을 경우 다음 차례의 모듈에게 순서를 넘긴다.
모듈이 실행될 경우 다음 차례의 모듈은 호출되지 않는다.
모든 로직이 수행되지 않는다면 아무 일도 일어나지 않아야 한다.

Context에 필요한 데이터가 없다면 Context를 수정한다. 로직이 스스로 가져오지 않는다. 예외는 기술부채로 남긴다.

사전 로직은 Context를 통해 로직이 실행될지 여부를 판단한다.
사전 로직은 기본적으로 Context를 통해 어떤 입력인지 의도를 구분하여야 한다. (예: 줍기/놓기인지, 던지기인지)

핵심 로직은 Context를 통해 InteractionManager의 헬퍼 함수를 호출한다.
직접 상태를 변경하는 것은 최대한 지양한다. 피치 못하게 변경한다면 기술 부채로 남긴다.

사후 로직은 항상 핵심 로직이 이루어진 후에만 진행되어야 한다.
현재 사후 로직은 공통 수행 로직이 없어 의미가 없는 상태이나 차후를 위해 남겨둠.



RecipeLogicModuleBase : LogicModuleBase
레시피를 사용하는 로직 전용 모듈. 기본적인 레시피 캐싱 및 정렬 가상 함수를 갖고 있음.

RecipeBookBase
레시피를 사용하는 로직 전용 데이터. 기본 Recipe 구조체가 구현되어 있지만 현재 미사용.

CombineRecipeBook : RecipeBookBase
Combine 로직 전용 레시피 데이터. 전용 Recipe 구조체를 가짐.



LogicEntityDataBase
에디터에서 DataAsset으로 관리됨. 로직과 스탯을 가짐.
로직은 LogicModuleBase를 사용.
스탯은 ItemStatValue를 사용. 태그로 관리됨.

VisualPreset
비주얼 관련 데이터를 미리 정의.
만들어진 이유는 메쉬마다 중심이 달라야 할 경우 중복 작업을 피하기 위함.



ItemBase : InteractionContextInterface, ILogicContextInterface
아이템의 베이스 클래스.

ItemData : LogicEntityDataBase
비주얼 관련 데이터를 가짐. 스폰 클래스, 메쉬, 콜리전/오프셋 등 시각적 기본값. (클래스는 추후 분리 가능)
비주얼 프리셋을 가지거나 커스텀 비주얼을 사용할 수 있음.
태그, 무게를 기본으로 가짐. 콜리전 크기는 Sphere 기준. (무게는 추후 분리 가능)

ItemStateComponent
아이템의 물리 및 복제 상태를 관리하는 컴포넌트.
상태 머신 기반으로 구현함.

ItemSmoothingComponent
아이템의 부드러운 움직임을 담당하는 컴포넌트.
클라이언트가 서버의 복제된 위치를 받을 때 보간하여 부드럽게 이동시킨다. 데드레코닝을 포함한다.
현재 Manager와 통신하지 않는다.

ContainerItemBase : ItemBase
접시 등 아이템을 담을 수 있는 아이템의 베이스 클래스.
ItemBase에 Property Component가 기본으로 들어간다면 필요없을 수도 있다.

ItemManagerSubsystem
아이템의 생성과 소멸을 관리하는 월드 서브시스템.
아이템 데이터의 GameplayTag와 자체 생성 고유 ID를 기반으로 모듈과 통신함.
아이템의 변화는 항상 매니저를 통해 이루어짐. 매니저는 Carried 등 게임에 필요한 아이템의 상태를 모두 래핑함.
아이템의 논리적 상태 갱신만 전담하며, 물리적인 부착(Snap) 등은 관여하지 않는다.
ItemStateComponent의 상태 머신을 그대로 호출하는 형태여도 예외로 두지 않음.

ItemRegistryData
매니저에 등록할 아이템을 모아놓은 데이터 에셋.
클래스가 지정되지 않은 아이템의 클래스를 기본 클래스 또는 에러 클래스로 지정하는 플래그를 갖고 있음.



Workstation : InteractionContextInterface, ILogicContextInterface
작업대의 베이스 클래스.

WorkstationData : LogicEntityDataBase
비주얼 관련 데이터를 가짐. 스폰 클래스, 메쉬, 콜리전/오프셋 등 시각적 기본값. (클래스는 추후 분리 가능)
비주얼 프리셋을 가지거나 커스텀 비주얼을 사용할 수 있음.
콜리전 크기는 Box 기준. 매니저를 만들게 될 경우 태그를 가지게 될 것.



GridManagerComponent
GameState의 컴포넌트. 작업대 등 움직이지 않는 객체들을 그리드로 관리한다.
맵 베이크 플래그를 가졌다. 체크 후 시작 시 갖고 있는 맵 데이터에 월드의 정보를 저장한다.
추후 Interactor 에게 그리드 기반 객체 정보를 제공해 트레이스 없이 접근 가능하게 할 예정.

GridCell
그리드를 구성하는 단위. 무엇이 존재하는지 정보를 갖고 있음.

MapData
베이크된 맵 데이터. 그리드 설정과 워크스테이션 배치 정보를 보유함.



구조 원칙
모든 객체는 서로의 역할을 침범하지 않는다.
PropertyComponent 는 아이템의 스냅을 담당하면서 아이템의 상태는 신경쓰지 않는다.
ItemManagerSubsystem 은 아이템의 상태를 관리하며 실제로 그 상태에 있는지는 신경쓰지 않는다.
LogicModule 은 Context를 기반으로 작업의 진행 여부를 판단하며 실제로 그 작업을 수행하지 않는다.
InteractionManager 는 헬퍼 함수들을 조합해 실제 행동을 정의하나 그 내용을 알지 못한다. 아는 것은 개발자다.

모든 구현은 철저히 역할을 나누어 분리한다.
던지기가 있다면, State에 논리적인 던지기 상태와 힘을 주는 헬퍼가 정의되어야 한다.
ItemManager는 State의 헬퍼를 래핑하여 외부 요청 헬퍼를 제공한다.
InteractorPropertyComponent는 아이템을 플레이어에게서 물리적으로 분리하는 헬퍼를 보유해야 한다.
Interactor는 Context에 Intent와 던지기에 필요한 데이터를 담아 요청을 보낸다.
Interactables는 받은 요청과 Context에 자신의 Context를 추가해 LogicModule에 전달한다.
LogicModule은 Context를 기반으로 진행 여부를 판단하고, Context의 데이터를 담아 Manager에 행동을 요청한다.
InteractionManager는 전달받은 데이터와 헬퍼 함수들을 조합해 실제 행동을 수행한다.



기술 부채
현재 Context는 모든 Component를 담는 식으로 구현되어 있다.
이는 로직 작성 속도를 높이고 컴포넌트 탐색 비용을 줄이기 위한 편의 기능이다.
단, 로직은 반드시 이 참조를 데이터 제공자로만 사용해야 하며, 상태를 직접 변경해서는 안 된다.
상태 변경은 반드시 InteractionManager의 헬퍼 함수를 통해 이루어져야 한다. 예외는 기술부채로 남긴다.

InteractorPropertyComponent가 ForceEquip 함수에서 InteractorComponent를 직접 참조함.

Stateless를 유지해야 하는 원칙에 Throw 모듈이 던지는 힘을 보유하고 있어 위배?



이슈 목록
클라이언트에서 아이템이 겹쳐있을 때 외곽선 대상과 상호작용 대상이 간헐적으로 어긋나는 경우 발생.

상호작용 가능 환경 세팅 워크플로우 만들기.


진행해야 하는 작업
메인 작업 로직 (예 : 요리, 재료 손질 등)




추가 예정
ProcessingLogicBase, ProcessorComponent
Interactor 없이도 스스로 진행되어야 하는 로직을 위한 베이스. (예 : 불 위에서 구워지는 고기)
상태는 블랙보드에 저장. 로직 데이터 투트랙.

LogicalPreset
모듈 배열과 스탯을 미리 정의해 반복 작업을 피하는 프리셋.

PropertyComp의 필드도 태그로 받아올 수 있게 (UI용)



개편 예정
LogicModule을 다시 현장 관리자로 변경.
InteractioManager는 헬퍼를 모아 통제된 래퍼를 제공. (예 : ChangeStateSafe, AttachItemSafe 등)
LogicModule의 블루프린트화. 에디터에서 실제 행동을 정의할 수 있도록 변경.

ItemSmoothing State에 따라 중지?