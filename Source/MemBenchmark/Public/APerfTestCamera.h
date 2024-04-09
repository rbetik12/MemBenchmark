#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
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

	void HandleAuthRequest(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleNewRunRequest(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

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
	FString	RunId;
};
