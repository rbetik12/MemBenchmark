#include "FRunData.h"
#include "JsonObjectConverter.h"

FString FRunData::ToJSON() const
{
	FString payload;
	FJsonObjectConverter::UStructToJsonObjectString(*this, payload, 0, 0);
	return payload;
}
