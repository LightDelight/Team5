#pragma once

#define MY_PRT(Msg) if(GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.0f, FColor::Green,Msg);\
					  else UE_LOG(LogTemp,Log,TEXT("GEngine is nullptr"));;

/**
 * 
 */
