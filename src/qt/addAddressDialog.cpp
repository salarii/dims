// Copyright (c) 2011-2013 The Bitcoin developers
// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include  "addAddressDialog.h"
#include  "ui_addAddressDialog.h"

#include "guiconstants.h"
#include "walletmodel.h"

#include "allocators.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>

AddAddressDialog::AddAddressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAddressDialog),
    model(0),
    fCapsLock(false)
{
    ui->setupUi(this);
    ui->checkBox->setChecked(true);
    ui->addressEdit->setReadOnly(true);
    ui->addressEdit->setMaxLength(MAX_ADDRESS_SIZE);

    // Setup Caps Lock detection.
    ui->addressEdit->installEventFilter(this);

    //textChanged();
    connect(ui->addressEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
}

AddAddressDialog::~AddAddressDialog()
{
    // Attempt to overwrite text so that they do not linger around in memory
    ui->addressEdit->setText(QString(" ").repeated(ui->addressEdit->text().size()));
    delete ui;
}

void AddAddressDialog::setModel(WalletModel *model)
{
    this->model = model;
}

void AddAddressDialog::accept()
{
	SecureString address;
	if(!model)
		return;
	address.reserve(MAX_ADDRESS_SIZE);

	address.assign(ui->addressEdit->text().toStdString().c_str());
	if (ui->checkBox->isChecked())
	{
		model->generateNewAddress();
	}
	else {



		if ( !model->addAddress( address ) )
		{
			QMessageBox::critical(this, tr("Failed to add this address"),
								  tr("Possible reasons: private address given, does not meet expected syntax."));
			return;
		}
	}
	QDialog::accept();
}

void AddAddressDialog::textChanged()
{
    // Validate input, set Ok button to enabled when acceptable
    bool acceptable = false;


    acceptable = !ui->addressEdit->text().isEmpty() && !ui->addressEdit->text().isEmpty();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(acceptable);


}

bool AddAddressDialog::event(QEvent *event)
{
    // Detect Caps Lock key press.
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_CapsLock) {
            fCapsLock = !fCapsLock;
        }
        if (fCapsLock) {
            ui->capsLabel->setText(tr("Warning: The Caps Lock key is on!"));
        } else {
            ui->capsLabel->clear();
        }
    }
    return QWidget::event(event);
}

bool AddAddressDialog::eventFilter(QObject *object, QEvent *event)
{
    /* Detect Caps Lock.
     * There is no good OS-independent way to check a key state in Qt, but we
     * can detect Caps Lock by checking for the following condition:
     * Shift key is down and the result is a lower case character, or
     * Shift key is not down and the result is an upper case character.
     */
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        QString str = ke->text();
        if (str.length() != 0) {
            const QChar *psz = str.unicode();
            bool fShift = (ke->modifiers() & Qt::ShiftModifier) != 0;
            if ((fShift && *psz >= 'a' && *psz <= 'z') || (!fShift && *psz >= 'A' && *psz <= 'Z')) {
                fCapsLock = true;
                ui->capsLabel->setText(tr("Warning: The Caps Lock key is on!"));
            } else if (psz->isLetter()) {
                fCapsLock = false;
                ui->capsLabel->clear();
            }
        }
    }
    return QDialog::eventFilter(object, event);
}

void AddAddressDialog::on_checkBox_stateChanged(int arg1)
{
    if (ui->checkBox->isChecked())
    {       ui->addressEdit->setEchoMode(QLineEdit::Normal);
            ui->addressEdit->setText("Uncheck the box to use your key");
            ui->addressEdit->setReadOnly(true);
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
            else {
            ui->addressEdit->setText("");
            ui->addressEdit->setEchoMode(QLineEdit::Password);
            ui->addressEdit->setReadOnly(false);
}
}

