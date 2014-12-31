#include "webicondatabase.h"

const QString WebIconDatabase::WEBKIT_DATABASE = ".QtWebKit";
const QString WebIconDatabase::ICON_DATABASE = "WebpageIcons.db";
const QString WebIconDatabase::PROVIDER_NAME = "favicons";

QSqlDatabase WebIconDatabase::_db;
int WebIconDatabase::_refcount = 0;

WebIconDatabase::WebIconDatabase(QObject *parent): QObject(parent)
{
    if(!this->_refcount)
        this->_db = QSqlDatabase::addDatabase("QSQLITE");

    this->_refcount++;
}

WebIconDatabase::~WebIconDatabase()
{
    this->_refcount--;

    if(!this->_refcount)
    {
        if(this->_db.isOpen())
            this->_db.close();
    }
}

bool WebIconDatabase::hasIcon(const QString &url)
{
    QString s = this->queryIconUrl(url);

    if(s.isEmpty())
        return false;

    QByteArray b = this->queryIconPixmap(url);
    return !b.isEmpty();
}

QString WebIconDatabase::provideIcon(const QString &url)
{
    if(this->hasIcon(url))
        return QString("image://%1/%2").arg(WebIconDatabase::PROVIDER_NAME, url);

    return "image://theme/icon-m-favorite-selected";
}

bool WebIconDatabase::open()
{
    if(this->_db.isOpen())
        return true;

    QDir dbpath = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    if(!dbpath.cd(WebIconDatabase::WEBKIT_DATABASE))
        return false;

    this->_db.setDatabaseName(dbpath.absoluteFilePath(WebIconDatabase::ICON_DATABASE));
    return this->_db.open();
}

int WebIconDatabase::queryIconId(const QString &url)
{
    if(!this->open())
        return -1;

    QSqlQuery q(this->_db);
    QString queryurl = QUrl(url).host().prepend("%").append("%");

    if(!this->prepare(q, "SELECT iconID FROM PageURL WHERE url LIKE ?"))
        return -1;

    q.bindValue(0, queryurl);

    if(!this->execute(q) || !q.first())
        return -1;

    return q.value(0).toInt();
}

QString WebIconDatabase::queryIconUrl(const QString &url)
{
    int iconid = this->queryIconId(url);

    if(iconid == -1)
        return QString();

    QSqlQuery q(this->_db);

    if(!this->prepare(q, "SELECT url FROM IconInfo WHERE iconID = ?"))
        return QString();

    q.bindValue(0, iconid);

    if(!this->execute(q) || !q.first())
        return QString();

    return q.value(0).toString();
}

QByteArray WebIconDatabase::queryIconPixmap(const QString &url)
{
    int iconid = this->queryIconId(url);

    if(iconid == -1)
        return QByteArray();

    QSqlQuery q(this->_db);

    if(!this->prepare(q, "SELECT data FROM IconData WHERE iconID = ?"))
        return QByteArray();

    q.bindValue(0, iconid);

    if(!this->execute(q) || !q.first())
        return QByteArray();

    return q.value(0).toByteArray();
}

bool WebIconDatabase::prepare(QSqlQuery& queryobj, const QString &query)
{
    if(!queryobj.prepare(query))
    {
        qWarning() << Q_FUNC_INFO << "failed to prepare query";
        qWarning() << queryobj.lastError();
        qWarning() << queryobj.lastQuery();

        return false;
    }

    return true;
}

bool WebIconDatabase::execute(QSqlQuery &queryobj)
{
    if(!queryobj.exec())
    {
        qWarning() << Q_FUNC_INFO << "failed to execute query";
        qWarning() << queryobj.lastError();
        qWarning() << queryobj.lastQuery();

        return false;
    }

    return true;
}