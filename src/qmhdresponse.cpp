#include "qmhdresponse.h"

#include <QJsonDocument>

#include <microhttpd.h>

class QMHDResponsePrivate
{
    public:
        QMHDResponsePrivate();

    public:
        MHD_Connection* mhdConnection;
        QMHDHttpStatus status;
        QStringHash headers;
        QByteArray body;
        int bodyFileDescriptor;
        size_t bodyFileSize;
        bool sent;
};

QMHDResponsePrivate::QMHDResponsePrivate()
    : mhdConnection(NULL),
      status(QMHDHttpStatus::Ok),
      bodyFileDescriptor(-1),
      bodyFileSize(0),
      sent(false)
{
}

QMHDResponse::QMHDResponse(void* mhdConnection)
    : QObject(),
      d(new QMHDResponsePrivate())
{
    d->mhdConnection = static_cast<MHD_Connection*>(mhdConnection);
}

QMHDResponse::~QMHDResponse()
{
    delete d;
}

void QMHDResponse::send(const QByteArray& buffer)
{
    if (d->sent == false) {
        d->body.append(buffer);
        send();
    }
}

void QMHDResponse::send(const QJsonDocument& json)
{
    send(json.toJson(QJsonDocument::Compact));
}

void QMHDResponse::send(int fileDescriptor, size_t fileSize)
{
    if (d->sent == false) {
        d->bodyFileDescriptor = fileDescriptor;
        d->bodyFileSize = fileSize;
        send();
    }
}

void QMHDResponse::send()
{
    MHD_Response* response;

    if (d->sent == false) {
        if (d->bodyFileDescriptor >= 0) {
            response = MHD_create_response_from_fd(d->bodyFileSize, d->bodyFileDescriptor);
        } else {
            response = MHD_create_response_from_buffer(d->body.size(), (void*) d->body.constData(), MHD_RESPMEM_PERSISTENT);
        }

        for (auto it = d->headers.begin(); it != d->headers.end(); ++it) {
            MHD_add_response_header(response, qPrintable(it.key()), qPrintable(it.value()));
        }

        MHD_queue_response(d->mhdConnection, (quint32) d->status, response);
        MHD_resume_connection(d->mhdConnection);
        MHD_destroy_response(response);

        d->sent = true;
    }
}

QMHDHttpStatus QMHDResponse::status() const
{
    return d->status;
}

void QMHDResponse::setStatus(QMHDHttpStatus status)
{
    if (d->sent == false) {
        d->status = status;
    }
}

QString QMHDResponse::header(const QString& name) const
{
    return d->headers.value(name.toLower());
}

void QMHDResponse::setHeader(const QString& name, const QString& value)
{
    d->headers.insert(name.toLower(), value);
}

QStringHash QMHDResponse::headers() const
{
    return d->headers;
}

void QMHDResponse::setHeaders(const QStringHash& headers)
{
    d->headers.clear();
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        setHeader(it.value(), it.key());
    }
}