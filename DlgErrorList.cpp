/*
 * PicRes - GUI program to resize pictures in an easy way
 * Copyright (C) 2020 Martial Demolins AKA Folco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * mail: martial <dot> demolins <at> gmail <dot> com
 */

#include "DlgErrorList.hpp"
#include "Global.hpp"
#include "ui_DlgErrorList.h"
#include <QPushButton>

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
    setWindowTitle(MAIN_WINDOW_TITLE);

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
    DlgErrorList* dlg = new DlgErrorList(message, filenames, parent);
    dlg->exec();
    delete dlg;
}
