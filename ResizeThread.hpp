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

#ifndef RESIZETHREAD_HPP
#define RESIZETHREAD_HPP

#include <QList>
#include <QPair>
#include <QSize>
#include <QString>
#include <QThread>

class ResizeThread : public QThread
{
    Q_OBJECT

public:
    static ResizeThread* instance();                 // Return a pointer to the object instance. Create the instance if needed
    void resize(QList<QPair<QString, QSize>> files); // Called when the Resize button is clicked
    QStringList invalidFiles() const;

private:
    static ResizeThread* resizethread; // Singleton instance pointer
    void run() override;               // Thread worker

    QList<QPair<QString, QSize>> Files; // Contain a description of the files that have to be resized
    QStringList InvalidFiles;           // Contain the list of the files which couldn't be resized

signals:
    void resizingFile(QString filename); // Emitted the name of the file whose resizing process starts
    void fileResized();                  // Emitted when a file resizing is terminated (successfully or not)
    void resizingTerminated();           // Emitted when all files have been resized-+
};

#endif // RESIZETHREAD_HPP
