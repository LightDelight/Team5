#pragma once

#if defined UE_BUILD_DEVELOPMENT && UE_BUILD_DEVELOPMENT == 1
#	define MY_PRINT(Msg) if(GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, FColor::Green,Msg);\
					  else UE_LOG(LogTemp,Log,TEXT("GEngine is nullptr"));;
#endif
//error should occur when shipping build. im retaining this to check flag...

#ifdef LOG_FUNCTION_NAME
#	error LOG_FUNCTION_NAME already defined
#else
#	if defined UE_BUILD_DEVELOPMENT && UE_BUILD_DEVELOPMENT == 1
#		define LOG_FUNCTION_NAME UE_LOG(LogTemp,Log,TEXT("%hs"),__FUNCTION__);
#	else
#		define LOG_FUNCTION_NAME
#	endif
#endif

#ifdef LOG_TEMP
#	error LOG_TEMP already defined
#else
#	if defined UE_BUILD_DEVELOPMENT && UE_BUILD_DEVELOPMENT == 1
#		define LOG_TEMP(...) UE_LOG(LogTemp,Log,__VA_ARGS__);
#	else
#		define LOG_TEMP(...)
#	endif
#endif


//DECLARE_LOG_CATEGORY_EXTERN(LogOnlineSubsystem, Log, All)
//DEFINE_LOG_CATEGORY(LogOnlineSubsystem)

/**
 * 
 */
