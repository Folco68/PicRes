#include "DropThread.hpp"
#include <QImageReader>

DropThread* DropThread::dropthread = nullptr;

DropThread* DropThread::instance()
{
    if (dropthread == nullptr) {
        dropthread = new DropThread;
    }
    return dropthread;
}

void DropThread::drop(QList<QUrl> URLs)
{
    // Add the new URLs to the queue when it's available
    this->MutexQueue.lock();
    this->Queue << URLs;
    this->MutexQueue.unlock();

    // Start the thread (harmless if already started)
    DropThread::instance()->start();
}

void DropThread::run()
{
    // Run while queue contains files to handle
    // Queue may be filled externally during process
    while (!isInterruptionRequested()) {
        this->MutexQueue.lock();
        // Nothing to do if no URL is pending, the thread may terminate
        if (this->Queue.isEmpty()) {
            this->MutexQueue.unlock();
            break;
        }

        // Take the first URL and get filename
        QUrl url = Queue.takeFirst();
        this->MutexQueue.unlock();
        QString filename(url.toLocalFile());

        // Emit a signal to say to the main UI which file is being processed²
        emit processingDroppedFile(filename);

        // Add the picture filename and its size to the result list. Size is invalid if the picture couldn't be read
        QImageReader image(filename);
        QSize size(image.canRead() ? image.size() : QSize());
        this->MutexResult.lock();
        this->Result << QPair<QString, QSize>(filename, size);
        this->MutexResult.unlock();

        // Emit a signal to say that at least one result is available
        emit dropResultReady();
    }

    // Clear the queue in case of process interruption
    this->MutexQueue.lock();
    this->Queue.clear();
    this->MutexQueue.unlock();

    // Emit a signal saying to the main UI that dropping is complete
    emit dropProcessTerminaded();
}

// May give an empty list
void DropThread::result(QList<QPair<QString, QSize>>* Result)
{
    this->MutexResult.lock();
    *Result = this->Result;     // Available results are copied into the caller variable
    this->Result.clear();       // And discarded from this object
    this->MutexResult.unlock(); //
}
