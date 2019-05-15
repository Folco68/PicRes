#ifndef DROPBOX_HPP
#define DROPBOX_HPP


#include <QUrl>
#include <QList>
#include <QGroupBox>
#include <QDropEvent>
#include <QDragEnterEvent>


//
//  Dropbox
//
// This class is used to override two methods of the QGroupBox object, to handle drag and drop
//

class Dropbox:
    public QGroupBox
{
        Q_OBJECT

    public:
        // Ctor. Mandatory because the UI factory creates the groupbox with a parent
        explicit Dropbox(QWidget* parent = nullptr): QGroupBox(parent) {}

    private:
        // Overloaded methods to handle drops
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dropEvent(QDropEvent* event) override;


    signals:
        // Sent to tell the MainWindow that files have been dropped
        void picturesDropped(QList<QUrl> URLs);
};


#endif // DROPBOX_HPP
