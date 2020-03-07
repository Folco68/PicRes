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

    // Center some widgets
    centralWidget()->layout()->setAlignment(ui->ButtonResize, Qt::AlignHCenter);
    ui->HLayoutPercentage->setAlignment(ui->SpinboxPercentage, Qt::AlignHCenter);
    ui->HLayoutAbsoluteSize->setAlignment(ui->SpinboxAbsoluteSize, Qt::AlignHCenter);

    // Set progress bar
    ui->ProgressBar->setAlignment(Qt::AlignCenter);
    ui->ProgressBar->setMinimum(0);

    // Configure the table displaying the dropped files
    // The table is created here and not with the WYSIWYG tool, because I prefer to configure it by hand
    QStringList Labels;
    Labels << "File name"
           << "Actual size"
           << "New size";

    Table->setColumnCount(COLUMN_COUNT);
    Table->setShowGrid(true);
    Table->setSortingEnabled(true);
    Table->setAlternatingRowColors(true);
    Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Table->setSelectionMode(QAbstractItemView::SingleSelection);
    Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    Table->horizontalHeader()->setStretchLastSection(true);
    Table->verticalHeader()->setVisible(false);
    Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    Table->setHorizontalHeaderLabels(Labels);

    ui->BoxDrop->layout()->addWidget(Table);

    updateUI();

    //
    //  Data
    //

    // Remove extensions supported only in read-only mode by Qt
    SupportedExtensionList.removeOne("GIF");
    SupportedExtensionList.removeOne("PBM");
    SupportedExtensionList.removeOne("PGM");

    //
    //  Connections
    //

    // Enable/disable the spinboxes according to the checked radio buttons. Force new size update
    connect(ui->RadioPercentage, &QRadioButton::clicked, [this]() { onResizingMethodChanged(); });
    connect(ui->RadioPercentage, &QRadioButton::clicked, [this]() { onSpinBoxValueChanged(); });
    connect(ui->RadioAbsoluteSize, &QRadioButton::clicked, [this]() { onResizingMethodChanged(); });
    connect(ui->RadioAbsoluteSize, &QRadioButton::clicked, [this]() { onSpinBoxValueChanged(); });

    // Connect the spinboxes to update the new size
    connect(ui->SpinboxPercentage, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this]() { onSpinBoxValueChanged(); });
    connect(ui->SpinboxAbsoluteSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this]() { onSpinBoxValueChanged(); });

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

// WARNING: doesn't take in account the resize thread
void MainWindow::updateUI()
{
    // Shortcuts for common properties
    bool TableIsEmpty        = this->Table->rowCount() == 0;
    bool DropThreadIsRunning = DropThread::instance()->isRunning();

    ui->ProgressBar->setVisible(DropThreadIsRunning);                       // Progress bar is visible if files are currently dropped
    ui->LabelDrop->setVisible(TableIsEmpty);                                // Hint label is displayed if the table is empty
    this->Table->setVisible(!TableIsEmpty);                                 // Table is displayed if it contains elements
    ui->BoxResizingMethod->setDisabled(TableIsEmpty);                       // Resizing methods are disabled if the table is empty
    ui->ButtonClearList->setVisible(!TableIsEmpty && !DropThreadIsRunning); // Can't clear the list if it's empty or in use
    ui->ButtonCancel->setVisible(DropThreadIsRunning);                      // Cancel button is visible only if a process is running
    ui->ButtonResize->setEnabled(!TableIsEmpty && !DropThreadIsRunning);    // We can resize when there is something to resize and drop process is complete
}

//
//  onButtonResizeClicked
//
// Called when the user wants to resize the selected files
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

void MainWindow::onDropResultReady()
{
    QList<QPair<QString, QSize>> result;
    DropThread::instance()->result(&result);

    for (int i = 0; i < result.size(); i++) {
        QString filename = result.at(i).first;
        QSize size       = result.at(i).second;

        // Add the file to the table if it could be read, else add it to the error list
        if (size.isValid()) {
            // Compute new size
            int NewWidth  = size.width();
            int NewHeight = size.height();
            updateSize(NewWidth, NewHeight);

            // Create the items to add. Use a QTableWidgetItem for the filename, because we don't want it to be centered
            QTableWidgetItem* ItemName = new QTableWidgetItem(filename);
            //            TableItem* ItemOldSize     = new TableItem(QString("%1 x %2").arg(size.width()).arg(size.height()));
            TableItem* ItemOldSize = new TableItem();
            ItemOldSize->setData(Qt::DisplayRole, size);
            TableItem* ItemNewSize = new TableItem(QString("%1 x %2").arg(NewWidth).arg(NewHeight));

            // Populate the table
            int row = this->Table->rowCount();
            this->Table->insertRow(row);
            this->Table->setItem(row, COLUMN_FILENAME, ItemName);
            this->Table->setItem(row, COLUMN_OLDSIZE, ItemOldSize);
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

void MainWindow::onDroppedFileProcessed(QString filename)
{
    ui->ProgressBar->setValue(ui->ProgressBar->value() + 1);       // Update percentage value
    ui->ProgressBar->setFormat(QString("%1 (%p%)").arg(filename)); // Write filename + percentage in the progressbar
}

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
//  onSpinBoxValueChanged
//
// Update new sizes in the tables
//

void MainWindow::onSpinBoxValueChanged()
{
    for (int i = 0; i < Table->rowCount(); i++) {
        QSize size = this->Table->item(i, COLUMN_OLDSIZE)->data(Qt::DisplayRole).toSize();
        int width  = size.width();
        int height = size.height();
        updateSize(width, height);
        Table->item(i, COLUMN_NEWSIZE)->setText(QString("%1 x %2").arg(width).arg(height));
    }
}


//
//  onResizingMethodChanged
//
// Called when the Percentage or the Absolute Size radio button is clicked
// Enable/disable the spinboxes according to the radio buttons
//

void MainWindow::onResizingMethodChanged()
{
    ui->SpinboxPercentage->setEnabled(ui->RadioPercentage->isChecked());
    ui->SpinboxAbsoluteSize->setEnabled(!ui->RadioPercentage->isChecked());
}


//
//  updateDimensions
//
// Update the given dimensions according to the selected resizing method
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

void MainWindow::clearTable()
{
    Table->clearContents();
    Table->setRowCount(0);
    updateUI();
}

void MainWindow::cancelTask()
{
    if (DropThread::instance()->isRunning()) {
        DropThread::instance()->requestInterruption();
    }
}
