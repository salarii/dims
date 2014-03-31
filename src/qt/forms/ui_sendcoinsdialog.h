/********************************************************************************
** Form generated from reading UI file 'sendcoinsdialog.ui'
**
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SENDCOINSDIALOG_H
#define UI_SENDCOINSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SendCoinsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *entries;
    QTableView *tableView;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *sendButton;
    QPushButton *clearButton;
    QPushButton *addButton;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLabel *labelBalance;

    void setupUi(QDialog *SendCoinsDialog)
    {
        if (SendCoinsDialog->objectName().isEmpty())
            SendCoinsDialog->setObjectName(QStringLiteral("SendCoinsDialog"));
        SendCoinsDialog->resize(850, 400);
        verticalLayout = new QVBoxLayout(SendCoinsDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(-1, -1, -1, 8);
        scrollArea = new QScrollArea(SendCoinsDialog);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 826, 341));
        verticalLayout_2 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        entries = new QVBoxLayout();
        entries->setSpacing(6);
        entries->setObjectName(QStringLiteral("entries"));

        verticalLayout_2->addLayout(entries);

        tableView = new QTableView(scrollAreaWidgetContents);
        tableView->setObjectName(QStringLiteral("tableView"));
        tableView->setContextMenuPolicy(Qt::CustomContextMenu);
        tableView->setTabKeyNavigation(false);
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->setSortingEnabled(true);
        tableView->verticalHeader()->setVisible(false);

        verticalLayout_2->addWidget(tableView);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        verticalLayout_2->setStretch(2, 1);
        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout->addWidget(scrollArea);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        sendButton = new QPushButton(SendCoinsDialog);
        sendButton->setObjectName(QStringLiteral("sendButton"));
        sendButton->setMinimumSize(QSize(150, 0));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/send"), QSize(), QIcon::Normal, QIcon::Off);
        sendButton->setIcon(icon);
        sendButton->setDefault(true);

        horizontalLayout->addWidget(sendButton);

        clearButton = new QPushButton(SendCoinsDialog);
        clearButton->setObjectName(QStringLiteral("clearButton"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(clearButton->sizePolicy().hasHeightForWidth());
        clearButton->setSizePolicy(sizePolicy);
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/remove"), QSize(), QIcon::Normal, QIcon::Off);
        clearButton->setIcon(icon1);
        clearButton->setAutoRepeatDelay(300);
        clearButton->setAutoDefault(false);

        horizontalLayout->addWidget(clearButton);

        addButton = new QPushButton(SendCoinsDialog);
        addButton->setObjectName(QStringLiteral("addButton"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/add"), QSize(), QIcon::Normal, QIcon::Off);
        addButton->setIcon(icon2);
        addButton->setAutoDefault(false);

        horizontalLayout->addWidget(addButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(3);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label = new QLabel(SendCoinsDialog);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout_2->addWidget(label);

        labelBalance = new QLabel(SendCoinsDialog);
        labelBalance->setObjectName(QStringLiteral("labelBalance"));
        labelBalance->setCursor(QCursor(Qt::IBeamCursor));
        labelBalance->setText(QStringLiteral("123.456 BTC"));
        labelBalance->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        horizontalLayout_2->addWidget(labelBalance);


        horizontalLayout->addLayout(horizontalLayout_2);


        verticalLayout->addLayout(horizontalLayout);

        verticalLayout->setStretch(0, 1);

        retranslateUi(SendCoinsDialog);

        QMetaObject::connectSlotsByName(SendCoinsDialog);
    } // setupUi

    void retranslateUi(QDialog *SendCoinsDialog)
    {
        SendCoinsDialog->setWindowTitle(QApplication::translate("SendCoinsDialog", "Send Coins", 0));
#ifndef QT_NO_TOOLTIP
        tableView->setToolTip(QApplication::translate("SendCoinsDialog", "Double-click to edit address or label", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        sendButton->setToolTip(QApplication::translate("SendCoinsDialog", "Confirm the send action", 0));
#endif // QT_NO_TOOLTIP
        sendButton->setText(QApplication::translate("SendCoinsDialog", "S&end", 0));
#ifndef QT_NO_TOOLTIP
        clearButton->setToolTip(QApplication::translate("SendCoinsDialog", "Clear all fields of the form.", 0));
#endif // QT_NO_TOOLTIP
        clearButton->setText(QApplication::translate("SendCoinsDialog", "Clear &All", 0));
#ifndef QT_NO_TOOLTIP
        addButton->setToolTip(QApplication::translate("SendCoinsDialog", "Send to multiple recipients at once", 0));
#endif // QT_NO_TOOLTIP
        addButton->setText(QApplication::translate("SendCoinsDialog", "Add &Recipient", 0));
        label->setText(QApplication::translate("SendCoinsDialog", "Balance:", 0));
    } // retranslateUi

};

namespace Ui {
    class SendCoinsDialog: public Ui_SendCoinsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SENDCOINSDIALOG_H
