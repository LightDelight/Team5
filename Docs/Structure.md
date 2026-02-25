

InteractorComponent
플레이어에게 부착되어 Carry 관련 상호작용을 담당하는 컴포넌트


InteractionInterface
InteractorComponent가 대상 Actor에게 Context와 함께 상호작용 요청 메시지를 보내는 인터페이스
Actor는 InteractionInterface를 상속받아 자신의 컴포넌트들에게 상호작용 요청을 전달한다.
아웃라인 온오프 요청 전달도 포함한다. (현재 아이템과 작업대의 아웃라인 구현부가 다름)


InteractableComponent
InteractorComponent가 상호작용을 요청하는 대상 Actor에게 부착되는 컴포넌트


