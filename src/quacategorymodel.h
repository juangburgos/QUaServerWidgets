#ifndef QUACATEGORYMODEL_H
#define QUACATEGORYMODEL_H

#include <QUaTreeModel>
#include <QUaModelItemTraits>

// NOTE : need to reimplement model methods instaed of using a special type T because:
// 1) That type T would have to be templated T<N> to support other types and
//    writting traits for it requires constructs that C++ does not support
// 2) The respective QUaView<N> requires the same N as the model, so user would have
//    to work with T and the API would not be properly abstracted
template <typename N, int I = 0>
class QUaCategoryModel : public QUaTreeModel<N>
{
public:

    explicit QUaCategoryModel(QObject* parent = nullptr);
    ~QUaCategoryModel();

    void addCategory(const QString& strCategory);

    void removeCategory(const QString& strCategory);

    bool hasCategory(const QString& strCategory) const;

    QStringList categories() const;

    void addNodeToCategory(const QString& strCategory, N node);

    template<typename X = N>
    typename std::enable_if<std::is_pointer<X>::value, QString>::type
    nodeCategory(N node);

    template<typename X = N>
    typename std::enable_if<!std::is_pointer<X>::value, QString>::type
    nodeCategory(N* node);

    template<typename X = N>
    typename std::enable_if<std::is_pointer<X>::value, QList<N>>::type
    nodesByCategory(const QString& strCategory);

    template<typename X = N>
    typename std::enable_if<!std::is_pointer<X>::value, QList<N*>>::type
    nodesByCategory(const QString& strCategory);

    template<typename X = N>
    typename std::enable_if<std::is_pointer<X>::value, bool>::type
    removeNode(N node);

    template<typename X = N>
    typename std::enable_if<!std::is_pointer<X>::value, bool>::type
    removeNode(N* node);

    QStringList indexesToCategories(const QModelIndexList& indexes) const;

    void clear();

    // Qt required API:

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    QHash<
        typename QUaModel<N, I>::QUaNodeWrapper*,
        QString
    > m_hashCategories;

    typename QUaModel<N, I>::QUaNodeWrapper* addCategoryInternal(const QString& strCategory);

    typename QUaModel<N, I>::QUaNodeWrapper* getCategoryInternal(const QString& strCategory);
};

template<typename N, int I>
inline QUaCategoryModel<N, I>::QUaCategoryModel(QObject* parent) :
    QUaTreeModel<N, I>(parent)
{
    QUaModel<N, I>::m_root = new typename QUaModel<N, I>::QUaNodeWrapper(
        QUaModelItemTraits::GetInvalid<N, I>()
    );
}

template<typename N, int I>
inline QUaCategoryModel<N, I>::~QUaCategoryModel()
{
    if (QUaModel<N, I>::m_root)
    {
        delete QUaModel<N, I>::m_root;
        QUaModel<N, I>::m_root = nullptr;
    }
}

template<typename N, int I>
inline void QUaCategoryModel<N, I>::addCategory(const QString& strCategory)
{
    this->addCategoryInternal(strCategory);
}

template<typename N, int I>
inline void QUaCategoryModel<N, I>::removeCategory(const QString& strCategory)
{
    auto root = QUaModel<N, I>::m_root;
    Q_CHECK_PTR(root);
    // ignore unexisting category
    auto category = m_hashCategories.key(strCategory, nullptr);
    if (!category)
    {
        return;
    }
    int idx = root->children().indexOf(category);
    Q_ASSERT(idx >= 0);
    if (idx < 0)
    {
        return;
    }
    auto wrapper = root->children().at(idx);
    // use internal method (deletes wrapper)
    this->QUaModel<N, I>::removeWrapper(wrapper);
    // remove from hash before deleting wrapper
    Q_ASSERT(m_hashCategories.contains(wrapper));
    m_hashCategories.remove(wrapper);
}

template<typename N, int I>
inline bool QUaCategoryModel<N, I>::hasCategory(const QString& strCategory) const
{
    return this->getCategoryInternal(strCategory);
}

template<typename N, int I>
inline QStringList QUaCategoryModel<N, I>::categories() const
{
    return m_hashCategories.values();
}

template<typename N, int I>
inline void QUaCategoryModel<N, I>::addNodeToCategory(const QString& strCategory, N node)
{
    // add category if not exists
    auto category = this->addCategoryInternal(strCategory);
    Q_CHECK_PTR(category);
    if (!category) { return; }
    QModelIndex index = category->index();
    // get new child's row
    int row = category->children().count();
    // notify views that row will be added
    this->beginInsertRows(index, row, row);
    // create new wrapper
    auto wrapper = new typename QUaModel<N, I>::QUaNodeWrapper(
        node,
        category,
        false // NOTE : not recursive
    );
    // apprend to parent's children list
    category->children() << wrapper;
    // notify views that row addition has finished
    this->endInsertRows();
    // force index creation (indirectly)
    // because sometimes they are not created until a view requires them
    // and if a child is added and parent's index is not ready then crash
    bool indexOk = this->checkIndex(this->index(row, 0, index), QAbstractItemModel::CheckIndexOption::IndexIsValid);
    Q_ASSERT(indexOk);
    Q_UNUSED(indexOk);
    // bind callback for data change on each column
    this->bindChangeCallbackForAllColumns(wrapper, false);
    // subscribe to instance removed
    auto conn = QUaModelItemTraits::DestroyCallback<N, I>(wrapper->node(),
        [this, wrapper]() {
            Q_CHECK_PTR(wrapper);
            auto root = QUaModel<N, I>::m_root;
            Q_CHECK_PTR(root);
            Q_UNUSED(root);
            // remove
            this->removeWrapper(wrapper);
        }
    );
    if (conn)
    {
        // NOTE : QUaNodeWrapper destructor removes connections
        wrapper->connections() << conn;
    }
}

template<typename N, int I>
template<typename X>
inline 
typename std::enable_if<std::is_pointer<X>::value, QString>::type 
QUaCategoryModel<N, I>::nodeCategory(N node)
{
    for (auto strCategory : this->categories())
    {
        auto nodes = this->nodesByCategory(strCategory);
        if (nodes.contains(node))
        {
            return strCategory;
        }
    }
    return QString();
}

template<typename N, int I>
template<typename X>
inline 
typename std::enable_if<!std::is_pointer<X>::value, QString>::type 
QUaCategoryModel<N, I>::nodeCategory(N* node)
{
    for (auto strCategory : this->categories())
    {
        auto nodes = this->nodesByCategory(strCategory);
        if (nodes.contains(node))
        {
            return strCategory;
        }
    }
    return QString();
}

template<typename N, int I>
template<typename X>
inline 
typename std::enable_if<std::is_pointer<X>::value, QList<N>>::type 
QUaCategoryModel<N, I>::nodesByCategory(const QString& strCategory)
{
    QList<N> nodes;
    auto category = getCategoryInternal(strCategory);
    if (!category)
    {
        return nodes;
    }
    for (auto wrapper : category->children())
    {
        nodes << wrapper->node();
    }
    return nodes;
}

template<typename N, int I>
template<typename X>
inline
typename std::enable_if<!std::is_pointer<X>::value, QList<N*>>::type
QUaCategoryModel<N, I>::nodesByCategory(const QString& strCategory)
{
    QList<N*> nodes;
    auto category = getCategoryInternal(strCategory);
    if (!category)
    {
        return nodes;
    }
    for (auto wrapper : category->children())
    {
        nodes << wrapper->node();
    }
    return nodes;
}

template<typename N, int I>
template<typename X>
inline
typename std::enable_if<std::is_pointer<X>::value, bool>::type
QUaCategoryModel<N, I>::removeNode(N node)
{
    bool res = false;
    // look for node in all categories
    auto& categories = QUaModel<N, I>::m_root->children();
    for (auto category : categories)
    {
        auto wrapper = category->childByNode(node);
        if (!wrapper)
        {
            continue;
        }
        this->removeWrapper(wrapper);
        res = true;
    }
    return res;
}

template<typename N, int I>
template<typename X>
inline 
typename std::enable_if<!std::is_pointer<X>::value, bool>::type 
QUaCategoryModel<N, I>::removeNode(N* node)
{
    bool res = false;
    // look for node in all categories
    auto& categories = QUaModel<N, I>::m_root->children();
    for (auto category : categories)
    {
        auto wrapper = category->childByNode(node);
        if (!wrapper)
        {
            continue;
        }
        this->removeWrapper(wrapper);
        res = true;
    }
    return res;
}

template<typename N, int I>
inline QStringList QUaCategoryModel<N, I>::indexesToCategories(
        const QModelIndexList& indexes
    ) const
{
    QStringList categories;
    for (auto index : indexes)
    {
        // ignore root
        if (!this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid))
        {
            continue;
        }
        // ignore nodes
        auto wrapper = static_cast<typename QUaCategoryModel<N, I>::QUaNodeWrapper*>(index.internalPointer());
        Q_CHECK_PTR(wrapper);
        if (QUaModelItemTraits::IsValid<N, I>(wrapper->node()))
        {
            continue;
        }
        // must be category
        Q_ASSERT(m_hashCategories.contains(wrapper));
        // ignore repeated (selected indexes include all cols of same row)
        if (categories.contains(m_hashCategories[wrapper]))
        {
            continue;
        }
        categories << m_hashCategories[wrapper];
    }
    return categories;
}

template<typename N, int I>
inline QVariant QUaCategoryModel<N, I>::data(
    const QModelIndex& index, 
    int role) const
{
    // early exit for inhandled cases
    if (!QUaModel<N, I>::m_root || !index.isValid())
    {
        return QVariant();
    }
    // get internal reference
    auto wrapper = static_cast<typename QUaModel<N, I>::QUaNodeWrapper*>(index.internalPointer());
    // NOTE : do not ignore invalid
    auto &categories = QUaModel<N, I>::m_root->children();
    auto res = std::find_if(categories.begin(), categories.end(),
    [wrapper](typename QUaModel<N, I>::QUaNodeWrapper * category) {
        return wrapper == category;
    });
    auto category = (res == categories.end()) ? nullptr : *res;
    // return category name if category
    if (category)
    {
        if (index.column() == 0 && role == Qt::DisplayRole)
        {
            Q_ASSERT(m_hashCategories.contains(category));
            return m_hashCategories[category];
        }
        return QVariant();
    }
    // default implementation if no ColumnDataSource has been defined
    if (QUaModelBase<N, I>::m_mapDataSourceFuncs.isEmpty())
    {
        using QUaModel = QUaModel<N, I>;
        Q_ASSERT(QUaModel::m_columnCount == 1);
        return QObject::tr("");
    }
    // empty if no ColumnDataSource defined for this column
    if (!QUaModelBase<N, I>::m_mapDataSourceFuncs.contains(index.column()) ||
        !QUaModelBase<N, I>::m_mapDataSourceFuncs[index.column()].m_dataCallback)
    {
        return QVariant();
    }
    // use user-defined ColumnDataSource
    return QUaModelBase<N, I>::m_mapDataSourceFuncs[index.column()].m_dataCallback(
        wrapper->node(),
        static_cast<Qt::ItemDataRole>(role)
    );
}

template<typename N, int I>
inline typename QUaModel<N, I>::QUaNodeWrapper*
QUaCategoryModel<N, I>::addCategoryInternal(const QString& strCategory)
{
    auto root = QUaModel<N, I>::m_root;
    Q_CHECK_PTR(root);
    // ignore existing category
    auto wrapper = this->getCategoryInternal(strCategory);
    if (wrapper)
    {
        return wrapper;
    }
    QModelIndex index = root->index();
    // get new child's row
    int row = root->children().count();
    // notify views that row will be added
    this->beginInsertRows(index, row, row);
    // create new wrapper
    // NOTE : is invalid, we use the wrapper pointer as a key to the categories hash
    wrapper = new typename QUaModel<N, I>::QUaNodeWrapper(
        QUaModelItemTraits::GetInvalid<N, I>(),
        root,
        false // NOTE : not recursive
    );
    // apprend to parent's children list
    root->children() << wrapper;
    // bind to string
    m_hashCategories[wrapper] = strCategory;
    // notify views that row addition has finished
    this->endInsertRows();
    // NOTE : do not need to subscribe to Datachange or DestroyCallback 
    //        because categories always have invalid nodes and are
    //        removed manually (in)directly by ::removeCategory
    return wrapper;
}

template<typename N, int I>
inline typename QUaModel<N, I>::QUaNodeWrapper*
QUaCategoryModel<N, I>::getCategoryInternal(const QString& strCategory)
{
    return m_hashCategories.key(strCategory, nullptr);
}

template<typename N, int I>
inline void QUaCategoryModel<N, I>::clear()
{
    this->beginResetModel();
    while (QUaModel<N, I>::m_root->children().count() > 0)
    {
        auto wrapper = QUaModel<N, I>::m_root->children().takeFirst();
        // NOTE : QUaNodeWrapper destructor removes connections
        delete wrapper;
    }
    m_hashCategories.clear();
    this->endResetModel();
}

#endif // QUACATEGORYMODEL_H
