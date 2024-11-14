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
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>

#include "imgpi.h"
/*****************************************************************************/
ImgPI::ImgPI(QWidget *parent, QString filePath):
    ImgUp(parent, filePath)
{
    serverName="postimages.org";
    serverScheme="https";
    serverPath = "/json/rr";

    removeAfterDays = "31"; // 0 1 7 31
}
/*****************************************************************************/
void ImgPI::postUploadRequest()
{
    QFileInfo fi(imageFilePath);
    const QString fileName = fi.fileName();
    const QString filePath = fi.filePath();
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    //const QByteArray boundary = multiPart->boundary();

    QNetworkRequest req;
    QUrl url;

    QString imgType = "jpeg", imgExt = fi.suffix().toLower();

    if(imgExt == "png" || imgExt == "webp" || imgExt == "avif")
        imgType = imgExt;

    QString contentType = QString("image/%1").arg(imgType);

    url.setScheme(serverScheme);
    url.setHost(serverName);
    url.setPath(serverPath);

    req.setUrl(url);
    qDebug() << "Uploading to" << req.url().toString();

    //req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("multipart/form-data; boundary=" + boundary));

    QHttpPart galleryPart;
    galleryPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"gallery\""));
    galleryPart.setBody("");

    QHttpPart optsizePart;
    optsizePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"optsize\""));
    optsizePart.setBody("0");

    QHttpPart expirePart;
    expirePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"expire\""));
    expirePart.setBody(removeAfterDays);

    QHttpPart numfilesPart;
    numfilesPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"numfiles\""));
    numfilesPart.setBody("1");

    QHttpPart sessionPart;
    // 1731426748791.2473295402939576 ~ date "+%s" . ???
    QByteArray epoch = QString("%1%2").arg(QDateTime::currentDateTime().toSecsSinceEpoch()).arg(QString(".2473295402939576")).toLatin1();
    sessionPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"upload_session\""));
    sessionPart.setBody(epoch);

    QHttpPart refererPart;
    // aHR0cHM6Ly9wb3N0aW1nLmNjLw==
    refererPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"upload_referer\""));
    refererPart.setBody("aHR0cDovL3Bvc3RpbWFnZXMub3JnCg=="); // "http://postimages.org" in base64


    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader,  contentType);
    QFile *imgFile = new QFile(filePath);
    imgFile->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(imgFile);
    imgFile->setParent(multiPart); // we cannot delete the imgFile now, so delete it with the multiPart

    multiPart->append(galleryPart);
    multiPart->append(optsizePart);
    multiPart->append(expirePart);
    multiPart->append(numfilesPart);
    multiPart->append(sessionPart);
    multiPart->append(refererPart);
    multiPart->append(filePart);

    //multiPart->setBoundary(boundary);

    uploadReply = nm->post(req, multiPart);
    multiPart->setParent(uploadReply);    // delete the multiPart with the reply

    connect(uploadReply,   &QNetworkReply::finished, this, &ImgPI::imageUploaded);
}
/*****************************************************************************/
void ImgPI::imageUploaded()
{
/*
    {
        "url":"https:\/\/postimg.cc\/rDCg9rmF\/4f57cd78",
        "status":"OK",
        "status_code":200,
        "success":{
            "message":"image uploaded",
            "code":200
        },
        "request":{
            "gallery":"",
            "optsize":"0",
            "expire":"0",
            "numfiles":"1",
            "upload_session":"1731426748791.2473295402939576",
            "upload_referer":"aHR0cHM6Ly9wb3N0aW1nLmNjLw=="
        },
        "status_txt":"OK"
    }
*/
    QNetworkReply *reply = uploadReply;

    QByteArray data = reply->readAll();
    qDebug() << "Reply:" << data;

    if(reply->error() == QNetworkReply::NoError)
    {
        auto jsonDoc = QJsonDocument::fromJson(data);

        if(!jsonDoc.isNull())
        {
            if(jsonDoc.isObject())
            {
                auto o = jsonDoc.object();

                if(!o.isEmpty())
                {
                    if(o["status_code"].toInt() == 200)
                    {
                        qDebug() << "Status text:" << o["status_txt"];
                        showUploadPage(o["url"].toString());
                    }
                    else
                        showErr(QString("%1\n%2: %3").arg(tr("Upload failed!")).arg(tr("Reason given")).arg(o["status_txt"].toString()));
                }
                else
                    showErr(tr("Empty JSON object!"));
            }
            else
                showErr(tr("Returned page does not contain JSON object!"));
        }
        else
            showErr(tr("Empty result!"));
    }
    else
        showErr(reply->errorString());

    reply->deleteLater();
}
/*****************************************************************************/
void ImgPI::showUploadPage(QString url)
{
    qDebug() << "Loading result page" << url << "...";
    //QUrl resulPage;
    //resulPage.setUrl(url);

#ifdef QT_WEBENGINEWIDGETS_LIB
    webEngine->load(QUrl(url));
#else
    /*
     *  https://postimg.cc/zHFQybdR/7496e92a
     *
        Link:                               https://postimg.cc/zHFQybdR
        Direct link:                        https://i.postimg.cc/kG0PhWKw/sample-s.jpg
        Markdown (reddit):                  [sample-s.jpg](https://postimg.cc/zHFQybdR)
        Markdown (github stackoverflow):    [![sample-s.jpg](https://i.postimg.cc/kG0PhWKw/sample-s.jpg)](https://postimg.cc/zHFQybdR)
        Thumbnail for forums:               [url=https://postimg.cc/zHFQybdR][img]https://i.postimg.cc/zHFQybdR/sample-s.jpg[/img][/url]
        Thumbnail for website:              <a href='https://postimg.cc/zHFQybdR' target='_blank'><img src='https://i.postimg.cc/zHFQybdR/sample-s.jpg' border='0' alt='sample-s'/></a>
        Hotlink for forums:                 [url=https://postimages.org/][img]https://i.postimg.cc/kG0PhWKw/sample-s.jpg[/img][/url]
        Hotlink for website:                <a href='https://postimages.org/' target='_blank'><img src='https://i.postimg.cc/kG0PhWKw/sample-s.jpg' border='0' alt='sample-s'/></a>
        Removal link:                       https://postimg.cc/delete/KSX2yX0h2/7496e92a
    */

    auto htmltext = QString("<div align=center><img src=\":/icons/postimgs.png\"><br><h2>Image has been uploaded.</h2><br><a href=\"%1\">%1</a></div>").arg(url);
    webBrowser->setHtml(htmltext);
#endif
    show();
}
/*****************************************************************************/
