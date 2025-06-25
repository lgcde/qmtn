Description
============

GUI Thumbnail Generator Using CLI [mtn](https://gitlab.com/movie_thumbnailer/mtn/wikis/home/).

Aplication focuses on creating a thumbnails-grid using drag&drop.

Features:

- Drag&drop files and folders
- Recursive search for movie files
- Instant image making in background
- Image and output log preview
- Open image in Fullscreen or and external image viewer
- Recreate image with new settings
- Managing mtn switches
- Settings profiles
- Extracting album art


Dependencies
============

Debian/Ubuntu/Mint

    sudo apt-get install qmake6 qt6-base-dev qt6-image-formats-plugins qt6-webengine-dev

Fedora

    dnf install qt6-qtbase-devel

Mageia

    sudo dnf install qtbase6-common-devel

Get source
==========

    git clone https://gitlab.com/movie_thumbnailer/qmtn.git


Configuration
=============

Optional Qt configuration:

use_git_version - gets the version number based on git tag  
use_webengine - may be useful for plugins for uploading images

    CONFIG += use_git_version use_webengine

Build
=====

    cd qmtn
    qmake6 src
    make


Install
=======

    sudo make install

Application settings
====================

Users normally expect an application to remember its settings (window sizes and positions, 
options, etc.) across sessions. This information is often stored in the system registry 
on Windows. On Unix systems, many applications (including the KDE applications) use INI 
text files. Typical file path is `$HOME/.config/Rusty Pipe/Qt Movie Thumbnailer.ini`.  

These settings are not changed by the application (user may change it manually):  
  
    [General]  
    MaxRecentFiles - Number of files/folders recently opened showing in menu "File"  
    VideoExtension - Comma separated list of file extensions allowed to process (usefull when processing folders)

Links
=====

* [Qmtn homepage](https://gitlab.com/movie_thumbnailer/qmtn/wikis)
* [Qt versions support](https://wiki.qt.io/Main)
