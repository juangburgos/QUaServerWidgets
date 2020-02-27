#ifndef QUATYPEMODEL_H
#define QUATYPEMODEL_H

#include <QUaNodeModel>

class QUaTypeModel : public QUaNodeModel
{
    Q_OBJECT

public:
    explicit QUaTypeModel(QObject *parent = nullptr);

private:
    void bindChangeCallbackForColumn(const int& column, QUaNodeWrapper* node) override;
};

#endif // QUATYPEMODEL_H
