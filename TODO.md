# TODO

* How to update instance types (not pointer types) using QUaView::setColumnEditor?

Maybe change:

```c+++
T QUaModel::nodeFromIndex(const QModelIndex& index) const;
// for
T& QUaModel::nodeFromIndex(const QModelIndex& index);

// and of course
T QUaNodeWrapper::node() const;
// for
T& QUaNodeWrapper::node();
```

This way `QUaView::m_mapEditorFuncs` can accept modifiable references.

But then we would have to write specializations for pointer types (using `std::is_pointer`) as in `QUaModelItemTraits`.