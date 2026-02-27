#include "LKH2/Interaction/Base/LogicModuleBase.h"

bool ULogicModuleBase::ExecuteInteraction(const FInteractionContext &Context) {
  // 1. 사전 검사 (Pre-Logic)
  if (!PreInteractCheck(Context)) {
    // 실패 시 후처리 호출 후 조기 반환 (성공여부 false)
    PostInteract(Context, false);
    return false;
  }

  // 2. 핵심 실행 (Core-Logic)
  bool bSuccess = PerformInteraction(Context);

  // 3. 사후 처리 (Post-Logic)
  PostInteract(Context, bSuccess);

  return bSuccess;
}
