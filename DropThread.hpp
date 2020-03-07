/*
 * PicRes - GUI program to resize pictures in an easy way
 * Copyright (C) 2020 Martial Demolins AKA Folco
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
