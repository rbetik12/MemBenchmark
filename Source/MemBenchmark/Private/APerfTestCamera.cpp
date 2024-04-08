#include "APerfTestCamera.h"
#include "HAL/MemoryMisc.h"

void APerfTestCamera::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &APerfTestCamera::CollectFrameTimes, 0.5f, true);
}

void APerfTestCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(timerHandle);
}

UE_DISABLE_OPTIMIZATION
void APerfTestCamera::CollectFrameTimes()
{
	if (const auto world = GetWorld())
	{
		if (const auto viewport = world->GetGameViewport())
		{
			PerfStats stats;

			float DiffTime;
			if (FApp::IsBenchmarking() || FApp::UseFixedTimeStep())
			{
				/** If we're in fixed time step mode, FApp::GetCurrentTime() will be incorrect for benchmarking */
				const double CurrentTime = FPlatformTime::Seconds();
				if (LastTime == 0)
				{
					LastTime = CurrentTime;
				}
				DiffTime = CurrentTime - LastTime;
				LastTime = CurrentTime;
			}
			else
			{
				/** Use the DiffTime we computed last frame, because it correctly handles the end of frame idling and corresponds better to the other unit times. */
				DiffTime = FApp::GetCurrentTime() - FApp::GetLastTime();
			}

			stats.cpuTime = DiffTime * 1000.0f;
			stats.gpuTime = FPlatformTime::ToMilliseconds(GDynamicRHI->RHIGetGPUFrameCycles(0));
			stats.gameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
			stats.renderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
			stats.rhiTime = FPlatformTime::ToMilliseconds(GRHIThreadTime);

			FGenericMemoryStats memStats;
			GMalloc->GetAllocatorStats(memStats);
			const auto name = GMalloc->GetDescriptiveName();

			UE_LOG(LogEngine, Display, TEXT("CPU Time: %.2f"), stats.cpuTime);
			UE_LOG(LogEngine, Display, TEXT("GPU Time: %.2f"), stats.gpuTime);

			int* kek = new int;

			statsStorage.Add(std::move(stats));
		}
	}
}
UE_ENABLE_OPTIMIZATION

