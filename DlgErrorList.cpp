#include <QPushButton>
#include "DlgErrorList.hpp"
#include "ui_DlgErrorList.h"

//
//  DlgErrorList
//
// Ctor
//

DlgErrorList::DlgErrorList(QString message, QStringList filenames, QWidget* parent) : QDialog(parent), ui(new Ui::DlgErrorList)
{
    // UI
    ui->setupUi(this);
    ui->LabelError->setText(message);                                 // Set the error message
    ui->verticalLayout->setAlignment(ui->ButtonOk, Qt::AlignHCenter); // Center the OK button
    ui->ListInvalidFiles->addItems(filenames);                        // Add filenames to the list
    setMinimumSize(size().width() * 2, size().height());              // Tweak the horizontal size to avoid a scrollable list widget

    // Connection
    connect(ui->ButtonOk, &QPushButton::clicked, [this]() { accept(); }); // Connect the Ok button to close the dialog
}

//
//  ~DlgDropError
//
// Dtor
//

DlgErrorList::~DlgErrorList()
{
    delete ui;
}

//
//  openDlgErrorList
//
// Static method provided for conveniency
//

void DlgErrorList::openDlgErrorList(QString message, QStringList filenames, QWidget* parent)
{
    DlgErrorList dlg(message, filenames, parent);
    dlg.exec();
}
