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

    sudo apt-get install qt5-default qtbase5-dev qtwebengine5-dev

Fedora

    dnf install qt6-qtbase-devel qt6-qtwebengine-devel

Mageia

    sudo dnf install lib64qt5webengine-devel

Get source
==========

    git clone https://gitlab.com/movie_thumbnailer/qmtn.git


Configuration
=============
Optional Qt configuration:

use_git_version - gets the version number based on git tag  
use_webengine - allows to view the upload image result web page  

    CONFIG += use_git_version use_webengine

Build
=====
    cd qmtn
    qmake6 src
    make


Install
=======
    sudo make install

Links
=====
* [Qmtn homepage](https://gitlab.com/movie_thumbnailer/qmtn/wikis)
* [Qt versions support](https://wiki.qt.io/Main)
