#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "APerfTestCamera.generated.h"

/**
 * 
 */
UCLASS()
class MEMBENCHMARK_API APerfTestCamera : public ASpectatorPawn
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void CollectFrameTimes();

	struct PerfStats
	{
		float cpuTime;
		float gpuTime;
		float gameThreadTime;
		float renderThreadTime;
		float rhiTime;
	};

	TArray<PerfStats> statsStorage;
	FTimerHandle timerHandle;
	double LastTime;
};
