// Parser-only Unreal Flecs delegate declaration.

#pragma once

namespace UE::FlecsLibrary
{
	struct FFlecsParserTypeRegisteredDelegate
	{
		template <typename... T>
		void Broadcast(T&&...)
		{
		}
	};

	inline FFlecsParserTypeRegisteredDelegate& GetTypeRegisteredDelegate()
	{
		static FFlecsParserTypeRegisteredDelegate Delegate;
		return Delegate;
	}
}
