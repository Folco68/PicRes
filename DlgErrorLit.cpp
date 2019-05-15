#include <QPushButton>
#include "DlgErrorList.hpp"
#include "ui_DlgErrorList.h"


//
//  DlgErrorList
//
// Ctor
//

DlgErrorList::DlgErrorList(QString message, QStringList filenames, QWidget *parent):
    QDialog(parent),
    ui(new Ui::DlgErrorList)
{
    //
    //  UI
    //

    ui->setupUi(this);

    // Set the error message
    ui->LabelError->setText(message);

    // Center the OK button
    ui->verticalLayout->setAlignment(ui->ButtonOk, Qt::AlignHCenter);

    // Add filenames to the list
    ui->ListInvalidFiles->addItems(filenames);

    // Tweak the horizontal size to avoid a scrollable list widget
    // FIXME: what is the right way to do that?
    setMinimumSize(size().width() * 2, size().height());

    //
    //  Connections
    //

    // Connect the Ok button to close the dialog
    connect(ui->ButtonOk, &QPushButton::clicked, this, &QDialog::accept);
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
