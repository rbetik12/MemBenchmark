#include "APerfTestCamera.h"
#include "HttpModule.h"
#include "HttpManager.h"
#include "FRunData.h"
#include "HAL/MemoryMisc.h"

UE_DISABLE_OPTIMIZATION
void APerfTestCamera::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &APerfTestCamera::CollectFrameTimes, 0.5f, true);

	FHttpModule& HttpModule = FHttpModule::Get();

	{
		auto Request = HttpModule.CreateRequest();
		Request->SetURL(TEXT("http://localhost:3000/api/auth"));
		Request->SetVerb(TEXT("POST"));
		Request->OnProcessRequestComplete().BindUObject(this, &APerfTestCamera::HandleAuthRequest);
		Request->ProcessRequest();
	}
}

void APerfTestCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(timerHandle);
}

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

			//statsStorage.Add(std::move(stats));

			FRunData runData;
			runData.cpuTime = stats.cpuTime;
			runData.gpuTime = stats.gpuTime;
			runData.fps = 1.0f / FApp::GetDeltaTime();
			runData.memAmount = memStats.Data[TEXT("MemAmount")];
			runData.memopsAmount = memStats.Data[TEXT("Alloc")] + memStats.Data[TEXT("ReAlloc")] + memStats.Data[TEXT("Free")];

			{
				FHttpModule& HttpModule = FHttpModule::Get();
				auto Request = HttpModule.CreateRequest();
				Request->SetURL(FString::Printf(TEXT("http://localhost:3000/api/run/%s/data"), *RunId));
				Request->SetVerb(TEXT("POST"));
				Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
				Request->SetContentAsString(runData.ToJSON());
				Request->OnProcessRequestComplete().BindUObject(this, &APerfTestCamera::HandleRunDataRequest);
				Request->ProcessRequest();
			}
		}
	}
}

void APerfTestCamera::HandleAuthRequest(FHttpRequestPtr _, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogActor, Error, TEXT("Auth request failed"));
		return;
	}

	TArray<FString> DefaultCookies;

	for (auto& header : Response->GetAllHeaders())
	{
		if (header.StartsWith("Set-Cookie"))
		{
			header.RightChopInline(12);
			TArray<FString> Cookie;
			header.ParseIntoArray(Cookie, TEXT(";"));

			DefaultCookies.Add(std::move(Cookie[0]));
		}
	}

	FHttpModule& HttpModule = FHttpModule::Get();

	FString CookieHeader;

	for (const auto& cookie : DefaultCookies)
	{
		if (DefaultCookies.Num() > 1)
		{
			CookieHeader += cookie + TEXT("; ");
		}
		else
		{
			CookieHeader += cookie;
		}
	}

	HttpModule.AddDefaultHeader("Cookie", CookieHeader);

	auto Request = HttpModule.CreateRequest();
	Request->SetURL(TEXT("http://localhost:3000/api/run/new"));
	Request->SetVerb(TEXT("POST"));
	Request->OnProcessRequestComplete().BindUObject(this, &APerfTestCamera::HandleNewRunRequest);
	Request->ProcessRequest();
}

void APerfTestCamera::HandleNewRunRequest(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response->GetContentType().StartsWith("application/json"))
	{
		UE_LOG(LogActor, Error, TEXT("New run request failed"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());

	FJsonSerializer::Deserialize(JsonReader, JsonObject);
	RunId = JsonObject->GetStringField("id");
}

void APerfTestCamera::HandleRunDataRequest(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
}

UE_ENABLE_OPTIMIZATION

