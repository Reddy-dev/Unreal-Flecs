// Elie Wiese-Namir © 2025. All Rights Reserved.

template <>
struct TFlecsHandleTraits<FFlecsCommonHandle>
{
	using SelfType = FFlecsCommonHandle;
	using ViewType = FFlecsEntityView;
	using MutableType  = FFlecsEntityHandle;
}; // struct TFlecsHandleTraits<FFlecsCommonHandle>

template <> 
struct TFlecsHandleTraits<FFlecsEntityView>
{
	using SelfType = FFlecsEntityView;
	using ViewType = FFlecsEntityView;
	using MutableType  = FFlecsEntityHandle;
}; // struct TFlecsHandleTraits<FFlecsEntityView>

template <> 
struct TFlecsHandleTraits<FFlecsEntityHandle>
{
	using SelfType = FFlecsEntityHandle;
	using ViewType = FFlecsEntityView;
	using MutableType  = FFlecsEntityHandle;
}; // struct TFlecsHandleTraits<FFlecsEntityHandle>