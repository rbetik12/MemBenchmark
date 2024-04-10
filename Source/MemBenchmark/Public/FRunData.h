#pragma once

#include "Containers/UnrealString.h"
#include "Sockets.h"
#include "FRunData.generated.h"

USTRUCT()
struct FRunData
{
	GENERATED_BODY()

	UPROPERTY()
	float fps;

	UPROPERTY()
	float cpuTime;

	UPROPERTY()
	float gpuTime;

	UPROPERTY()
	int64 memopsAmount;

	UPROPERTY()
	int64 memAmount;

	FString ToJSON() const;
};