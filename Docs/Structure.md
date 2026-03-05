


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
실제 행동의 원자적 API를 제공하는 곳. (예 : SafeSpawnItem, SafePickUpItem)
게임에 필요한 모든 실제 행동은 매니저에 존재해야 한다. 그러나 반드시 헬퍼 함수를 통해서만 이루어져야 한다.
InteractionManager가 전달받는 인자는 할 수 있는 가장 최소 단위여야 한다.



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

에디터 누락을 방지하기 위해 기본 태그를 모아두고 배정한다.



ItemRecipeManager
아이템 레시피를 관리하는 GameInstanceSubsystem.
시작 후 레시피를 등록해야 한다. Tag 기반으로 통신한다.

ItemRecipeData
레시피를 담는 데이터 에셋. 레시피 DT와 설정 DA로 이루어져 있다.



HoldingStepLogicModuleBase
Montage&Notify에 의존하는 Step 진행도를 가진 로직 모듈의 베이스. (예 : 도마에 칼을 내려칠 때마다 진행)

LogicProgressWidget
진행도를 가진 로직을 위한 UI.
아이템이 보유하고 모든 클라이언트에게 실시간으로 진행도를 공유한다.



LogicTaskBase
로직의 비동기 작업을 위한 베이스.
로직 모듈의 인스턴스를 생성하지 않으면서 지속적인 상태 보관이 필요한 로직에 사용한다.

LogicTask_PlayMontageAndWait
Montage와 관련된 모듈을 위한 Task.



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



핵심 구조
Interaction System
모든 기능은 로직 모듈의 형태로 하나의 메시지를 통한다.
메시지는 전달 과정에서 로직 처리에 필요한 모든 정보를 담는다. (Context Payload)
모듈은 사전 로직에서 메시지 의도 검사를 통해 자신이 수행되어야 하는지 판단한다.
플레이어 입력뿐 아니라, 컴포넌트 혹은 모듈 스스로도 메시지를 발송해 모듈을 작동시킴으로써 모든 로직 모듈이 동일한 흐름을 타게 된다.

Task Architecture
로직의 비동기 작업 관리 패턴
단일성 및 단순 로직: Task 인스턴스화 없이 LogicModule 내부에서 즉시 처리 (성능 최적화).
지연 및 가변 로직: 시간이 필요하거나 상태(몽타주, 게이지 등)가 필요한 경우에만 LogicTaskBase를 인스턴스화하여 사용.
데이터 주입 및 통신: LogicModule은 생성한 Task에 설정값을 주입하고, 해당 Task의 포인터는 GameplayTag를 키로 하여 LogicContextComponent에 저장.
재호출 및 중단: 키 뗌(Release) 등의 추가 Intent가 발생하면, 다시 기존의 로직 호출 흐름을 따라가며, 해당 모듈의 사전 로직에서 Intent 검사와 Tag를 통해 현재 Task 존재 여부를 통해 분기를 만들어 추가 작업 진행.



설계 원칙
모든 객체는 서로의 역할을 침범하지 않는다.
PropertyComponent 는 아이템의 스냅을 담당하면서 아이템의 상태는 신경쓰지 않는다.
ItemManagerSubsystem 은 아이템의 상태를 관리하며 실제로 그 상태에 있는지는 신경쓰지 않는다.
LogicModule 은 Context를 기반으로 작업의 진행 여부를 판단하며 실제로 그 작업을 수행하지 않는다.
InteractionManager 는 헬퍼 함수들을 조합해 실제 행동을 정의하나 그 내용을 알지 못한다.

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

ItemStateComp에서 ItemSmoothingComp를 직접 참조함.

현재 대부분의 인터페이스는 컴포넌트를 직접 전달하는 반쪽짜리. 데이터를 꺼내주는 형태로 개선할 필요 있음.

ItemSpanwer 급하게 추가. 점검 필요

Cart Snap, Spill에 CartGameComponent 직접 연결



이슈 목록
클라이언트에서 아이템이 겹쳐있을 때 외곽선 대상과 상호작용 대상이 간헐적으로 어긋나는 버그 (해결)
이미 C++ 내부에서 RPC였던 함수를 블루프린트에서도 RPC로 호출해서 서버 기준으로 판정이 잡혀서 발생했음.

Item 부착 클라이언트 이동 동기화 문제 (해결)
캐릭터 부착이 잘 되는 이유: InteractorPropertyComponent가 OnRep_CarriedActor를 통해 클라이언트에서 수동으로 AttachToComponent를 호출해주고 있기 때문입니다.
컨테이너 부착이 안 되는 이유: InteractablePropertyComponent는 아이템 보관 맵을 리플리케이션하지 않으며, 클라이언트에서의 수동 부착 로직이 없습니다. 언리얼 기본 액터 부착 리플리케이션에만 의존하고 있는데, 액터-액터 간의 중첩 부착은 매우 불안정하여 클라이언트에서 누락되는 경우가 많습니다.
해결 방안 (수동 동기화):
아이템 스스로 부모를 알게 함: ItemStateComponent에 리플리케이션 변수(RepParentActor, RepAttachComponent)를 추가합니다.
클라이언트 부착 강제: 클라이언트에서 OnRep이 발생할 때, 서버가 알려준 부모 액터/컴포넌트에 강제로 AttachToComponent를 수행하여 캐릭터 부착과 동일한 수준의 신뢰성을 확보합니다.



진행해야 하는 작업
Throw 로직의 던지는 힘을 캐릭터 Stats로 이동
상호작용 가능 환경 세팅 워크플로우 만들기. (Post Process, Overlap Channel)
진행도 위젯 기본 Screen? 쿼터뷰 기준 한 방향으로만 만들 예정이었으나 tps라 현재 미정.
ItemManager Registry 타이밍 문제 해결.



필수 작업
게임모드
UI에 제한 시간과 목표 아이템 목록 표시.
카트에 목표 아이템 담기면 체크해서 UI에 표시.
쏟아질 경우 UI 초기화.
계산대 통과 시 목표 달성. 결과 표시 후 대기 화면으로 돌아가기.

카트 인벤토리
Box Overlap 시 아이템 스냅.
전복 감지 시 아이템 쏟기, 에디터에서 꽂은 DataAsset을 통해 Workstation 생성.
Workstation은 홀딩 로직 모듈 하나를 가짐. 생성 후 Context에 아이템 UID와 Intent를 담아 전송해줌.
로직 모듈은 Context를 받아 아이템을 보관. 상호작용 시 해당 아이템들을 쓰레기 봉투에 모으고 작업대는 삭제.
쏟아진 아이템은 로컬 시뮬레이션으로 굴린다. 상호작용 불가. 카트도 해당 상호작용을 통해 일으켜 세운다.



추가 예정
LogicalPreset : 모듈 배열과 스탯을 미리 정의해 반복 작업을 피하는 프리셋.
PropertyComp의 필드도 태그로 받아올 수 있게 (UI용)



개선 아이디어
아이템의 상태 변화를 UID를 이용해 관리한다면 코드가 훨씬 깔끔해질 것으로 예상. 필요할 경우 WeakPtr 포함.

접시에 무엇이 담겼는지 확인할 수 있는 아이콘 표시?

컨테이너 비워질 경우 스스로에게 블랙보드 초기화 메시지 전송.
모듈들도 스스로에게 메시지를 전송하는 방식으로 다음 단계 진행하는 방식으로 확장? 델리게이트 없이?

아이템의 상태 변화를 유도하는 모듈을 더 쪼개서 아이템이 스스로 상태 변화를 유도하는 방식으로 변경?
굽기 완료 시 아이템에 메시지를 보내 아이템이 스스로 변화하게 하거나, 수납 시 아이템이 스스로 붙게 하거나?
현재 DA와 모듈은 별도의 존재나 다름 없음. 한 액터에 묶여있을 뿐. 액터와도 극단적인 디커플링으로 거의 별도의 존재. 그렇다면 차라리 모듈을 쪼개기 보다 하나로 뭉치는 게? 지금도 이미 역할을 나눠놓고 인터랙션 매니저를 통해 묶고 있으니? 오히려 DA로도 묶지 말고 더 자유로운 형태로? 싱글톤 객체로 만들고 DA에는 조건만 보유? 이 경우 태그 작업 등 에디터 반복 작업이 크게 줄어들 것으로 예상.

다중 레시피 개선? 순서 상관 없게 중간 아이템까지 포함할 수 있도록?