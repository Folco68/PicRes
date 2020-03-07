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
