#include "quacommondialog.h"
#include "ui_quacommondialog.h"

#include <QPushButton>

QUaCommonDialogPtr QUaCommonDialog::CreateModal(QWidget * parent)
{
	auto ptr = new QUaCommonDialog(parent);
	auto ptrShared = QUaCommonDialogPtr(ptr,
	[](QUaCommonDialog *dialog) {
		dialog->deleteLater();
	});
	// NOTE : copy the shared ptr in the finished lambda capture
	QObject::connect(ptr, &QDialog::finished,
	[ptr, ptrShared]() {
		// disconnect this signal to lambda capture gets cleared up
		QObject::disconnect(ptr, &QDialog::finished, 0, 0);
	});
	// return shared
	return ptrShared;
}

QUaCommonDialog::QUaCommonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaCommonDialog)
{
    ui->setupUi(this);
	ui->buttonBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->buttonBox->button(QDialogButtonBox::StandardButton::Cancel)->setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

QUaCommonDialog::~QUaCommonDialog()
{
	emit this->dialogDestroyed();
    delete ui;
}

QWidget * QUaCommonDialog::widget() const
{
	// find the widget and return a reference to it
	return this->findChild<QWidget*>("diagwidget");
}

void QUaCommonDialog::setWidget(QWidget * w)
{
	// check pointer
	Q_CHECK_PTR(w);
	if (!w)
	{
		return;
	}
	// check if has widget already
	Q_ASSERT(!this->widget());
	if (this->widget())
	{
		return;
	}
	// adjust widget to compensate for hidden widgets
	w->adjustSize();
	// take ownership
	w->setParent(this);
	// set object name in order to be able to retrieve later
	w->setObjectName("diagwidget");
	// put the widget in the layout
	ui->verticalLayout->insertWidget(0, w);
	// adjust dialog's geometry
	auto geo = this->geometry();
	geo.setWidth (1.1 * w->width ());
	geo.setHeight(1.1 * w->height());
	this->setGeometry(geo);
	// if parent defined, move to center of parent
	auto parent = qobject_cast<QWidget*>(this->parent());
	if (!parent)
	{
		return;
	}
	// get root parent
	while(parent)
	{
		auto newParent = qobject_cast<QWidget*>(parent->parent());
		if (!newParent)
		{
			break;
		}
		parent = newParent;
	}
	this->move(
		parent->x() + (parent->width () / 2) - (geo.width () / 2),
		parent->y() + (parent->height() / 2) - (geo.height() / 2)
	);
}

void QUaCommonDialog::clearButtons()
{
	ui->buttonBox->clear();
}

void QUaCommonDialog::addButton(const QString & text, QDialogButtonBox::ButtonRole role)
{
	auto butt = ui->buttonBox->addButton(text, role);
	butt->setFocusPolicy(Qt::FocusPolicy::NoFocus);
}