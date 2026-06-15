// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/Base/K2Node_FlecsGenericEntityOperation.h"

#include "EdGraphSchema_K2.h"

#include "Entities/FlecsEntityHandle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsGenericEntityOperation)

void UK2Node_FlecsGenericEntityOperation::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, FName(TEXT("Execute")));
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, FName(TEXT("Then")));
	
	UEdGraphNode::FCreatePinParams EntityPinParams;
	EntityPinParams.bIsConst = true;
	EntityPinParams.bIsReference = true;
	
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, 
		FFlecsEntityHandle::StaticStruct(), FName(TEXT("Entity")), EntityPinParams);
}

UEdGraphPin* UK2Node_FlecsGenericEntityOperation::GetEntityPin() const
{
	return FindPin(FName(TEXT("Entity")), EEdGraphPinDirection::EGPD_Input);
}
