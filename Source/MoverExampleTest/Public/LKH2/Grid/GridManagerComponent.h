// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LKH2/Grid/GridCell.h"
#include "GridManagerComponent.generated.h"

class AActor;
class UMapData;
class AWorkStationBase;

/**
 * 쿼터뷰 파티 게임용 2D 그리드 매니저.
 * GameState에 부착하여 워크스테이션의 그리드 위치를 관리하고,
 * WorkComponent 등 외부 시스템에 공간 쿼리 API를 제공합니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOVEREXAMPLETEST_API UGridManagerComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UGridManagerComponent();

  // ── 좌표 변환 ──

  /** 월드 좌표를 그리드 좌표로 변환 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  FIntPoint WorldToGrid(const FVector &WorldLocation) const;

  /** 그리드 좌표를 월드 중심점으로 변환 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  FVector GridToWorld(const FIntPoint &GridCoord) const;

  // ── 등록 / 해제 ──

  /** 액터를 지정 셀에 등록. 이미 점유 중이면 false 반환 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  bool RegisterActor(AActor *Actor, const FIntPoint &GridCoord);

  /** 월드 위치를 기반으로 자동 좌표 계산 후 등록 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  bool RegisterActorAtWorldLocation(AActor *Actor,
                                    const FVector &WorldLocation);

  /** 지정 셀에서 액터 해제 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  void UnregisterActor(const FIntPoint &GridCoord);

  // ── 쿼리 ──

  /** 특정 셀의 액터 조회 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  AActor *GetActorAt(const FIntPoint &GridCoord) const;

  /** 월드 위치에서 가장 가까운 셀의 액터 조회 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  AActor *GetActorAtWorldLocation(const FVector &WorldLocation) const;

  /**
   * 주어진 월드 위치로부터 맨해튼 거리 Radius 이내의 워크스테이션 목록 반환.
   * WorkComponent가 주변 상호작용 대상을 얻기 위해 사용합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  TArray<AActor *> GetNearbyActors(const FVector &WorldLocation,
                                   int32 Radius = 1) const;

  /** 그리드 좌표가 유효 범위 내인지 확인 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  bool IsValidCoord(const FIntPoint &GridCoord) const;

  /** 특정 셀이 비어있는지 확인 */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  bool IsCellEmpty(const FIntPoint &GridCoord) const;

  /** 셀 정보 조회 (읽기 전용) */
  UFUNCTION(BlueprintCallable, Category = "Grid")
  FGridCell GetCell(const FIntPoint &GridCoord) const;

  // ── 맵 베이킹 파이프라인 ──

  /**
   * 현재 레벨의 모든 AWorkStationBase를 스캔하여 MapData 에셋에 저장합니다.
   * BeginPlay에서 bBakeOnBeginPlay가 true면 자동 실행됩니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Grid|Baking")
  void BakeCurrentLevel();

  /**
   * MapData 에셋을 읽어 워크스테이션을 스폰하고 그리드에 등록합니다.
   * BeginPlay에서 MapData가 있으면 자동 호출됩니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Grid|Baking")
  void LoadFromMapData();

  /**
   * 스폰된 모든 워크스테이션을 제거하고 그리드를 초기화합니다.
   */
  UFUNCTION(BlueprintCallable, Category = "Grid|Baking")
  void ClearGrid();

  // ─── 런타임 워크스테이션 스폰 ───

  /**
   * 지정 그리드 좌표에 워크스테이션을 스폰하고 그리드에 등록합니다.
   * SpawnActorDeferred를 사용하여 BeginPlay 전에 WorkstationData를 설정합니다.
   *
   * @param StationClass 스폰할 워크스테이션 블루프린트 클래스
   * @param StationData  적용할 워크스테이션 데이터 에셋
   * @param GridCoord    배치할 그리드 좌표
   * @param Rotation     월드 회전값
   * @return 스폰된 AWorkStationBase* (실패 시 nullptr)
   */
  UFUNCTION(BlueprintCallable, Category = "Grid|Spawn")
  AWorkStationBase *SpawnWorkstation(TSubclassOf<AWorkStationBase> StationClass,
                                     UWorkstationData *StationData,
                                     const FIntPoint &GridCoord,
                                     const FRotator &Rotation = FRotator::ZeroRotator);

  /**
   * 지정 그리드 좌표의 워크스테이션을 파괴하고 그리드에서 해제합니다.
   *
   * @param GridCoord 제거할 워크스테이션의 그리드 좌표
   * @return 성공 시 true
   */
  UFUNCTION(BlueprintCallable, Category = "Grid|Spawn")
  bool DestroyWorkstation(const FIntPoint &GridCoord);

protected:
  virtual void BeginPlay() override;

  /** 그리드를 생성하여 셀 배열을 초기화합니다 */
  void InitializeGrid();

protected:
  /** 그리드 시작점 (월드 좌표, 좌하단 기준) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Setup")
  FVector GridOrigin = FVector::ZeroVector;

  /** 타일 한 변의 크기 (cm) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Setup",
            meta = (ClampMin = "10.0"))
  float CellSize = 200.0f;

  /** 그리드 가로 타일 수 (X축) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Setup",
            meta = (ClampMin = "1"))
  int32 GridWidth = 10;

  /** 그리드 세로 타일 수 (Y축) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Setup",
            meta = (ClampMin = "1"))
  int32 GridHeight = 10;

  /** 로드/베이크 대상 맵 데이터 에셋 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Baking")
  TObjectPtr<UMapData> MapData;

  /**
   * 체크하면 BeginPlay 시 레벨의 워크스테이션을 스캔하여 MapData에 저장합니다.
   * 베이크 완료 후 자동으로 false로 복원됩니다.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Baking")
  bool bBakeOnBeginPlay = false;

private:
  /** 그리드 셀 저장소: FIntPoint(X, Y) → FGridCell */
  UPROPERTY()
  TMap<FIntPoint, FGridCell> Cells;

  /** LoadFromMapData로 스폰된 액터들 (ClearGrid에서 정리용) */
  UPROPERTY()
  TArray<TObjectPtr<AActor>> SpawnedActors;
};
