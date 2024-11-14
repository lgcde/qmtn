/* Qt Movie Thumbnailer - movie screencast generator

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef IMGPI_H
#define IMGPI_H

#include <QObject>
#include "imgup.h"

namespace imguploader {
class ImgPI;
}

class ImgPI : public ImgUp
{
   Q_OBJECT
    QByteArray removeAfterDays;

public:
    ImgPI(QWidget *parent, QString filePath=QString());

protected:
    virtual void postUploadRequest() Q_DECL_OVERRIDE;
    void showUploadPage(QString pagePath);

private slots:
    void imageUploaded();
};

#endif // IMGPI_H
