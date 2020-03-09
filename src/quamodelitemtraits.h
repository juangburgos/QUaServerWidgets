#ifndef QUAHASMODELITEMTRAITS_H
#define QUAHASMODELITEMTRAITS_H

#include <QList>
#include <type_traits>

// NOTE : specialize these static methods for specialized implementation for type T
struct QUaModelItemTraits
{
public:
	// call callback to notify when T is about to be destroyed (e.g. QObject::destroyed signal)
	// default implementation if T is type
	template<typename T> static
	typename std::enable_if<!std::is_pointer<T>::value, QMetaObject::Connection>::type
	DestroyCallback(T& n, std::function<void(void)> callback)
	{
		Q_UNUSED(n);
		Q_UNUSED(callback);
		return QMetaObject::Connection();
	}
	// default implementation if T is pointer
	template<typename T> static
	typename std::enable_if<std::is_pointer<T>::value, QMetaObject::Connection>::type
	DestroyCallback(T n, std::function<void(void)> callback)
	{
		Q_UNUSED(n);
		Q_UNUSED(callback);
		return QMetaObject::Connection();
	}
	// return a list of children of T
	// default implementation if T is type
	template<typename T> static
	typename std::enable_if<!std::is_pointer<T>::value, QList<T>>::type
	GetChildren(const T& n)
	{
		Q_UNUSED(n);
		return QList<T>();
	}
	// default implementation if T is pointer
	template<typename T> static
	typename std::enable_if<std::is_pointer<T>::value, QList<T>>::type
	GetChildren(T n)
	{
		Q_UNUSED(n);
		return QList<T>();
	}
	// return true if T is valid, else return false
	// default implementation if T is type
	template<typename T> static
	typename std::enable_if<!std::is_pointer<T>::value, bool>::type
	IsValid(const T& n)
	{
		return n;
	}
	// default implementation if T is pointer
	template<typename T> static
	typename std::enable_if<std::is_pointer<T>::value, bool>::type
	IsValid(const T n)
	{
		return n;
	}
	// construct an invalid T (one that fails the above IsValid trait)
	// default implementation if T is type or pointer
	template<typename T> static 
	T GetInvalid()
	{
		return nullptr;
	}
	// return true if two Ts are equal, else return false
	// default implementation if T is type
	template<typename T> static
	typename std::enable_if<!std::is_pointer<T>::value, bool>::type
	IsEqual(const T& n1, const T& n2)
	{
		return n1 == n2;
	}
	// default implementation if T is pointer
	template<typename T> static
	typename std::enable_if<std::is_pointer<T>::value, bool>::type
	IsEqual(const T n1, const T n2)
	{
		return n1 == n2;
	}
};

#endif // QUAHASMODELITEMTRAITS_H
