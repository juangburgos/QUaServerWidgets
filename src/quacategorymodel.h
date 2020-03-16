#ifndef QUACATEGORYMODEL_H
#define QUACATEGORYMODEL_H

#include <QUaTreeModel>
#include <QUaModelItemTraits>

// NOTE : need to reimplement model methods instaed of using a special type T because:
// 1) That type T would have to be templated T<N> to support other types and
//    writting traits for it requires constructs that C++ does not support
// 2) The respective QUaView<N> requires the same N as the model, so user would have
//    to work with T and the API would not be properly abstracted
template <typename N>
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
        typename QUaModel<N>::QUaNodeWrapper*,
        QString
    > m_hashCategories;

    QUaModel<N>::QUaNodeWrapper* addCategoryInternal(const QString& strCategory);

    QUaModel<N>::QUaNodeWrapper* getCategoryInternal(const QString& strCategory);
};

template<typename N>
inline QUaCategoryModel<N>::QUaCategoryModel(QObject* parent) :
    QUaTreeModel<N>(parent)
{
    QUaModel<N>::m_root = new typename QUaModel<N>::QUaNodeWrapper(
        QUaModelItemTraits::GetInvalid<N>()
    );
}

template<typename N>
inline QUaCategoryModel<N>::~QUaCategoryModel()
{
    if (QUaModel<N>::m_root)
    {
        delete QUaModel<N>::m_root;
        QUaModel<N>::m_root = nullptr;
    }
}

template<typename N>
inline void QUaCategoryModel<N>::addCategory(const QString& strCategory)
{
    this->addCategoryInternal(strCategory);
}

template<typename N>
inline void QUaCategoryModel<N>::removeCategory(const QString& strCategory)
{
    Q_CHECK_PTR(QUaModel<N>::m_root);
    // ignore unexisting category
    auto category = m_hashCategories.key(strCategory, nullptr);
    if (!category)
    {
        return;
    }
    int idx = QUaModel<N>::m_root->children().indexOf(category);
    Q_ASSERT(idx >= 0);
    if (idx < 0)
    {
        return;
    }
    auto wrapper = QUaModel<N>::m_root->children().at(idx);
    // use internal method (deletes wrapper)
    this->QUaModel<N>::removeWrapper(wrapper);
    // remove from hash before deleting wrapper
    Q_ASSERT(m_hashCategories.contains(wrapper));
    m_hashCategories.remove(wrapper);
}

template<typename N>
inline bool QUaCategoryModel<N>::hasCategory(const QString& strCategory) const
{
    return this->getCategoryInternal(strCategory);
}

template<typename N>
inline QStringList QUaCategoryModel<N>::categories() const
{
    return m_hashCategories.values();
}

template<typename N>
inline void QUaCategoryModel<N>::addNodeToCategory(const QString& strCategory, N node)
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
    auto wrapper = new typename QUaModel<N>::QUaNodeWrapper(
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
    auto conn = QUaModelItemTraits::DestroyCallback<N>(wrapper->node(),
        [this, wrapper]() {
            Q_CHECK_PTR(wrapper);
            Q_CHECK_PTR(QUaModel<N>::m_root);
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

template<typename N>
template<typename X>
inline 
typename std::enable_if<std::is_pointer<X>::value, QString>::type 
QUaCategoryModel<N>::nodeCategory(N node)
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

template<typename N>
template<typename X>
inline 
typename std::enable_if<!std::is_pointer<X>::value, QString>::type 
QUaCategoryModel<N>::nodeCategory(N* node)
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

template<typename N>
template<typename X>
inline 
typename std::enable_if<std::is_pointer<X>::value, QList<N>>::type 
QUaCategoryModel<N>::nodesByCategory(const QString& strCategory)
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

template<typename N>
template<typename X>
inline
typename std::enable_if<!std::is_pointer<X>::value, QList<N*>>::type
QUaCategoryModel<N>::nodesByCategory(const QString& strCategory)
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

template<typename N>
template<typename X>
inline
typename std::enable_if<std::is_pointer<X>::value, bool>::type
QUaCategoryModel<N>::removeNode(N node)
{
    bool res = false;
    // look for node in all categories
    auto& categories = QUaModel<N>::m_root->children();
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

template<typename N>
template<typename X>
inline 
typename std::enable_if<!std::is_pointer<X>::value, bool>::type 
QUaCategoryModel<N>::removeNode(N* node)
{
    bool res = false;
    // look for node in all categories
    auto& categories = QUaModel<N>::m_root->children();
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

template<typename N>
inline QStringList QUaCategoryModel<N>::indexesToCategories(
        const QModelIndexList& indexes
    ) const
{
    QStringList categories;
    for (auto index : indexes)
    {
        // ignore root
        if (!this->checkIndex(index, CheckIndexOption::IndexIsValid))
        {
            continue;
        }
        // ignore nodes
        auto wrapper = static_cast<QUaNodeWrapper*>(index.internalPointer());
        Q_CHECK_PTR(wrapper);
        if (QUaModelItemTraits::IsValid<N>(wrapper->node()))
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

template<typename N>
inline QVariant QUaCategoryModel<N>::data(
    const QModelIndex& index, 
    int role) const
{
    // early exit for inhandled cases
    if (!QUaModel<N>::m_root || !index.isValid())
    {
        return QVariant();
    }
    // only display data (text)
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    // get internal reference
    auto wrapper = static_cast<QUaModel<N>::QUaNodeWrapper*>(index.internalPointer());
    // NOTE : do not ignore invalid
    auto &categories = QUaModel<N>::m_root->children();
    auto res = std::find_if(categories.begin(), categories.end(),
    [wrapper](QUaModel<N>::QUaNodeWrapper * category) {
        return wrapper == category;
    });
    auto category = (res == categories.end()) ? nullptr : *res;
    // return category name if category
    if (category)
    {
        if (index.column() == 0)
        {
            Q_ASSERT(m_hashCategories.contains(category));
            return m_hashCategories[category];
        }
        return QVariant();
    }
    // default implementation if no ColumnDataSource has been defined
    if (m_mapDataSourceFuncs.isEmpty())
    {
        Q_ASSERT(m_columnCount == 1);
        return tr("");
    }
    // empty if no ColumnDataSource defined for this column
    if (!m_mapDataSourceFuncs.contains(index.column()) ||
        !m_mapDataSourceFuncs[index.column()].m_dataCallback)
    {
        return QVariant();
    }
    // use user-defined ColumnDataSource
    return m_mapDataSourceFuncs[index.column()].m_dataCallback(wrapper->node());

}

template<typename N>
inline typename QUaModel<N>::QUaNodeWrapper* 
QUaCategoryModel<N>::addCategoryInternal(const QString& strCategory)
{
    Q_CHECK_PTR(QUaModel<N>::m_root);
    // ignore existing category
    auto wrapper = this->getCategoryInternal(strCategory);
    if (wrapper)
    {
        return wrapper;
    }
    QModelIndex index = QUaModel<N>::m_root->index();
    // get new child's row
    int row = QUaModel<N>::m_root->children().count();
    // notify views that row will be added
    this->beginInsertRows(index, row, row);
    // create new wrapper
    // NOTE : is invalid, we use the wrapper pointer as a key to the categories hash
    wrapper = new typename QUaModel<N>::QUaNodeWrapper(
        QUaModelItemTraits::GetInvalid<N>(),
        QUaModel<N>::m_root,
        false // NOTE : not recursive
    );
    // apprend to parent's children list
    QUaModel<N>::m_root->children() << wrapper;
    // bind to string
    m_hashCategories[wrapper] = strCategory;
    // notify views that row addition has finished
    this->endInsertRows();
    // NOTE : do not need to subscribe to Datachange or DestroyCallback 
    //        because categories always have invalid nodes and are
    //        removed manually (in)directly by ::removeCategory
    return wrapper;
}

template<typename N>
inline typename QUaModel<N>::QUaNodeWrapper* 
QUaCategoryModel<N>::getCategoryInternal(const QString& strCategory)
{
    return m_hashCategories.key(strCategory, nullptr);
}

template<typename N>
inline void QUaCategoryModel<N>::clear()
{
    this->beginResetModel();
    while (QUaModel<N>::m_root->children().count() > 0)
    {
        auto wrapper = QUaModel<N>::m_root->children().takeFirst();
        // NOTE : QUaNodeWrapper destructor removes connections
        delete wrapper;
    }
    m_hashCategories.clear();
    this->endResetModel();
}

#endif // QUACATEGORYMODEL_H