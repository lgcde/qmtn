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

#include <QFileInfo>
#include <QHttpMultiPart>
#include "klikr.h"
/*****************************************************************************/
Klikr::Klikr(QWidget *parent, QString filePath):
    ImgUp(parent, filePath)
{
    serverName="www.klikr.org";
    serverScheme="http";
}
/*****************************************************************************/
void Klikr::postUploadRequest()
{
    QFileInfo fi(imageFilePath);
    const QString fileName = fi.fileName();
    const QString filePath = fi.filePath();
    const QString filesuffix = fi.suffix().toLower();
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    const QByteArray boundary = multiPart->boundary();

    QNetworkRequest req;
    QUrl url;
    QStringList allowedSuff; allowedSuff << "jpeg" << "jpg";
    QString contentType = QString("image/%1").arg(fi.suffix().toLower());

    if(!allowedSuff.contains(filesuffix))
    {
        showErr("Image types allowed: "+allowedSuff.join(", "));
        reject();
        return;
    }

    url.setScheme(serverScheme);
    url.setHost(serverName);
    url.setPath("/upload.php");

    req.setUrl(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("multipart/form-data; boundary=" + boundary));

    QHttpPart userfilePart;
    userfilePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"imagedata\"; filename=\"%1\"").arg(fileName));
    userfilePart.setHeader(QNetworkRequest::ContentTypeHeader,  contentType);
    QFile *imgFile = new QFile(filePath);
    imgFile->open(QIODevice::ReadOnly);
    userfilePart.setBodyDevice(imgFile);
    imgFile->setParent(multiPart); // we cannot delete the imgFile now, so delete it with the multiPart
    multiPart->append(userfilePart);

    uploadReply = nm->post(req, multiPart);
    multiPart->setParent(uploadReply);    // delete the multiPart with the reply

    connect(uploadReply,   &QNetworkReply::finished, this, &Klikr::imageUploaded);
}
/*****************************************************************************/
void Klikr::imageUploaded()
{
    QNetworkReply *reply = uploadReply;
    const int successCode = 302;

    if(reply->error() == QNetworkReply::NoError)
    {
        if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == successCode)
        {
            auto resultLink = reply->header(QNetworkRequest::LocationHeader).toString();
            auto htmltext = QString("<div align=center><img height=50 src=\":/icons/image.svg\"><br><h2>Image has been uploaded.</h2><br><a href=\"%1\">%1</a></div>").arg(resultLink);

        #ifdef QT_WEBENGINEWIDGETS_LIB
            webEngine->setHtml(htmltext);
        #else
            webBrowser->setHtml(htmltext);
        #endif
            showUploadPage(QString());
        }
        else
            showErr(QString("%1\nPage code != %2").arg(tr("Something went wrong!")).arg(successCode));
    }
    else
        showErr(reply->errorString());

    reply->deleteLater();
}
/*****************************************************************************/
void Klikr::showUploadPage(QString)
{
    show();
}
/*****************************************************************************/
