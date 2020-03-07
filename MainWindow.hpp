#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QByteArray>
#include <QList>
#include <QMainWindow>
#include <QStringList>
#include <QTableWidget>
#include <QUrl>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        MainWindow(int argc, char** argv);
        ~MainWindow();

    private:
        // UI
        Ui::MainWindow *ui;
        QTableWidget* Table;                        // Main table, contaning filenames and size informations
        QList<QByteArray> SupportedExtensionList;   // List of supported picture extension
        QStringList InvalidDroppedFiles;            // Files that cannot be processed when they are dropped into the UI

        void updateSize(int& width, int& height); // Compute the new dimensions of a picture, according to the selected resizing method
        void updateUI();                          // Update UI, depending on program state

        // Slots linked to UI
        void clearTable();                        // Remove all entries imported in the main table
        void cancelTask();                        // Cancel on the flight file dropping or resizing
        void onResizingMethodChanged();           // Called when resizing method changes, to refresh new sizes
        void onButtonResizeClicked();             // Start the main worker of this program
        void onPicturesDropped(QList<QUrl> URLs); // Called when the UI receives files
        void onSpinBoxValueChanged();             // Called when resizing values change, to update new sizes

        // Slots linked to drop thread
        void onDropResultReady();                      // Says to the main window that data of a picture is ready to use
        void onDroppedFileProcessed(QString filename); // Emitted when processed file changes
        void onDropProcessTerminated();                // Says to the main window that all dropped files have been processed
};

//
//  Column index
//

#define COLUMN_FILENAME 0
#define COLUMN_OLDSIZE 1
#define COLUMN_NEWSIZE 2
#define COLUMN_COUNT 3

//
// max() macro
//

#define max(a,b) (a < b ? b : a)

#endif // MAINWINDOW_HPP
