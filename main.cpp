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

#include "UI/MainWindow.hpp"
#include <QApplication>
#include <QGuiApplication>
#include <QIcon>

//  main
//
// Create the MainWindow, show it and execute it
//

int main(int argc, char* argv[])
{
    QApplication Application(argc, argv);
    QGuiApplication::setWindowIcon(QIcon(":/Main/Icon.png"));
    MainWindow Window(argc, argv); // Handle files dropped on the program icon (or passed from CLI)
    Window.show();
    return Application.exec();
}
