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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QByteArray>
#include <QCloseEvent>
#include <QList>
#include <QMainWindow>
#include <QSize>
#include <QStringList>
#include <QTableWidget>
#include <QUrl>

//
//  MainWindow
//
// This class handles the main interface
//

namespace Ui {
    class MainWindow;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(int argc, char* argv[]);
    ~MainWindow() override;

  private:
    // UI
    Ui::MainWindow*   ui;
    QTableWidget*     Table;                  // Main table, contaning filenames and size informations
    QList<QByteArray> SupportedExtensionList; // List of supported picture extension
    QStringList       InvalidDroppedFiles;    // Files that cannot be processed when they are dropped into the UI
    bool              CloseRequested;         // True if close is requested, preventing some dialogs to pop up

    void updateSize(QSize& orgsize, QSize& newsize);     // Compute the new dimensions of a picture, according to the selected resizing method
    void updateAllSizes();                               // Update sizes displayed in the table
    void updateUI();                                     // Update UI, depending on program state
    void closeEvent(QCloseEvent* event) override;        // Intercept close event to allow program termination while a thread is running
    void getFiles(QList<QUrl>& list, QUrl dirurl) const; // Return the list of the files contained in a directory

    // Slots linked to UI
    void clearTable();                       // Remove all entries imported in the main table
    void cancelTask();                       // Cancel on the flight file dropping or resizing
    void onResizingMethodChanged();          // Called when resizing method changes, to refresh new sizes
    void onButtonResizeClicked();            // Start the main worker of this program
    void onPicturesDropped(QList<QUrl> url); // Called when the UI receives files
    void onPercentageValueChanged();         // Called when resizing values change, to update new sizes
    void onAbsoluteValueChanged();           // Called when resizing values change, to update new sizes

    // Slots linked to drop thread
    void onDropResultReady();                      // Triggered when picture data is ready to use
    void onDroppedFileProcessed(QString filename); // Triggered when processed file changes
    void onDropProcessTerminated();                // Triggered when all dropped files have been handled
    void onFileResizing(QString filename);         // Triggered when a file resizing starts
    void onFileResized();                          // Triggered when a file have been resized
    void onResizingTerminated();                   // Triggered when resizing of all files is done
    void onResizingAborted();                      // Trigerred when the resizing process is aborted by user
};

//
//  Column index
//

#define COLUMN_FILENAME 0
#define COLUMN_ORGSIZE  1
#define COLUMN_NEWSIZE  2
#define COLUMN_COUNT    3

//
// max() macro
//

#define max(a, b) (a < b ? b : a)

#endif // MAINWINDOW_HPP
