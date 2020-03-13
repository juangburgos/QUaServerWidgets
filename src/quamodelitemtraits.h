#ifndef QUAHASMODELITEMTRAITS_H
#define QUAHASMODELITEMTRAITS_H

#include <QList>
#include <type_traits>

// NOTE : specialize these static methods for specialized implementation for type T
struct QUaModelItemTraits
{
public:
	// notify when T is about to be destroyed by calling *callback* (e.g. QObject::destroyed signal)
	// default implementation if T is type
	template<typename T, typename M1 = const std::function<void(void)>&> static
	typename std::enable_if<!std::is_pointer<T>::value, QMetaObject::Connection>::type
	DestroyCallback(T* n, M1 &&callback)
	{
		Q_UNUSED(n);
		Q_UNUSED(callback);
		return QMetaObject::Connection();
	}
	// default implementation if T is pointer
	template<typename T, typename M1 = const std::function<void(void)>&> static
	typename std::enable_if<std::is_pointer<T>::value, QMetaObject::Connection>::type
	DestroyCallback(T n, M1 &&callback)
	{
		Q_UNUSED(n);
		Q_UNUSED(callback);
		return QMetaObject::Connection();
	}
	// notify when T just got a new child by calling *callback* (e.g. QUaNode::childAdded signal)
	// default implementation if T is type
	template<typename T, typename M1 = const std::function<void(T&)>&> static
	typename std::enable_if<!std::is_pointer<T>::value, QMetaObject::Connection>::type
	NewChildCallback(T* n, M1 &&callback)
	{
		Q_UNUSED(n);
		Q_UNUSED(callback);
		return QMetaObject::Connection();
	}
	// default implementation if T is pointer
	template<typename T, typename M1 = const std::function<void(T)>&> static
	typename std::enable_if<std::is_pointer<T>::value, QMetaObject::Connection>::type
	NewChildCallback(T n, M1 &&callback)
	{
		Q_UNUSED(n);
		Q_UNUSED(callback);
		return QMetaObject::Connection();
	}
	// return a list of children of T
	// default implementation if T is type
	template<typename T> static
	typename std::enable_if<!std::is_pointer<T>::value, QList<T>>::type
	GetChildren(const T* n)
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
	IsValid(const T* n)
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
	IsEqual(const T* n1, const T* n2)
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
	// set data into T (reference)
	// default implementation if T is type
	template<typename T> static
	typename std::enable_if<!std::is_pointer<T>::value, bool>::type
	SetData(T* n, const int &column, const QVariant& value)
	{
		Q_UNUSED(n);
		Q_UNUSED(column);
		Q_UNUSED(value);
		return false;
	}
	// default implementation if T is pointer
	template<typename T> static
	typename std::enable_if<std::is_pointer<T>::value, bool>::type
	SetData(T n, const int& column, const QVariant& value)
	{
		Q_UNUSED(n);
		Q_UNUSED(column);
		Q_UNUSED(value);
		return false;
	}
};

#endif // QUAHASMODELITEMTRAITS_H
