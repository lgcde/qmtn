#ifndef INTERFACES_H
#define INTERFACES_H

#include <QtPlugin>

class ImageProcessInteface
{
public:
    virtual ~ImageProcessInteface() {}

    /// \brief Name of the plugin. May be used to add ation to the menubar
    virtual QString Name() = 0;

    ///
    /// \brief Start procesing file (e.g. upload to server)
    /// \param ImageFileName - path to the generated image
    ///
    virtual void process(QString ImageFileName) = 0;

    ///
    /// \brief Result of processing file
    /// \return false until processing finished successfuly
    ///
    virtual bool getResult() = 0;

    ///
    /// \brief Web URL for result page about finished processing
    /// \return QUrl of web page
    ///
    virtual QUrl getResultUrl() = 0;

    ///
    /// \brief Build richtext formated result page about finished processing
    /// \return html text for QTextEdit
    ///
    virtual QString getResultPage() = 0;

    ///
    /// \brief Holds text info about last error
    /// \return error text
    ///
    virtual QString getErrText() = 0;

    // signals:
    ///
    /// \brief This signal is emitted whenever processing file is finished
    /// \param success - true if processing file was successful
    ///
    // void finished(bool success);
};

Q_DECLARE_INTERFACE(ImageProcessInteface, "org.rustypipe.qmtn/1.0")

#endif // INTERFACES_H
