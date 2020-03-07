#include <QMimeData>
#include "Dropbox.hpp"

//
//  dragEnterEvent
//
// Tell the Dropbox to accept only file drops
//

void Dropbox::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

//
//  dropEvent
//
// Send a signal caught by the MainWindow
//

void Dropbox::dropEvent(QDropEvent *event)
{
    emit picturesDropped(event->mimeData()->urls());
}
