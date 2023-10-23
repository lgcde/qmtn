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

#include <QMessageBox>
#include "imgup.h"
/*****************************************************************************/
ImgUp::ImgUp(QWidget *parent, QString filePath):
    QDialog(parent),
    mainWindow(parent),
    imageFilePath (filePath)
{
    nm = new QNetworkAccessManager(this);
    layout = new QHBoxLayout(this);
#ifdef QT_WEBENGINEWIDGETS_LIB
    webEngine = new QWebEngineView(this);
    layout->addWidget(webEngine);
#else
    webBrowser = new QTextEdit(this);
    webBrowser->setReadOnly(true);
    layout->addWidget(webBrowser);
#endif
    setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(serverName);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    resize(parent->size() * 0.8);
    setVisible(false);
}
/*****************************************************************************/
void ImgUp::setImagePath(QString imagePath)
{
    imageFilePath = imagePath;
}
/*****************************************************************************/
QString ImgUp::hostName()
{
    return serverName;
}
/*****************************************************************************/
void ImgUp::upload()
{
    if(QFile::exists(imageFilePath))
        postUploadRequest();
    else
        showErr(tr("File '%1' not found!").arg(imageFilePath));
}
/*****************************************************************************/
void ImgUp::showErr(QString msg)
{
    QMessageBox::critical(mainWindow, tr("Error"), msg);
}
/*****************************************************************************/
