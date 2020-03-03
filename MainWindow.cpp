#include "MainWindow.hpp"
#include "DlgErrorList.hpp"
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
    , TableAlreadyShown(false)
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
    ui->ProgressBar->setVisible(false);
    ui->ProgressBar->setAlignment(Qt::AlignCenter);

    // Configure the table displaying the dropped files
    // The table is created here and not with the WYSIWYG tool, because I prefer to configure it by hand
    QStringList Labels;
    Labels << "File name" << "Width" << "Height" << "New size";

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
    Table->setVisible(false);

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
    connect(ui->RadioPercentage, &QRadioButton::clicked, this, &MainWindow::onResizingMethodChanged);
    connect(ui->RadioPercentage, &QRadioButton::clicked, this, &MainWindow::onSpinBoxValueChanged);
    connect(ui->RadioAbsoluteSize, &QRadioButton::clicked, this, &MainWindow::onResizingMethodChanged);
    connect(ui->RadioAbsoluteSize, &QRadioButton::clicked, this, &MainWindow::onSpinBoxValueChanged);

    // Connect the spinboxes to update the new size
    connect(ui->SpinboxPercentage, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onSpinBoxValueChanged);
    connect(ui->SpinboxAbsoluteSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onSpinBoxValueChanged);

    // Connect the process button
    connect(ui->ButtonResize, &QPushButton::clicked, this, &MainWindow::onButtonResizeClicked);

    // Connect the main window to the signal emitted by the dropbox
    connect(ui->BoxDrop, &Dropbox::picturesDropped, this, &MainWindow::onPicturesDropped);

    // If files were dropped on the program icon, add them to the UI
    if (argc != 1) {
        QList<QUrl> urls;
        for (int i = 1; i < argc; i++) {
            urls << QUrl::fromLocalFile(argv[i]);
        }

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
//  onButtonResizeClicked
//
// Called when the user wants to resize the selected files
//

void MainWindow::onButtonResizeClicked(bool)
{
    // Show and set the progress bar
    ui->ProgressBar->setVisible(true);
    ui->ProgressBar->setRange(0, Table->rowCount());
    ui->ProgressBar->reset();

    // Disable some UI elements, to avoid modifications during resizing process
    ui->RadioPercentage->setDisabled(true);
    ui->RadioAbsoluteSize->setDisabled(true);
    ui->SpinboxPercentage->setDisabled(true);
    ui->SpinboxAbsoluteSize->setDisabled(true);
    ui->ButtonResize->setDisabled(true);

    // List of filename that couldn't be resized
    QStringList InvalidFiles;

    // Resize images
    for (int i = 0; i < Table->rowCount(); i++) {
        // Get filename
        QString Filename = Table->item(i, COLUMN_FILENAME)->data(Qt::DisplayRole).toString();

        // Refresh progress bar
        ui->ProgressBar->setValue(i + 1);
        ui->ProgressBar->setFormat(tr("Resizing file: %1").arg(Filename));
        QCoreApplication::processEvents();

        // Open image
        QImage Image(Filename);

        // Get dimensions and update them
        int Width = Table->item(i, COLUMN_WIDTH)->data(Qt::DisplayRole).toInt();
        int Height = Table->item(i, COLUMN_HEIGHT)->data(Qt::DisplayRole).toInt();
        updateDimensions(Width, Height);

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
    TableAlreadyShown = false;
    Table->clearContents();
    Table->setRowCount(0);
    Table->setVisible(false);
    ui->LabelDrop->setVisible(true);
    ui->ProgressBar->setVisible(false);
}


//
//  onPicturesDropped
//
// Slot triggered when files are dropped in the drop box
//

void MainWindow::onPicturesDropped(QList<QUrl> URLs)
{
    //    ui->LabelDrop->setText(URLs.at(0).toLocalFile());
    bool OnePictureValid = false;
    QStringList InvalidFiles;

    // Show and set the progress bar
    ui->ProgressBar->setVisible(true);
    ui->ProgressBar->setRange(0, URLs.size());
    ui->ProgressBar->reset();

    // Browse the list to add valid picture files
    for (int i = 0; i < URLs.size(); i++) {
        // Read filename
        QString filename(URLs.at(i).toLocalFile());

        // Refresh progress bar
        ui->ProgressBar->setValue(i + 1);
        ui->ProgressBar->setFormat(tr("Reading file data: %1").arg(filename));
        QCoreApplication::processEvents();

        // Don't add the file twice if it's already part of the list
        bool FileAlreadyPresent = false;
        for (int i = 0; i < Table->rowCount(); i++) {
            if (filename == Table->item(i, COLUMN_FILENAME)->data(Qt::DisplayRole).toString()) {
                FileAlreadyPresent = true;
                break;
            }
        }
        if (FileAlreadyPresent) {
            continue;
        }

        // Open image
        QImageReader image(filename);

        // Add the picture to the table if it's a valid one
        if (image.canRead()) {
            OnePictureValid = true;

            // Get actual and new dimensions
            QSize Dimensions(image.size());
            int NewWidth = Dimensions.width();
            int NewHeight = Dimensions.height();
            updateDimensions(NewWidth, NewHeight);

            // Create the items to add. Use a QTableWidgetItem for the filename, because we don't want it to be centered
            QTableWidgetItem* ItemName = new QTableWidgetItem(filename);
            TableItem* ItemWidth = new TableItem(QString("%1").arg(Dimensions.width()));
            TableItem* ItemHeight = new TableItem(QString("%1").arg(Dimensions.height()));
            TableItem* ItemNewSize = new TableItem(QString("%1 x %2").arg(NewWidth).arg(NewHeight));

            // Populate the table
            int Row = Table->rowCount();
            Table->insertRow(Row);
            Table->setItem(Row, COLUMN_FILENAME, ItemName);
            Table->setItem(Row, COLUMN_WIDTH, ItemWidth);
            Table->setItem(Row, COLUMN_HEIGHT, ItemHeight);
            Table->setItem(Row, COLUMN_NEWSIZE, ItemNewSize);
        }
        // The file can't be opened as an image, add its name to the reject list
        else {
            InvalidFiles << filename;
        }
    }

    // Remove progress bar
    ui->ProgressBar->setVisible(false);

    // Update UI if no file was dropped yet and if at least one file is valid
    if (OnePictureValid && !TableAlreadyShown) {
        TableAlreadyShown = true;

        // Remove the hint text and show the table
        ui->LabelDrop->setVisible(false);
        Table->setVisible(true);

        // Enable some settings
        ui->RadioAbsoluteSize->setEnabled(true);
        ui->RadioPercentage->setEnabled(true);
        ui->ButtonResize->setEnabled(true);

        // Enable the right spinbox
        onResizingMethodChanged();
    }

    // Display an error message if some files couldn't be opened
    if (!InvalidFiles.isEmpty()) {
        DlgErrorList Dialog(tr("Some files couldn't be opened:"), InvalidFiles);
        Dialog.exec();
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
        int Width = Table->item(i, COLUMN_WIDTH)->data(Qt::DisplayRole).toInt();
        int Height = Table->item(i, COLUMN_HEIGHT)->data(Qt::DisplayRole).toInt();
        updateDimensions(Width, Height);
        Table->item(i, COLUMN_NEWSIZE)->setText(QString("%1 x %2").arg(Width).arg(Height));
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

void MainWindow::updateDimensions(int& width, int& height)
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
