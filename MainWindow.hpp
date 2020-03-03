#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP


#include <QUrl>
#include <QList>
#include <QByteArray>
#include <QMainWindow>
#include <QTableWidget>


namespace Ui {
    class MainWindow;
}


class MainWindow:
    public QMainWindow
{
        Q_OBJECT

    public:
        // Ctor/dtor
        MainWindow(int argc, char** argv);
        ~MainWindow();

    private:
        // UI
        Ui::MainWindow *ui;
        QTableWidget* Table;
        QList<QByteArray> SupportedExtensionList;   // List of supported picture extension
        bool TableAlreadyShown;

        // Slots used by connections
        void onResizingMethodChanged();
        void onButtonResizeClicked(bool);
        void onPicturesDropped(QList<QUrl> URLs);
        void onSpinBoxValueChanged();

        // Method computing the new dimensions of a picture, according to the selected resizing method
        void updateDimensions(int& width, int& height);
};


//
//  Column index
//

#define COLUMN_FILENAME 0
#define COLUMN_WIDTH    1
#define COLUMN_HEIGHT   2
#define COLUMN_NEWSIZE  3
#define COLUMN_COUNT    4


//
// max() macro
//

#define max(a,b) (a < b ? b : a)


#endif // MAINWINDOW_HPP
