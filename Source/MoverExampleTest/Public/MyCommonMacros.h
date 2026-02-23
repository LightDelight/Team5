#pragma once

#ifdef PRINT
#	error MY_PRINT already defined
#else
#	if UE_BUILD_DEVELOPMENT
#		define PRINT(...) check(GEngine); GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, FColor::Green,FString::Printf(__VA_ARGS__));
#	else 
#		define PRINT(...)
#	endif
#endif

#ifdef PRINT_FUNCTION_NAME
#	error PRINT_FUNCATION_NAME already defined
#else
#	if UE_BUILD_DEVELOPMENT
#		define PRINT_FUNCATION_NAME check(GEngine); GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, FColor::Orange,FString::Printf(TEXT("%hs"),__FUNCTION__);
#	else 
#		define PRINT_FUNCATION_NAME
#	endif
#endif

#ifdef LOG_FUNCTION_NAME
#	error LOG_FUNCTION_NAME already defined
#else
#	if UE_BUILD_DEVELOPMENT
#		define LOG_FUNCTION_NAME UE_LOG(LogTemp,Log,TEXT("%hs"),__FUNCTION__);
#	else
#		define LOG_FUNCTION_NAME
#	endif
#endif

#ifdef LOG_TEMP
#	error LOG_TEMP already defined
#else
#	if UE_BUILD_DEVELOPMENT
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
