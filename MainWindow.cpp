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

#include "MainWindow.hpp"
#include "DlgErrorList.hpp"
#include "DropThread.hpp"
#include "TableItem.hpp"
#include "ui_MainWindow.h"
#include <QAbstractItemView>
#include <QCoreApplication>
#include <QHeaderView>
#include <QImageReader>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QVariant>

//
//  MainWindow
//
// Constructor
//

MainWindow::MainWindow(int argc, char** argv)
    : ui(new Ui::MainWindow)
    , Table(new QTableWidget)
    , SupportedExtensionList(QImageReader::supportedImageFormats())
{
    //
    //  UI
    //

    ui->setupUi(this);

    // Align some widgets
    centralWidget()->layout()->setAlignment(ui->ButtonResize, Qt::AlignHCenter);
    ui->HLayoutPercentage->setAlignment(ui->SpinboxPercentage, Qt::AlignHCenter);
    ui->HLayoutAbsoluteSize->setAlignment(ui->SpinboxAbsoluteSize, Qt::AlignHCenter);
    ui->HLayoutProgress->setAlignment(Qt::AlignRight);
    ui->BoxDrop->layout()->setAlignment(Qt::AlignCenter);

    // Set progress bar
    ui->ProgressBar->setAlignment(Qt::AlignCenter);
    ui->ProgressBar->setMinimum(0);

    // Configure the table displaying the dropped files
    // The table is created here and not with the WYSIWYG tool, because I prefer to configure it by hand
    QStringList Labels;
    Labels << "File name"
           << "Actual size"
           << "New size";

    this->Table->setColumnCount(COLUMN_COUNT);
    this->Table->setShowGrid(true);
    this->Table->setSortingEnabled(true);
    this->Table->setAlternatingRowColors(true);
    this->Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->Table->setSelectionMode(QAbstractItemView::SingleSelection);
    this->Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->Table->horizontalHeader()->setStretchLastSection(true);
    this->Table->verticalHeader()->setVisible(false);
    this->Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    this->Table->setHorizontalHeaderLabels(Labels);

    ui->VLayoutDrop->insertWidget(1, this->Table);

    updateUI();

    //
    //  Data
    //

    // Remove extensions supported only in read-only mode by Qt
    this->SupportedExtensionList.removeOne("GIF");
    this->SupportedExtensionList.removeOne("PBM");
    this->SupportedExtensionList.removeOne("PGM");

    //
    //  Connections
    //

    // Enable/disable the spinboxes according to the checked radio buttons. Force new size update
    connect(ui->RadioPercentage, &QRadioButton::clicked, [this]() { updateAllSizes(); });
    connect(ui->RadioAbsoluteSize, &QRadioButton::clicked, [this]() { updateAllSizes(); });

    // Connect the spinboxes to update the new size
    connect(ui->SpinboxPercentage, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this]() { onPercentageValueChanged(); });
    connect(ui->SpinboxAbsoluteSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this]() { onAbsoluteValueChanged(); });

    // Connect other buttons
    connect(ui->ButtonResize, &QPushButton::clicked, [this]() { onButtonResizeClicked(); });
    connect(ui->ButtonClearList, &QPushButton::clicked, [this]() { clearTable(); });
    connect(ui->ButtonCancel, &QPushButton::clicked, [this]() { cancelTask(); });

    // Connect the main window to the signal emitted by the dropbox
    connect(ui->BoxDrop, &Dropbox::picturesDropped, this, &MainWindow::onPicturesDropped);
    //    connect(ui->BoxDrop, &Dropbox::picturesDropped, [this](QList<QUrl> URLs) { onPicturesDropped(URLs); });

    // Connect dropping worker to main UI. Queued connections needed because of the different threads
    connect(DropThread::instance(), &DropThread::dropResultReady, this, &MainWindow::onDropResultReady, Qt::QueuedConnection);
    connect(DropThread::instance(), &DropThread::processingDroppedFile, this, &MainWindow::onDroppedFileProcessed, Qt::QueuedConnection);
    connect(DropThread::instance(), &DropThread::dropProcessTerminaded, this, &MainWindow::onDropProcessTerminated, Qt::QueuedConnection);

    // If files were dropped on the program icon, add them to the UI
    if (argc != 1) {
        QList<QUrl> urls;
        for (int i = 1; i < argc; i++) {
            urls << QUrl::fromLocalFile(argv[i]);
        }

        // Force file handling
        onPicturesDropped(urls);
    }
}

//
//  ~MainWindow
//
// Destructor
//

MainWindow::~MainWindow()
{
    delete ui;
}

//
//  updateUI
//
// Set UI according to program state
//

// WARNING: doesn't take in account the resize thread
void MainWindow::updateUI()
{
    // Shortcuts for common properties
    bool TableIsEmpty        = this->Table->rowCount() == 0;
    bool DropThreadIsRunning = DropThread::instance()->isRunning();
    int ItemCount            = this->Table->rowCount();

    ui->ProgressBar->setVisible(DropThreadIsRunning);                       // Progress bar is visible if files are currently dropped
    ui->LabelDrop->setVisible(TableIsEmpty && !DropThreadIsRunning);        // Hint label is displayed if the table is empty and no process running
    this->Table->setVisible(!TableIsEmpty);                                 // Table is displayed if it contains elements
    ui->BoxResizingMethod->setDisabled(TableIsEmpty);                       // Resizing methods are disabled if the table is empty
    ui->ButtonClearList->setVisible(!TableIsEmpty && !DropThreadIsRunning); // Can't clear the list if it's empty or in use
    ui->ButtonCancel->setVisible(DropThreadIsRunning);                      // Cancel button is visible only if a process is running
    ui->ButtonResize->setEnabled(!TableIsEmpty && !DropThreadIsRunning);    // We can resize when there is something to resize and drop process is complete
    ui->ButtonClearList->setText(tr("Clear list (%1 items)").arg(ItemCount));
}

//
//  onButtonResizeClicked
//
// Called when the user wants to resize the files displayed in the table
//

void MainWindow::onButtonResizeClicked()
{
    /*
    // Show and set the progress bar
    ui->ProgressBar->setVisible(true);
    ui->ProgressBar->setRange(0, Table->rowCount());
    ui->ProgressBar->reset();

    // List of filename that couldn't be resized
    QStringList InvalidFiles;

    // Resize images
    for (int i = 0; i < Table->rowCount(); i++) {
        // Get filename
        QString Filename = Table->item(i, COLUMN_FILENAME)->data(Qt::DisplayRole).toString();

        // Refresh progress bar
        ui->ProgressBar->setValue(i + 1);
        ui->ProgressBar->setFormat(tr("Resizing file: %1 (%p%)").arg(Filename));

        // Open image
        QImage Image(Filename);

        // Get dimensions and update them
        int Width  = Table->item(i, COLUMN_WIDTH)->data(Qt::DisplayRole).toInt();
        int Height = Table->item(i, COLUMN_HEIGHT)->data(Qt::DisplayRole).toInt();
        updateSize(Width, Height);

        // Resize the image
        QImage ResizedImage = Image.scaled(Width, Height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (ResizedImage.isNull()) {
            InvalidFiles << Filename;
            continue;
        }

        // Save the image
        if (!ResizedImage.save(Filename)) {
            InvalidFiles << Filename;
        }
    }

    // Display the success message if all was fine
    if (InvalidFiles.isEmpty()) {
        QMessageBox::information(
                    this,
                    "PicRes",
                    tr("Resizing successful!"));
    }
    else {
        DlgErrorList Dialog(tr("Some files couldn't be resized:"), InvalidFiles);
        Dialog.exec();
    }

    // Clear the table and reset the UI
    clearTable();
*/
}


//
//  onPicturesDropped
//
// Slot triggered when files are dropped in the drop box
//

void MainWindow::onPicturesDropped(QList<QUrl> URLs)
{
    DropThread::instance()->drop(URLs);
    ui->ProgressBar->setMaximum(ui->ProgressBar->maximum() + URLs.count());
    updateUI();
}

//
//  onDropResultReady
//
// Slot called when the drop thread has available results
// Get the results and display them in the main table
//

void MainWindow::onDropResultReady()
{
    QList<QPair<QString, QSize>> result;
    DropThread::instance()->result(&result);

    for (int i = 0; i < result.size(); i++) {
        QString filename = result.at(i).first;
        QSize size       = result.at(i).second;

        // Check that the file is not added yet. If so, discard it without any warning message
        bool AlreadyPresent = false;
        for (int i = 0; i < this->Table->rowCount(); i++) {
            if (filename == this->Table->item(i, COLUMN_FILENAME)->text()) {
                AlreadyPresent = true;
                break;
            }
        }
        if (AlreadyPresent) {
            continue;
        }

        // Add the file to the table if it could be read, else add it to the error list
        if (size.isValid()) {
            // Compute new size
            int NewWidth  = size.width();
            int NewHeight = size.height();
            updateSize(NewWidth, NewHeight);

            // Create the items to add. Use a QTableWidgetItem for the filename, because we don't want it to be centered
            QTableWidgetItem* ItemName = new QTableWidgetItem(filename);
            TableItem* ItemOrgSize     = new TableItem(QString("%1 x %2").arg(size.width()).arg(size.height()));
            TableItem* ItemNewSize     = new TableItem(QString("%1 x %2").arg(NewWidth).arg(NewHeight));
            ItemOrgSize->setData(Qt::UserRole, QVariant::fromValue(size));

            // Populate the table
            int row = this->Table->rowCount();
            this->Table->insertRow(row);
            this->Table->setItem(row, COLUMN_FILENAME, ItemName);
            this->Table->setItem(row, COLUMN_ORGSIZE, ItemOrgSize);
            this->Table->setItem(row, COLUMN_NEWSIZE, ItemNewSize);
        }
        else {
            this->InvalidDroppedFiles << filename;
        }
    }

    // Update UI only if the list contains data
    if (result.count() != 0) {
        updateUI();
    }
}

//
//  onDroppedFileProcessed
//
// Slot triggerer when the drop thread starts to process a file.
// Allow to update the status bar with file name and percentage
//

void MainWindow::onDroppedFileProcessed(QString filename)
{
    ui->ProgressBar->setValue(ui->ProgressBar->value() + 1);       // Update percentage value
    ui->ProgressBar->setFormat(QString("%1 (%p%)").arg(filename)); // Write filename + percentage in the progressbar
}

//
//  onDropProcessTerminated
//
// Slot called when drop thread has terminated to handles dropped files. Update UI, and inform about process errors
//

void MainWindow::onDropProcessTerminated()
{
    // Reset progress bar and update UI
    ui->ProgressBar->setMaximum(0);
    updateUI();

    // Display invalid files if a problem occured, then clear the list
    if (!this->InvalidDroppedFiles.isEmpty()) {
        DlgErrorList::openDlgErrorList(tr("Some files couldn't be opened:"), this->InvalidDroppedFiles, this);
        this->InvalidDroppedFiles.clear();
    }
}

//
//  updateAllSizes
//
// Compute and display the new picture sizes, depending on the current resizing method
//

void MainWindow::updateAllSizes()
{
    for (int i = 0; i < this->Table->rowCount(); i++) {
        QSize size = this->Table->item(i, COLUMN_ORGSIZE)->data(Qt::UserRole).toSize();
        int width  = size.width();
        int height = size.height();
        updateSize(width, height);
        Table->item(i, COLUMN_NEWSIZE)->setText(QString("%1 x %2").arg(width).arg(height));
    }
}

//
//  onPercentageValueChanged
//
// Ensure that the percentage radio button is set, then update sizes
//

void MainWindow::onPercentageValueChanged()
{
    ui->RadioPercentage->setChecked(true);
    updateAllSizes();
}

//
//  onAbsoluteValueChanged
//
// Ensure that the absolute radio button is set, then update sizes
//

void MainWindow::onAbsoluteValueChanged()
{
    ui->RadioAbsoluteSize->setChecked(true);
    updateAllSizes();
}

//
//  updateSize
//
// Update the given size according to the selected resizing method
//

void MainWindow::updateSize(int& width, int& height)
{
    // Percentage method selected
    if (ui->RadioPercentage->isChecked()) {
        int Percentage = ui->SpinboxPercentage->value();
        width = (width * Percentage) / 100;
        height = (height * Percentage) / 100;
    }
    // Absolute Size method selected
    else {
        int MaxSize = ui->SpinboxAbsoluteSize->value();
        if (width > height) {
            height = (height * MaxSize) / width;
            width = MaxSize;
        }
        else {
            width = (width * MaxSize) / height;
            height = MaxSize;
        }
    }

    // Prevent from getting a null size
    width = max(width, 1);
    height = max(height, 1);
}

//
//  cleaTable
//
// Reset the main table
//

void MainWindow::clearTable()
{
    Table->clearContents();
    Table->setRowCount(0);
    updateUI();
}

//
//  cancelTask
//
// Slot triggere when the user wants to interrupt the dropping or resizing thread
//

void MainWindow::cancelTask()
{
    if (DropThread::instance()->isRunning()) {
        DropThread::instance()->requestInterruption();
    }
}
