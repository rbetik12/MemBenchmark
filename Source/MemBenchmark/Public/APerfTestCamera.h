#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "APerfTestCamera.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MEMBENCHMARK_API APerfTestCamera : public ASpectatorPawn
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasFinishedRoute = false;

private:
	void CollectFrameTimes();

	void HandleAuthRequest(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleNewRunRequest(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleRunDataRequest(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	struct PerfStats
	{
		float cpuTime;
		float gpuTime;
		float gameThreadTime;
		float renderThreadTime;
		float rhiTime;
	};


	FTimerHandle timerHandle;
	double LastTime;
	FString	RunId;
};
