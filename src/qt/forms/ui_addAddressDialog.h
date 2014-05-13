/********************************************************************************
** Form generated from reading UI file 'addAddressDialog.ui'
**
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDADDRESSDIALOG_H
#define UI_ADDADDRESSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AddAddressDialog
{
public:
    QLabel *warningLabel;
    QDialogButtonBox *buttonBox;
    QLabel *capsLabel;
    QLabel *keyLabel1;
    QLineEdit *addressEdit;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBox;
    QLabel *label;

    void setupUi(QDialog *AddAddressDialog)
    {
        if (AddAddressDialog->objectName().isEmpty())
            AddAddressDialog->setObjectName(QStringLiteral("AddAddressDialog"));
        AddAddressDialog->resize(598, 164);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AddAddressDialog->sizePolicy().hasHeightForWidth());
        AddAddressDialog->setSizePolicy(sizePolicy);
        AddAddressDialog->setMinimumSize(QSize(550, 0));
        warningLabel = new QLabel(AddAddressDialog);
        warningLabel->setObjectName(QStringLiteral("warningLabel"));
        warningLabel->setGeometry(QRect(8, 8, 16, 16));
        warningLabel->setTextFormat(Qt::RichText);
        warningLabel->setWordWrap(true);
        buttonBox = new QDialogButtonBox(AddAddressDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(420, 130, 165, 21));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        capsLabel = new QLabel(AddAddressDialog);
        capsLabel->setObjectName(QStringLiteral("capsLabel"));
        capsLabel->setGeometry(QRect(94, 63, 16, 16));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        capsLabel->setFont(font);
        capsLabel->setAlignment(Qt::AlignCenter);
        keyLabel1 = new QLabel(AddAddressDialog);
        keyLabel1->setObjectName(QStringLiteral("keyLabel1"));
        keyLabel1->setGeometry(QRect(10, 60, 193, 16));
        addressEdit = new QLineEdit(AddAddressDialog);
        addressEdit->setObjectName(QStringLiteral("addressEdit"));
        addressEdit->setGeometry(QRect(10, 90, 401, 22));
        addressEdit->setEchoMode(QLineEdit::Normal);
        widget = new QWidget(AddAddressDialog);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(10, 20, 493, 22));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        checkBox = new QCheckBox(widget);
        checkBox->setObjectName(QStringLiteral("checkBox"));

        horizontalLayout->addWidget(checkBox);

        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);


        retranslateUi(AddAddressDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AddAddressDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AddAddressDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AddAddressDialog);
    } // setupUi

    void retranslateUi(QDialog *AddAddressDialog)
    {
        AddAddressDialog->setWindowTitle(QApplication::translate("AddAddressDialog", "Address Dialog", 0));
        capsLabel->setText(QString());
        keyLabel1->setText(QApplication::translate("AddAddressDialog", "Paste bitcoin or ratcoin private key", 0));
        checkBox->setText(QApplication::translate("AddAddressDialog", "Generate/Use", 0));
        label->setText(QApplication::translate("AddAddressDialog", "Press OK to obtain new key or uncheck the box to use your own key", 0));
    } // retranslateUi

};

namespace Ui {
    class AddAddressDialog: public Ui_AddAddressDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDADDRESSDIALOG_H
