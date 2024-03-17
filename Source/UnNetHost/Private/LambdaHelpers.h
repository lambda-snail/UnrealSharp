#pragma once

/**
 * Helper structures and functions to allow conversion of lambda with captures to a function pointer.
 * Based on Evgeny Karpov's reply to this post: https://stackoverflow.com/questions/7852101/c-lambda-with-captures-as-a-function-pointer
 */
namespace LambdaSnail::UnrealSharp::Lambda
{
	template <class TClass>
	struct TLambdaTraits : TLambdaTraits<decltype(&TClass::operator())>
	{
	};

	template <typename TLambda, typename TReturn, typename... TArgs>
	struct TLambdaTraits<TReturn(TLambda::*)(TArgs...)> : TLambdaTraits<TReturn(TLambda::*)(TArgs...) const>
	{
	};

	template <class TLambda, class TReturn, class... TArgs>
	struct TLambdaTraits<TReturn(TLambda::*)(TArgs...) const>
	{
		using Pointer = std::add_pointer_t<TReturn(TArgs...)>;

		static Pointer const ToFunctionPointer(TLambda&& InFunction)
		{
			static TLambda Function = std::forward<TLambda>(InFunction);
			return [](TArgs... Args)
			{
				return Function(std::forward<TArgs>(Args)...);
			};
		}
	};

	template <class TLambda>
	typename TLambdaTraits<TLambda>::Pointer ToFunctionPointer(TLambda&& Function)
	{
		return TLambdaTraits<TLambda>::ToFunctionPointer(std::forward<TLambda>(Function));
	}
}
