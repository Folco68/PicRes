    /***************************************************************************
     *                                                                         *
     *                                 PicRes                                  *
     *                                                                         *
     *                         Batch pictures resizer                          *
     *                                                                         *
     **************************************************************************/


About
-----

Author:                           Martial Demolins
Mail:                             PLACEHOLDER_EMAIL_PERSONAL, PLACEHOLDER_EMAIL_PROFESSIONAL
Position:                         PLACEHOLDER_POSITION_STR

License:                          GPL v3. You can find it at https://www.gnu.org/licenses/gpl-3.0.en.html
Copyright:                        PLACEHOLDER_COPYRIGHT_STR
Version:                          PLACEHOLDER_APPLICATION_VERSION_STR
Language:                         C++
Framework:                        Qt PLACEHOLDER_QT_VERSION_STR
Source and binary repository:     https://github.com/Folco68/PicRes
Changelog:                        https://github.com/Folco68/PicRes/commits?author=Folco68


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

This version separates UI factory and data processing: file drop and resizing are handled in external threads,
which allows to keep UI smooth, usable, while processes are interruptable and the program closable.
This is especially usefull when working with big remote files.
