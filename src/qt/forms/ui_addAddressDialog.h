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
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AddAddressDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *warningLabel;
    QFormLayout *formLayout;
    QLabel *capsLabel;
    QLabel *keyLabel1;
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
        verticalLayout = new QVBoxLayout(AddAddressDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        warningLabel = new QLabel(AddAddressDialog);
        warningLabel->setObjectName(QStringLiteral("warningLabel"));
        warningLabel->setTextFormat(Qt::RichText);
        warningLabel->setWordWrap(true);

        verticalLayout->addWidget(warningLabel);

        formLayout = new QFormLayout();
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        capsLabel = new QLabel(AddAddressDialog);
        capsLabel->setObjectName(QStringLiteral("capsLabel"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        capsLabel->setFont(font);
        capsLabel->setAlignment(Qt::AlignCenter);

        formLayout->setWidget(3, QFormLayout::FieldRole, capsLabel);

        keyLabel1 = new QLabel(AddAddressDialog);
        keyLabel1->setObjectName(QStringLiteral("keyLabel1"));

        formLayout->setWidget(1, QFormLayout::FieldRole, keyLabel1);

        addressEdit = new QLineEdit(AddAddressDialog);
        addressEdit->setObjectName(QStringLiteral("addressEdit"));
        addressEdit->setEchoMode(QLineEdit::Normal);

        formLayout->setWidget(2, QFormLayout::FieldRole, addressEdit);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(AddAddressDialog);
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
        capsLabel->setText(QString());
        keyLabel1->setText(QApplication::translate("AddAddressDialog", "Paste bitcoin or ratcoin private key", 0));
    } // retranslateUi

};

namespace Ui {
    class AddAddressDialog: public Ui_AddAddressDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDADDRESSDIALOG_H
