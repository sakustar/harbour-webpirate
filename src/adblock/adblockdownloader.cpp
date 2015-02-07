#include "adblockdownloader.h"

const int AdBlockDownloader::FILES_MAX = 2;
const QString AdBlockDownloader::GITHUB_RAW_URL = "https://raw.githubusercontent.com/Dax89/harbour-webpirate/master";

AdBlockDownloader::AdBlockDownloader(QObject *parent): QObject(parent), _downloading(false), _filesdownloaded(0), _progressvalue(0), _progresstotal(0), _adblockmanager(NULL), _downloadreply(NULL)
{
    connect(&this->_networkmanager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onDownloadFinished(QNetworkReply*)));
}

bool AdBlockDownloader::downloading() const
{
    return this->_downloading;
}

qint64 AdBlockDownloader::progressValue() const
{
    return this->_progressvalue;
}

qint64 AdBlockDownloader::progressTotal() const
{
    return this->_progresstotal;
}

void AdBlockDownloader::downloadFile(const QString &filename, const QString& localfile)
{
    this->_progressvalue = 0;
    this->_progresstotal = 0;

    emit connectionStarted();

    this->_localfile = localfile;
    this->_tempfile.setFileName(localfile);
    this->_tempfile.open(QFile::WriteOnly | QFile::Truncate);

    QUrl url(QString("%1/%2").arg(AdBlockDownloader::GITHUB_RAW_URL, filename));
    QNetworkRequest request(url);

    this->_downloadreply = this->_networkmanager.get(request);
    connect(this->_downloadreply, SIGNAL(readyRead()), this, SLOT(onNetworkReplyReadyRead()));
    connect(this->_downloadreply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
    connect(this->_downloadreply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onDownloadError(QNetworkReply::NetworkError)));
}

void AdBlockDownloader::downloadFilters(AdBlockManager* adblockmanager)
{
    this->_adblockmanager = adblockmanager;
    this->_filesdownloaded = 0;
    this->_errormessage = QString();
    this->_localfile = QString();

    adblockmanager->rulesFileInstance().close();
    this->downloadFile(AdBlockManager::CSS_FILENAME, adblockmanager->cssFile());
}

void AdBlockDownloader::onNetworkReplyReadyRead()
{
    this->_tempfile.write(this->_downloadreply->readAll());
}

void AdBlockDownloader::onDownloadProgress(qint64 bytesreceived, qint64 bytestotal)
{
    if(!this->_downloading)
    {
        this->_downloading = true;
        emit downloadingChanged();
    }

    this->_progressvalue = bytesreceived;

    if(this->_progresstotal != bytestotal)
    {
        this->_progresstotal = bytestotal;
        emit progressTotalChanged();
    }

    emit progressValueChanged();
}

void AdBlockDownloader::onDownloadError(QNetworkReply::NetworkError)
{
    this->_tempfile.close();
    this->_tempfile.remove();

    emit downloadError(this->_downloadreply->errorString());
}

void AdBlockDownloader::onDownloadFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    this->_downloadreply = NULL;
    this->_filesdownloaded++;

    this->_tempfile.close();

    if(this->_filesdownloaded == 1) /* Download the second file */
        this->downloadFile(AdBlockManager::TABLE_FILENAME, this->_adblockmanager->tableFile());
    else if(this->_filesdownloaded == AdBlockDownloader::FILES_MAX)
    {
        this->_downloading = false;

        emit downloadingChanged();
        emit downloadCompleted();
        emit this->_adblockmanager->rulesChanged();
    }
}
