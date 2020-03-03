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
	static QList<T> GetChildren(T& n)
	{
		Q_UNUSED(n);
		return QList<T>();
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
	static QList<T> GetChildren(T& n)
	{
		return GetChildrenTrait(n);
	}
};

#endif // QUAHASMODELITEMTRAITS_H