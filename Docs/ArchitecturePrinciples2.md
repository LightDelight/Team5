# **🏗️ 소프트웨어 아키텍처 원칙 (Architecture Principles)**

본 프로젝트는 의존성을 최소화하고 확장성을 극대화하기 위해 다형성(Polymorphism)과 데이터 주도(Data-Driven) 설계를 결합하여 다음과 같은 엄격한 책임 분리 대원칙을 따릅니다:

## **1\. Component는 상태(State)와 명세(Specification)를 맡는다.**

* **역할**: 데이터 컨테이너, 인터페이스 수신부, 수동적 반응자.  
* **설명**: 컴포넌트(UCarryComponent, UCarryableComponent, UItemStateComponent, 데이터 구조체 등)는 "현재 상태가 어떠한가?" 혹은 "어떤 로직 모듈들을 가지고 있는가?"와 같은 \*\*현재 상태(State)\*\*와 객체의 \*\*고유 속성/명세(Spec)\*\*만을 정의합니다.  
* **제한**: 컴포넌트 내부에는 "이 상태일 때 저렇게 한다"는 조건 판정(규칙)이나, 능동적으로 물리적 액션(Impulse, Attach/Detach 등)을 가하는 **행동(Behavior)** 코드가 존재해서는 안 됩니다. 외부(Logic)가 변수를 바꿔주면 그에 맞춰 시각적/물리적 세팅을 갱신하는 수동적인 \*\*반응(Reaction / OnRep)\*\*만을 수행합니다.

## **2\. Logic은 판단(Judgment)과 행동(Behavior)을 맡는다.**

* **역할**: 조건 판정(Rule Enforcement), 능동적 상태 전이, 물리 제어.  
* **설명**: 모든 ULogicModuleBase 파생 모듈(Logic\_Carryable\_Common, Logic\_CarryInteract\_Combine 등)은 실제 오브젝트가 상황을 판단하고 무엇을 어떻게 "할 것"인지 결정하는 유일한 \*\*실행 주체(Doer)\*\*입니다.  
* **동작**: Base나 Component가 위임(Dispatch)한 인터페이스 메시지를 받아 책임을 연쇄적으로(Chain of Responsibility) 확인합니다. "플레이어 손에 무엇이 있는가?", "작업대가 비어있는가?" 등의 **규칙(Rules)과 조건**을 오직 Logic만이 판정하며, 검증이 완료되면 비로소 대상의 상태 컴포넌트 및 물리를 직접 조작합니다.

## **3\. Base는 연결(Connection)과 고유 책임만을 맡는다.**

* **역할**: 설계 기반(Foundation), 컴포넌트 조립 뼈대, 엔진 상호작용 브리지.  
* **설명**: AItemBase, AWorkStationBase 등 기본 액터 클래스들은 스스로 복잡한 상태 처리나 물리 제어를 일절 수행하지 않습니다. 분리하기에는 너무 작고 결합도가 높은 규칙이거나, 언리얼 엔진 액터 클래스 단에서만 가질 수 있는 고유한 규칙(예: GetLifetimeReplicatedProps를 통한 멤버 리플리케이션 선언, 기본 콜리전/메시 구조 조립, 순수한 인터페이스 호출 전달) 만을 허용합니다.

## **🧩 주요 시스템 세부 구조 및 역할 (System Breakdown)**

### **📦 Components (상태 및 데이터 홀더)**

* **UItemStateComponent**: 아이템의 현재 상태(EItemState: Placed, Carried, Stored 등)를 관리합니다. *RepNotify*를 통해 서버에서 변경된 상태를 클라이언트로 복제하여 시각적 처리 및 물리 설정을 동기화하는 핵심 축입니다.  
* **UCarryComponent**: 플레이어(Interactor)에 부착되어 들기/내려놓기/던지기 입력 버퍼링을 처리하고, 캐스팅 대신 인터페이스를 통해 대상에게 메시지를 전달(Dispatch)하는 역할을 합니다.  
* **UCarryableComponent**: 대상 아이템에 부착되어 해당 아이템이 들릴 수 있는 객체임을 나타내며, 상호작용 메시지를 받아 자신의 ItemData에 등록된 LogicModule들에게 전달합니다.  
* **UCarryInteractComponent**: 워크스테이션(작업대 등)에 부착되어 상호작용(예: 조합, 거치) 메시지를 수신하며, 내장된 WorkstationData의 로직 모듈 배열을 순회해 책임을 위임합니다. 자체적으로 \*\*FLogicBlackboard\*\*를 호스팅합니다.

### **🔌 Interfaces (통신 규약 및 의존성 분리)**

* **ICarryInterface** (OnCarryInteract): 베이스 클래스와 컴포넌트 간의 주된 상호작용 진입점입니다. ECarryInteractionType(Interact, Throw)을 매개변수로 받아 의도를 명확히 전달합니다.  
* **ICarryLogicInterface** (OnModuleInteract): 로직 모듈들이 실제 상호작용을 처리하는 진입점입니다. 컴포넌트들은 이 인터페이스를 통해 모듈들에게 "이 행동을 처리할 수 있는가?" (Chain of Responsibility)를 묻습니다.  
* **IInstigatorContextInterface**: 상호작용 주체(플레이어)로부터 컨텍스트(예: "현재 양손이 비어있는가?", "현재 들고 있는 아이템 컴포넌트는 무엇인가?")를 안전하게 질의하기 위한 인터페이스입니다. 이를 통해 아이템이 플레이어 클래스에 직접 의존하는 것을 방지합니다.

### **🧠 Logic Modules (판단 및 실행 주체)**

* **Logic\_Carryable\_Common**: 일반적인 아이템의 줍기, 내려놓기, 던지기를 담당합니다.  
  * *줍기*: IInstigatorContextInterface를 통해 플레이어 손이 빈 것을 확인한 뒤 아이템 상태를 Carried로 변경하고 캐릭터에 Attach 합니다.  
  * *던지기/내려놓기*: 아이템 상태를 Placed 로 변경하고 Detach 후 AddImpulse 와 물리 시뮬레이션을 활성화합니다.  
* **Logic\_CarryInteract\_Common**: 워크스테이션에 아이템을 올려두거나(거치), 다시 플레이어가 집어가는 행위를 담당합니다. 대상 아이템 상태를 Stored(물리 비활성화, 부착 유지)로 변경하며, 유령 현상(Phantom Snapping) 방지를 위해 지연 평가를 수행합니다.  
* **Logic\_CarryInteract\_Combine**: 작업대에 놓인 아이템과 플레이어가 들고 있는 아이템을 조합하여 새로운 아이템을 스폰하는 로직입니다.

### **🗄️ Data Assets (데이터 주도 설계)**

기능 확장을 위해 클래스 상속(Subclassing)을 최소화하고 데이터 에셋 중심의 확장을 지향합니다.

* **UItemData**: 각 아이템이 어떤 논리적 특성(LogicModules)을 가질지 정의하는 데이터 컨테이너입니다.  
* **UWorkstationData**: 각 워크스테이션이 처리가능한 로직 모듈들을 담는 데이터 컨테이너입니다.

### **📋 Logic Blackboard (상태 공유 공간)**

* **FLogicBlackboard**: 워크스테이션이(UCarryInteractComponent) 갖는 일종의 메모리(Memory) 공간입니다.  
* 모듈 간(예: Common 로직 모듈이 아이템을 거치하면, 나중에 Combine 모듈이 이를 읽어 조합에 사용)에 데이터를 주고받기 위해 고안된 구조체로, GameplayTag를 키로 하여 액터를 저장합니다. 멀티플레이어 환경에서 안전하게 복제(Replicated)되어 상태 일관성을 보장합니다.

## **🛠️ 핵심 구현 상세 및 설계 이유 (Implementation Details & Rationale)**

본 프로젝트는 단순한 스파게티 코드를 피하기 위해 특정 패턴과 엔진의 고급 기능을 적극 활용하여 구조화 되었습니다.

### **1\. FastArraySerializer를 활용한 Blackboard 네트워크 동기화**

* **구현 방법**: FLogicBlackboard는 단순 TArray가 아닌 언리얼 엔진의 FFastArraySerializer를 상속(FFastArraySerializerItem 구조체와 함께 사용) 받아 구현되었습니다.  
* **도입 이유**:  
  1. 일반적인 TArray 리플리케이션은 배열 내 항목 하나만 변경되어도 배열 전체를 다시 직렬화(Serialize)하여 전송하므로 네트워크 대역폭 낭비가 큽니다.  
  2. FFastArraySerializer는 추가(Add), 삭제(Remove), 변경(Modify)된 구체적인 **델타(Delta) 업데이트만 서버에서 클라이언트로 전송**하므로 트래픽 최적화에 탁월합니다.  
  3. 워크스테이션에 여러 아이템이 거치되고 조합되는 과정에서 빈번하게 일어나는 배열 변경을 가장 우아하고 안전하게 동기화하기 위한 선택입니다.

### **2\. Template 매핑을 활용한 RecipeLogicModule 설계**

* **구현 방법**: URecipeLogicModuleBase를 비롯한 레시피 시스템은 C++의 템플릿(Template) 방식을 사용하여 다양한 구조체(FRecipeUIData를 상속/포함하는 임의의 레시피 데이터 타입)를 동적으로 처리하도록 설계되었습니다.  
* **도입 이유**:  
  1. 레시피는 '조합(Combine)', '분해(Deconstruct)', '요리(Cook)' 등 모듈의 목적에 따라 각기 다른 데이터 필드(Prioritiy, Input, Output 등)를 요구합니다.  
  2. 단일 구조체나 UObject 상속으로 모든 레시피 데이터를 묶으면 오버헤드가 커지거나 다운캐스팅(Downcasting) 지옥에 빠지게 됩니다.  
  3. 템플릿을 사용하여 로직 모듈이 자신이 다룰 \*\*구체적인 레시피 구조체 타입(T)\*\*을 컴파일 타임에 결정하도록 함으로써, 유연한 확장을 보장하면서도 타입 안정성(Type Safety)을 유지했습니다.

### **3\. 지연 평가(Lazy Validating)를 통한 유령 동기화(Phantom Snapping) 해결**

* **구현 방법**: 컴포넌트이나 로직 매 틱마다 상태를 갱신하는 대신, Logic\_CarryInteract\_Common 등이 블랙보드 캐시를 "사용하려고 시도할 때(OnModuleInteract 시점)" 유효성을 1회 검증합니다.  
* **도입 이유**:  
  1. 누군가가 워크스테이션에 거치된 아이템을 직접(CarryableComponent를 통해) 낚아채갈 경우, 워크스테이션의 블랙보드는 아이템이 사라졌다는 사실을 즉각 알 방법이 없습니다(상호 의존성을 없앴기 때문).  
  2. IsValid(Item) 체크와 더불어 TargetActor-\>IsAttachedTo(Workstation) 체크를 수행하여, 만약 아이템이 물리적으로 분리되었다면 즉각 블랙보드 슬롯을 초기화합니다.  
  3. 상태 감시(Polling) 비용을 아끼고, "필요할 때만 묻는다"는 의존성 역전 원칙을 지키기 위한 최적화입니다.