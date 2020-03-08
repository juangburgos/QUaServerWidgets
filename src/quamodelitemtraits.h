#ifndef QUAHASMODELITEMTRAITS_H
#define QUAHASMODELITEMTRAITS_H

#include <QList>

// trait to activate specialized implementation on a given type T
template<typename T>
struct QUaHasModelItemTraits
{
	static const bool value = false;
};
// default implementation
template<bool b>
struct QUaModelItemTraits
{
	// default a way to notify when T is about to be destroyed (e.g. QObject::destroyed signal)
	template<typename T>
	static QMetaObject::Connection DestroyCallback(T& n, std::function<void(void)> callback)
	{
		Q_UNUSED(n);
		Q_UNUSED(callback);
		return QMetaObject::Connection();
	}
	// default a way to return a list of children of T
	template<typename T>
	static QList<T> GetChildren(const T& n)
	{
		Q_UNUSED(n);
		return QList<T>();
	}
	// default a way to check if T is valid
	template<typename T>
	static bool IsValid(const T& n)
	{
		return n;
	}
	// default a way to construct a T from a bool
	template<typename T>
	static T GetInvalid()
	{
		return nullptr;
	}
	// default a way to check if two Ts are equal
	template<typename T>
	static bool IsEqual(const T& n1, const T& n2)
	{
		return n1 == n2;
	}
};
// what is expected of a type T that implements a specialized implementation
template<>
struct QUaModelItemTraits<true>
{
	// call specialized way to notify when T is about to be destroyed
	template<typename T>
	static QMetaObject::Connection DestroyCallback(T& n, std::function<void(void)> callback)
	{
		return DestroyCallbackTrait(n, callback);
	}
	// call specialized way to return a list of children of T
	template<typename T>
	static QList<T> GetChildren(const T& n)
	{
		return GetChildrenTrait(n);
	}
	// call specialized way to check if T is valid
	template<typename T>
	static bool IsValid(const T& n)
	{
		return IsValidTrait(n);
	}
	// call specialized way to construct a T from a bool
	template<typename T>
	static T GetInvalid()
	{
		return GetInvalidTrait();
	}
	// default a way to check if two Ts are equal
	template<typename T>
	static bool IsEqual(const T& n1, const T& n2)
	{
		return IsEqualTrait(n1, n2);
	}
};

#endif // QUAHASMODELITEMTRAITS_H