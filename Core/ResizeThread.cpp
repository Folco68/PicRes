/*
 * PicRes - GUI program to resize pictures in an easy way
 * Copyright (C) 2020-2025 Martial Demolins AKA Folco
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

void ResizeThread::release()
{
    if (resizethread != nullptr) {
        delete resizethread;
        resizethread = nullptr;
    }
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
        QPair<QString, QSize> File = this->Files.at(i);

        // Read data, and emit a signal to UI
        QString Filename = File.first;
        QSize   Size     = File.second;
        emit resizingFile(Filename);

        // Open image
        QImage Image(Filename);

        // Resize the image
        QImage ResizedImage = Image.scaled(Size.width(), Size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (ResizedImage.isNull() || !ResizedImage.save(Filename)) {
            this->InvalidFiles << Filename;
        }

        // Tell the UI that a file has been processed
        emit fileResized();

        // Terminate if cancellaation has been requested
        if (isInterruptionRequested()) {
            break;
        }
    }

    // Tell the UI that process is terminated
    if (isInterruptionRequested()) {
        emit resizingAborted();
    }
    else {
        emit resizingTerminated();
    }
}

QStringList ResizeThread::invalidFiles() const
{
    return this->InvalidFiles;
}
