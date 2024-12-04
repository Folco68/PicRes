/*
 * PicRes - GUI program to resize pictures in an easy way
 * Copyright (C) 2020-2025 Martial Demolins AKA Folco
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

#include "DlgHelp.hpp"
#include "../BeforeRelease.hpp"
#include "../Global.hpp"
#include "ui_DlgHelp.h"
#include <QFile>
#include <QPushButton>
#include <QTextStream>

DlgHelp::DlgHelp(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DlgHelp)
{
    // UI
    ui->setupUi(this);
    ui->VLayoutMain->setAlignment(ui->ButtonClose, Qt::AlignRight); // Put the Close button on the right side
    ui->TabWidget->setCurrentIndex(0);                              // Ensure that the active tab is the firt one
    setWindowTitle(MAIN_WINDOW_TITLE);

    // Readme
    QFile FileReadme(":/Readme.txt");
    if (FileReadme.open(QIODevice::ReadOnly)) {
        QTextStream StreamReadme(&FileReadme);
        QString     Readme = StreamReadme.readAll();
        Readme.replace("PLACEHOLDER_POSITION_STR", POSITION_STR);
        Readme.replace("PLACEHOLDER_COPYRIGHT_STR", COPYRIGHT_STR);
        Readme.replace("PLACEHOLDER_APPLICATION_VERSION_STR", APPLICATION_VERSION_STR);
        Readme.replace("PLACEHOLDER_QT_VERSION_STR", QT_VERSION_STR);
        Readme.replace("PLACEHOLDER_EMAIL_PERSONAL", EMAIL_PERSONAL);
        Readme.replace("PLACEHOLDER_EMAIL_PROFESSIONAL", EMAIL_PROFESSIONAL);
        ui->TextReadme->setPlainText(Readme);
    }

    // License
    QFile FileLicense(":/License.txt");
    if (FileLicense.open(QIODevice::ReadOnly)) {
        QTextStream StreamLicense(&FileLicense);
        ui->TextLicense->setPlainText(StreamLicense.readAll());
    }

    // Changelog
    QFile FileChangelog(":/Changelog.txt");
    if (FileChangelog.open(QIODevice::ReadOnly)) {
        QTextStream StreamChangelog(&FileChangelog);
        ui->TextChangelog->setPlainText(StreamChangelog.readAll());
    }

    // Connection
    connect(ui->ButtonClose, &QPushButton::clicked, [this]() { accept(); });
}

DlgHelp::~DlgHelp()
{
    delete ui;
}

void DlgHelp::openDlgHelp(QWidget* parent)
{
    DlgHelp* Dlg = new DlgHelp(parent);
    Dlg->exec();
    delete Dlg;
}
