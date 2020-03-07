#ifndef DROPTHREAD_HPP
#define DROPTHREAD_HPP

#include <QList>
#include <QMutex>
#include <QSize>
#include <QString>
#include <QThread>
#include <QUrl>

//
//  DropThread
//
// This class is a worker thread that retrieve data of files dropped in the UI
//

class DropThread : public QThread
{
    Q_OBJECT

public:
    static DropThread* instance();                     // Return a ptr to the object instance; create it if needed
    void drop(QList<QUrl> URLs);                       // Called when the main UI receives files
    void result(QList<QPair<QString, QSize>>* Result); // Gives the result processed by the worker thread

private:
    void run() override; // Worker

    static DropThread* dropthread;       // Singleton pointer
    QList<QUrl> Queue;                   // Store the URLs dropped into the UI
    QList<QPair<QString, QSize>> Result; // Store the result of the worker thread
    QMutex MutexQueue;                   // Control access to the queue list
    QMutex MutexResult;                  // Control access to the result list

signals:
    void dropResultReady();                       // One or several results are available for the main window
    void processingDroppedFile(QString filename); // The worker start to process a file
    void dropProcessTerminaded();                 // Nothing more to handle, worker stops
};

#endif // DROPTHREAD_HPP
