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

#include "ResizeThread.hpp"
#include <QImage>

ResizeThread* ResizeThread::resizethread = nullptr;

ResizeThread* ResizeThread::instance()
{
    if (resizethread == nullptr) {
        resizethread = new ResizeThread;
    }
    return resizethread;
}

void ResizeThread::resize(QList<QPair<QString, QSize>> files)
{
    this->Files = files;
    start();
}

void ResizeThread::run()
{
    // Clear the list of files that we failed to resize
    this->InvalidFiles.clear();

    // Resize images
    for (int i = 0; i < this->Files.count(); i++) {
        QPair<QString, QSize> file = this->Files.at(i);

        // Read data, and emit a signal to UI
        QString filename = file.first;
        QSize size       = file.second;
        emit resizingFile(filename);

        // Open image
        QImage image(filename);

        // Resize the image
        QImage ResizedImage = image.scaled(size.width(), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (ResizedImage.isNull() || !ResizedImage.save(filename)) {
            this->InvalidFiles << filename;
        }

        // Tell the UI that a file has been processed
        emit fileResized();

        // Terminate if cancellaation has been requested
        if (isInterruptionRequested()) {
            break;
        }
    }

    // Tell the UI that all files have been processed
    emit resizingTerminated();
}

QStringList ResizeThread::invalidFiles() const
{
    return this->InvalidFiles;
}
