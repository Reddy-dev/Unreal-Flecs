// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

template <typename T>
struct TFlecsHandleTraits;
	
template <typename T>
using TFlecsHandleSelfTrait = TFlecsHandleTraits<T>::SelfType;
	
template <typename T>
using TFlecsHandleViewTrait = TFlecsHandleTraits<T>::ViewType;
	
template <typename T>
using TFlecsHandleMutableTrait = TFlecsHandleTraits<T>::MutableType;