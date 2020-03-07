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

class Dropbox : public QGroupBox
{
    Q_OBJECT

public:
    explicit Dropbox(QWidget* parent) : QGroupBox(parent) {} // Ctor is mMandatory because the UI factory creates the groupbox with a parent

private:
    void dragEnterEvent(QDragEnterEvent* event) override; // Called when something is dragged over the object
    void dropEvent(QDropEvent* event) override;           // Called when something is dropped on the object

signals:
    void picturesDropped(QList<QUrl> URLs); // Sent to tell the MainWindow that files have been dropped
};

#endif // DROPBOX_HPP
