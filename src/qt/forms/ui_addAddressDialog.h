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
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBox;
    QLabel *warningLabel;
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
        layoutWidget = new QWidget(AddAddressDialog);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(30, 10, 541, 132));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        checkBox = new QCheckBox(layoutWidget);
        checkBox->setObjectName(QStringLiteral("checkBox"));

        horizontalLayout->addWidget(checkBox);


        verticalLayout->addLayout(horizontalLayout);

        warningLabel = new QLabel(layoutWidget);
        warningLabel->setObjectName(QStringLiteral("warningLabel"));
        warningLabel->setTextFormat(Qt::RichText);
        warningLabel->setWordWrap(true);

        verticalLayout->addWidget(warningLabel);

        capsLabel = new QLabel(layoutWidget);
        capsLabel->setObjectName(QStringLiteral("capsLabel"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        capsLabel->setFont(font);
        capsLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(capsLabel);

        keyLabel1 = new QLabel(layoutWidget);
        keyLabel1->setObjectName(QStringLiteral("keyLabel1"));

        verticalLayout->addWidget(keyLabel1);

        addressEdit = new QLineEdit(layoutWidget);
        addressEdit->setObjectName(QStringLiteral("addressEdit"));
        addressEdit->setEchoMode(QLineEdit::Normal);

        verticalLayout->addWidget(addressEdit);

        buttonBox = new QDialogButtonBox(layoutWidget);
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
        checkBox->setText(QApplication::translate("AddAddressDialog", "Generate the key", 0));
        capsLabel->setText(QString());
        keyLabel1->setText(QApplication::translate("AddAddressDialog", "Paste bitcoin or ratcoin private key", 0));
    } // retranslateUi

};

namespace Ui {
    class AddAddressDialog: public Ui_AddAddressDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDADDRESSDIALOG_H
