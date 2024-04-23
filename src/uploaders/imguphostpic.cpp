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

#include "imguphostpic.h"
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QDebug>
#include <QNetworkCookieJar>
#include <QNetworkCookie>


/*****************************************************************************/
ImgUpHostPic::ImgUpHostPic(QWidget *parent, QString filePath):
    ImgUp(parent, filePath)
{
    serverScheme="https";
    serverName="www.hostpic.org";
    serverUploadPath = "/inc/uploader.php";
    serverViewPath = "/view.php";
    serverViewQuery = "filename=%1";
    serverDirectPath = "/images/%1";
    privateUpload = true;
}

/*****************************************************************************/
void ImgUpHostPic::postUploadRequest()
{
    QNetworkRequest pagereq;
    QUrl u;
    u.setScheme(serverScheme);
    u.setHost(serverName);
    pagereq.setUrl(u);

    qDebug() << "Uploading to" << serverName << "...";
    pagereply = nm->get(pagereq);
    pagereply->ignoreSslErrors();
    connect(pagereply,   &QNetworkReply::finished, this, &ImgUpHostPic::gotPageForCookie);
}

/*****************************************************************************/
QString ImgUpHostPic::imageNameFromReplyData(QByteArray ReplyData)
{
    QString Filename = "";

    auto splitted = ReplyData.split(',');

    if(splitted.count() == 12)
    {
        //splitted[3] = "'|2404222000110096.jpg'"
        Filename = QString(splitted[3]).mid(2,-1).chopped(1);
    }
    else
    {
        qDebug() << "Got reply:" << ReplyData;
        qDebug() << "Expected:" << "<script language='javascript'>parent.upload('','|1551438','','|2404221946590096.jpg','|/home/hostpic.org/public_html/images/2404221946590096.jpg','|https://www.hostpic.org/images/2404221946590096.jpg','|2404221946590096.jpg','|https://www.hostpic.org/images/2404221946590096_tn.jpg','https://www.hostpic.org','https://www.hostpic.org','Host Pic.Org - Free Image Picture Photo Hosting','1');</script>";
    }

    return Filename;
}

/*****************************************************************************/
void ImgUpHostPic::sendFile()
{
    QFileInfo fi(imageFilePath);
    const QString fileName = fi.fileName();
    const QString filePath = fi.filePath();
    //const QByteArray boundary = "---------------------------10138964235405";


    QNetworkRequest req;
    QUrl url;
    QString contentType = fi.suffix() == "jpg" ? "jpeg" : fi.suffix();
    QString contentTypeHeader = QString("image/%1").arg(contentType);

    url.setScheme(serverScheme);
    url.setHost(serverName);
    url.setPath(serverUploadPath);

    req.setUrl(url);
    //req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("multipart/form-data; boundary=" + boundary));

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QFile *imgFile = new QFile(filePath);
    imgFile->open(QIODevice::ReadOnly);

    QHttpPart thefile0;
    thefile0.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"thefile0\"; filename=\"%1\"").arg(fileName));
    thefile0.setHeader(QNetworkRequest::ContentTypeHeader,  contentTypeHeader);
    thefile0.setBodyDevice(imgFile);
    imgFile->setParent(multiPart); // we cannot delete the imgFile now, so delete it with the multiPart

    if(privateUpload)
    {
        QHttpPart privatePart;
        privatePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"prv\""));
        privatePart.setBody("1");
    }

    QHttpPart submitX;
    submitX.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"submit.x\""));
    submitX.setBody("48");

    QHttpPart submitY;
    submitY.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"submit.y\""));
    submitY.setBody("28");

    multiPart->append(thefile0);
    multiPart->append(submitX);
    multiPart->append(submitY);

    //multiPart->setBoundary(boundary);

    nm->setTransferTimeout(60000);
    uploadReply = nm->post(req, multiPart);
    uploadReply->ignoreSslErrors();
    multiPart->setParent(uploadReply);    // delete the multiPart with the reply
    connect(uploadReply,   &QNetworkReply::finished, this, &ImgUpHostPic::imageUploaded);
}

/*****************************************************************************/
void ImgUpHostPic::imageUploaded()
{
    QNetworkReply *reply = uploadReply;

    if(reply->error() == QNetworkReply::NoError)
    {
        // ReplyData: "<script language='javascript'>parent.upload('','|1551438','','|2404221946590096.jpg','|/home/hostpic.org/public_html/images/2404221946590096.jpg','|https://www.hostpic.org/images/2404221946590096.jpg','|2404221946590096.jpg','|https://www.hostpic.org/images/2404221946590096_tn.jpg','https://www.hostpic.org','https://www.hostpic.org','Host Pic.Org - Free Image Picture Photo Hosting','1');</script>"
        QByteArray ReplyData = reply->readAll();
        QString filename = imageNameFromReplyData(ReplyData);
        showUploadPage(filename);
    }
    else
        showErr(reply->errorString());

    reply->deleteLater();
}

/*****************************************************************************/
void ImgUpHostPic::gotPageForCookie()
{
    QNetworkReply *reply = pagereply;

    if(reply->error() == QNetworkReply::NoError)
        sendFile();
    else
        showErr(reply->errorString());

    reply->deleteLater();
}

/*****************************************************************************/
void ImgUpHostPic::showUploadPage(QString filename)
{
    QUrl viewUrl;
    viewUrl.setScheme(serverScheme);
    viewUrl.setHost(serverName);
    viewUrl.setPath(serverViewPath);
    viewUrl.setQuery(serverViewQuery.arg(filename));

    qDebug() << "Uploaded image URL: " << viewUrl.toString();

#ifdef QT_WEBENGINEWIDGETS_LIB
    webEngine->load(viewUrl.toDisplayString());
#else
    QUrl directLinkUrl;
    directLinkUrl.setScheme(serverScheme);
    directLinkUrl.setHost(serverName);
    directLinkUrl.setPath(serverDirectPath.arg(filename), QUrl::DecodedMode);

    auto htmltext = QString(R"(
<div align=center>
<img src=":/icons/hostpic.jpg">
<br>
<h2>Image has been uploaded</h2>
<br>

<div align="center">
    <strong >Link for viewing the photo</strong>
    <br>
    <a href="%1">%1</a>
</div>
<br>
<div align="center">
    <strong >Link directly to your photo</strong>
    <br>
    <a href="%2">%2</a>
</div>

</div>
)"
).arg(viewUrl.toString(), directLinkUrl.toString());

    webBrowser->setHtml(htmltext);
#endif
    show();
}
/*****************************************************************************/
