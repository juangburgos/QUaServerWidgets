#ifndef QUACOMMONDIALOG_H
#define QUACOMMONDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QSharedPointer>

namespace Ui {
class QUaCommonDialog;
}

class QUaCommonDialog;

typedef QSharedPointer<QUaCommonDialog> QUaCommonDialogPtr;

class QUaCommonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QUaCommonDialog(QWidget *parent = nullptr);
    ~QUaCommonDialog();

	QWidget * widget() const;
	// NOTE : takes ownership
	void      setWidget(QWidget * w);

	void clearButtons();
	void addButton(const QString &text, QDialogButtonBox::ButtonRole role);

	static QUaCommonDialogPtr CreateModal(QWidget *parent = nullptr);

signals:
	void dialogDestroyed();

private:
    Ui::QUaCommonDialog *ui;
};

#endif // QUACOMMONDIALOG_H
