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
#include "DlgHelp.hpp"
#include "DropThread.hpp"
#include "Global.hpp"
#include "ResizeThread.hpp"
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

MainWindow::MainWindow(int argc, char* argv[])
    : ui(new Ui::MainWindow), Table(new QTableWidget), SupportedExtensionList(QImageReader::supportedImageFormats()), CloseRequested(false)
{
    //
    //  UI
    //

    ui->setupUi(this);
    setWindowTitle(tr(MAIN_WINDOW_TITLE));

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

    // Insert between the tip label and the progress bar
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
    connect(ui->ButtonHelp, &QPushButton::clicked, [this]() { DlgHelp::openDlgHelp(this); });

    // Connect the main window to the signal emitted by the dropbox
    // connect(ui->BoxDrop, &Dropbox::picturesDropped, this, &MainWindow::onPicturesDropped);
    connect(ui->BoxDrop, &Dropbox::picturesDropped, [this](QList<QUrl> URLs) { onPicturesDropped(URLs); });

    // Connect drop thread to main UI. Queued connections needed because of the different threads
    connect(DropThread::instance(), &DropThread::dropResultReady, this, &MainWindow::onDropResultReady, Qt::QueuedConnection);
    connect(DropThread::instance(), &DropThread::processingDroppedFile, this, &MainWindow::onDroppedFileProcessed, Qt::QueuedConnection);
    connect(DropThread::instance(), &DropThread::dropProcessTerminaded, this, &MainWindow::onDropProcessTerminated, Qt::QueuedConnection);

    // Connect resize thread to main UI. Queued connections needed because of the different threads
    connect(ResizeThread::instance(), &ResizeThread::resizingFile, this, &MainWindow::onFileResizing, Qt::QueuedConnection);
    connect(ResizeThread::instance(), &ResizeThread::fileResized, this, &MainWindow::onFileResized, Qt::QueuedConnection);
    connect(ResizeThread::instance(), &ResizeThread::resizingTerminated, this, &MainWindow::onResizingTerminated, Qt::QueuedConnection);

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
    int ItemCount              = this->Table->rowCount();
    bool TableIsEmpty          = ItemCount == 0;
    bool DropThreadIsRunning = DropThread::instance()->isRunning();
    bool ResizeThreadIsRunning = ResizeThread::instance()->isRunning();
    bool AThreadIsRunning      = DropThreadIsRunning || ResizeThreadIsRunning;

    ui->ProgressBar->setVisible(AThreadIsRunning);                               // Progress bar is visible if files are currently dropped
    ui->LabelDrop->setVisible(TableIsEmpty && !AThreadIsRunning);                // Hint label is displayed if the table is empty and no process running
    this->Table->setVisible(!TableIsEmpty);                                      // Table is displayed if it contains elements
    ui->RadioPercentage->setDisabled(TableIsEmpty || ResizeThreadIsRunning);     // Resizing methods are disabled if the table is empty
    ui->RadioAbsoluteSize->setDisabled(TableIsEmpty || ResizeThreadIsRunning);   // Resizing methods are disabled if the table is empty
    ui->SpinboxPercentage->setDisabled(TableIsEmpty || ResizeThreadIsRunning);   // Resizing methods are disabled if the table is empty
    ui->SpinboxAbsoluteSize->setDisabled(TableIsEmpty || ResizeThreadIsRunning); // Resizing methods are disabled if the table is empty
    ui->ButtonClearList->setVisible(!TableIsEmpty && !AThreadIsRunning);         // Can't clear the list if it's empty or in use
    ui->ButtonCancel->setVisible(AThreadIsRunning);                              // Cancel button is visible only if a process is running
    ui->ButtonResize->setEnabled(!TableIsEmpty && !AThreadIsRunning);            // We can resize when there is something to resize and no thread is working
    ui->ButtonClearList->setText(tr("Clear list (%1 items)").arg(ItemCount));    // Display the size of the table in the Clear button

    // Reset progress bar if no thread is running
    if (!AThreadIsRunning) {
        ui->ProgressBar->setMaximum(0);
        ui->ProgressBar->setValue(0);
    }
}

//
//  onPicturesDropped
//
// Slot triggered when files are dropped in the drop box
//

void MainWindow::onPicturesDropped(QList<QUrl> URLs)
{
    // Prevent drops during resizing
    if (ResizeThread::instance()->isRunning()) {
        QMessageBox::critical(this, MAIN_WINDOW_TITLE, tr("Cannot drop files while resizing."), QMessageBox::Ok);
        return;
    }

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
        QSize orgsize    = result.at(i).second;

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
        if (orgsize.isValid()) {
            // Compute new size
            QSize newsize;
            updateSize(orgsize, newsize);

            // Create the items to add. Use a QTableWidgetItem for the filename, because we don't want it to be centered
            // Store related data in dedicated locations
            QTableWidgetItem* ItemName = new QTableWidgetItem(filename);

            TableItem* ItemOrgSize = new TableItem(QString("%1 x %2").arg(orgsize.width()).arg(orgsize.height()));
            ItemOrgSize->setData(Qt::UserRole, QVariant::fromValue(orgsize));

            TableItem* ItemNewSize = new TableItem(QString("%1 x %2").arg(newsize.width()).arg(newsize.height()));
            ItemNewSize->setData(Qt::UserRole, QVariant::fromValue(newsize));

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
    // Display invalid files if a problem occured, then clear the list
    if (!this->InvalidDroppedFiles.isEmpty()) {
        DlgErrorList::openDlgErrorList(tr("Some files couldn't be opened:"), this->InvalidDroppedFiles, this);
        this->InvalidDroppedFiles.clear();
    }

    updateUI();
}

//
//  onButtonResizeClicked
//
// Called when the user wants to resize the files displayed in the table
//

void MainWindow::onButtonResizeClicked()
{
    QMessageBox::warning(this, MAIN_WINDOW_TITLE, tr("Original pictures will be overwritten. Do you want to continue?"), QMessageBox::Yes | QMessageBox::No);

    // Build the list of files to resize
    QList<QPair<QString, QSize>> files;
    for (int i = 0; i < this->Table->rowCount(); i++) {
        QString filename = this->Table->item(i, COLUMN_FILENAME)->text();
        QSize size       = this->Table->item(i, COLUMN_NEWSIZE)->data(Qt::UserRole).toSize();
        files << QPair<QString, QSize>(filename, size);
    }

    // Start the thread and set UI
    ResizeThread::instance()->resize(files);
    ui->ProgressBar->setMaximum(this->Table->rowCount());
    updateUI();
}

//
//  onFileResizing
//
// Triggered when the resizer thread starts to process a file. Update UI according to
//

void MainWindow::onFileResizing(QString filename)
{
    // Display file related information in the progress bar
    ui->ProgressBar->setValue(ui->ProgressBar->value() + 1);
    ui->ProgressBar->setFormat(QString("%1 (%p%)").arg(filename));
}

//
//  onFileResized
//
// Triggered when a file has been processed
//

void MainWindow::onFileResized()
{
    this->Table->removeRow(0);
}

//
//  onResizingTerminated
//
// Triggered when all files have been resized. Display a dialog to inform the user about operation success or failure.
// Don't display anything if the user wants to close the program while a process is running
//

void MainWindow::onResizingTerminated()
{
    if (!this->CloseRequested) {
        QStringList files = ResizeThread::instance()->invalidFiles();
        if (files.isEmpty()) {
            QMessageBox::information(this, MAIN_WINDOW_TITLE, tr("All files successfully resized!"), QMessageBox::Ok);
        }
        else {
            DlgErrorList::openDlgErrorList(tr("Some files couldn't be resized"), files, this);
        }

        updateUI();
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
//  updateAllSizes
//
// Compute and display the new picture sizes, depending on the current resizing method
//

void MainWindow::updateAllSizes()
{
    for (int i = 0; i < this->Table->rowCount(); i++) {
        QSize orgsize = this->Table->item(i, COLUMN_ORGSIZE)->data(Qt::UserRole).toSize();
        QSize newsize;
        updateSize(orgsize, newsize);
        this->Table->item(i, COLUMN_NEWSIZE)->setText(QString("%1 x %2").arg(newsize.width()).arg(newsize.height()));
        this->Table->item(i, COLUMN_NEWSIZE)->setData(Qt::UserRole, QVariant::fromValue(newsize));
    }
}

//
//  updateSize
//
// Update the given size according to the selected resizing method
//

void MainWindow::updateSize(QSize& orgsize, QSize& newsize)
{
    // Percentage method selected
    if (ui->RadioPercentage->isChecked()) {
        int Percentage = ui->SpinboxPercentage->value();
        newsize.setWidth((orgsize.width() * Percentage) / 100);
        newsize.setHeight((orgsize.height() * Percentage) / 100);
    }
    // Absolute Size method selected
    else {
        int MaxSize = ui->SpinboxAbsoluteSize->value();
        if (orgsize.width() > orgsize.height()) {
            newsize.setHeight((orgsize.height() * MaxSize) / orgsize.width());
            newsize.setWidth(MaxSize);
        }
        else {
            newsize.setWidth((orgsize.width() * MaxSize) / orgsize.height());
            newsize.setHeight(MaxSize);
        }
    }

    // Prevent from getting a null size
    newsize.setWidth(max(newsize.width(), 1));
    newsize.setHeight(max(newsize.height(), 1));
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
// Slot triggered when the user wants to interrupt the dropping or resizing thread
//

void MainWindow::cancelTask()
{
    if (DropThread::instance()->isRunning()) {
        DropThread::instance()->requestInterruption();
    }

    if (ResizeThread::instance()->isRunning()) {
        ResizeThread::instance()->requestInterruption();
    }
}

//
//  closeEvent
//
// Overload closeEvent method to allow proper closing while processing files
//

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (DropThread::instance()->isRunning() || ResizeThread::instance()->isRunning()) {
        auto answer = QMessageBox::question(this, MAIN_WINDOW_TITLE, tr("Do you want to interrupt current process ?"), QMessageBox::Yes | QMessageBox::No);
        if (answer == QMessageBox::Yes) {
            // Request interruption of both thread, don't care about which one is running
            DropThread::instance()->requestInterruption();
            ResizeThread::instance()->requestInterruption();
            // Wait until threads have stopped
            while (DropThread::instance()->isRunning() || ResizeThread::instance()->isRunning())
                ;
            // Prevent tome dialogs to pop up on exit
            this->CloseRequested = true;
            event->accept();
        }
        else {
            // User doesn't want to stop
            event->ignore();
        }
    }
    else {
        // Nothing do to if no thread is running
        event->accept();
    }
}
