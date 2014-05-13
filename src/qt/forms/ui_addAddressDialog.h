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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AddAddressDialog
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBox;
    QLabel *label;
    QLabel *keyLabel1;
    QLabel *warningLabel;
    QLabel *capsLabel;
    QLineEdit *addressEdit;
    QDialogButtonBox *buttonBox;

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
        widget = new QWidget(AddAddressDialog);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(30, 10, 541, 132));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        checkBox = new QCheckBox(widget);
        checkBox->setObjectName(QStringLiteral("checkBox"));

        horizontalLayout->addWidget(checkBox);

        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);


        verticalLayout->addLayout(horizontalLayout);

        keyLabel1 = new QLabel(widget);
        keyLabel1->setObjectName(QStringLiteral("keyLabel1"));

        verticalLayout->addWidget(keyLabel1);

        warningLabel = new QLabel(widget);
        warningLabel->setObjectName(QStringLiteral("warningLabel"));
        warningLabel->setTextFormat(Qt::RichText);
        warningLabel->setWordWrap(true);

        verticalLayout->addWidget(warningLabel);

        capsLabel = new QLabel(widget);
        capsLabel->setObjectName(QStringLiteral("capsLabel"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        capsLabel->setFont(font);
        capsLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(capsLabel);

        addressEdit = new QLineEdit(widget);
        addressEdit->setObjectName(QStringLiteral("addressEdit"));
        addressEdit->setEchoMode(QLineEdit::Normal);

        verticalLayout->addWidget(addressEdit);

        buttonBox = new QDialogButtonBox(widget);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(AddAddressDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AddAddressDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AddAddressDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AddAddressDialog);
    } // setupUi

    void retranslateUi(QDialog *AddAddressDialog)
    {
        AddAddressDialog->setWindowTitle(QApplication::translate("AddAddressDialog", "Address Dialog", 0));
        checkBox->setText(QApplication::translate("AddAddressDialog", "Generate/Use", 0));
        label->setText(QApplication::translate("AddAddressDialog", "Press OK to obtain new key or uncheck the box to use your own key", 0));
        keyLabel1->setText(QApplication::translate("AddAddressDialog", "Paste bitcoin or ratcoin private key", 0));
        capsLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class AddAddressDialog: public Ui_AddAddressDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDADDRESSDIALOG_H
