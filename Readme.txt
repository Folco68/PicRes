PicRes by Martial Demolins
Version: 2.1
License: GPL v3+ (https://www.gnu.org/licenses/gpl-3.0.txt)
Source + download page: https://github.com/Folcogh/PicRes (or, for TP users, \Partage_Techniciens\Demolins\Softs\PicRes)
Bug reports and suggestions: martial <dot> demolins <at> gmail <dot> com


How to
======

This program is intended to resize multiple images at once, in an easy way:
1. drag and drop files
2. set resizing method (absolute or relative)
3. click "Resize"

/!\ Warning: resized files replace original ones /!\


Features
========

- Supported image formats: BMP, JPG, JPEG, PNG, CUR, ICNS, ICO, PPM, SVG, SVGZ, TGA, TIF, WBMP, WEBP, XBM, XPM
- You can drop files multiple times before resizing, making drop from multiple locations easy

With its multi-threaded design, version 2 brings several new features:
- file list may be cleared (Clear List button)
- file count is displayed in the Clear List button
- both drop and resize processes may be interrupted properly (Cancel button)
- you can drop files even when previous drop handling is not temrinated (useful when working with remote pictures)


Under the hood
==============

Written in C++, statically built on Windows against Qt 5.12.3.
Sources are provided with a .pro file, so you can easily rebuild the program.
See https://www.qt.io/ for infos.

This version separates UI factory and data processing: file drop and resizing are handled in external threads,
which allows to keep UI smouth, usable, while processes are interruptable and the program closable.
This is especially usefull when working with big remote files.
