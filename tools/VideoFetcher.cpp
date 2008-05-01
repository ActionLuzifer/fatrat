#include "VideoFetcher.h"
#include <QMessageBox>
#include <QHttp>
#include <QUrl>
#include <QRegExp>
#include <QBuffer>
#include <QFile>
#include <QtDebug>
#include "MainWindow.h"
#include "fatrat.h"

const char* YOUTUBE_URL_REGEXP = "http://youtube\\.com/watch\\?v=([0-9A-Za-z_-]+).*";
const char* YOUTUBE_URL_T = ", \"t\": \"([^\"]+)\"";
const char* YOUTUBE_TITLE = "<title>YouTube - ([^<]+)</title>";

static const VideoFetcher::Fetcher m_fetchers[] = { { "YouTube", YOUTUBE_URL_REGEXP, VideoFetcher::decodeYouTube } };

VideoFetcher::VideoFetcher() : m_nextType(-1)
{
	setupUi(this);
	
	connect(labelInfo, SIGNAL(linkActivated(const QString&)), this, SLOT(openLink(const QString&)));
	connect(pushDownload, SIGNAL(clicked()), this, SLOT(download()));
	connect(pushDecode, SIGNAL(clicked()), this, SLOT(decode()));
}

void VideoFetcher::openLink(const QString& link)
{
	if(link != "showServices")
		return;
	
	QString text = tr("The following services are supported:");
	
	text += "<ul>";
	for(size_t i=0;i<sizeof(m_fetchers)/sizeof(m_fetchers[0]);i++)
		text += QString("<li>%1</li>").arg(m_fetchers[i].name);
	text += "</ul>";
	
	QMessageBox::information(this, "FatRat", text);
}

void VideoFetcher::decode()
{
	QString text = textLinks->toPlainText();
	m_videos = text.split('\n');
	
	m_unsupp.clear();
	pushDecode->setDisabled(true);
	
	doNext();
}

void VideoFetcher::finalize()
{
	pushDecode->setEnabled(true);
	
	if(!m_unsupp.isEmpty() || !m_err.isEmpty())
	{
		QString err;
			
		if(!m_unsupp.isEmpty())
		{
			err = tr("These links aren't supported:");
			err += "\n\n";
			foreach(QString l, m_unsupp)
			{
				err += l;
				err += '\n';
			}
		}
			
		if(!m_err.isEmpty())
		{
			if(!err.isEmpty())
				err += "\n\n";
				
			err = tr("Couldn't decode these links:");
			err += "\n\n";
			foreach(QString l, m_err)
			{
				err += l;
				err += '\n';
			}
		}
			
		QMessageBox::critical(this, "FatRat", err);
	}
	else
		textLinks->setText(QString());
}

void VideoFetcher::doNext()
{
	while(true)
	{
		if(m_videos.isEmpty())
		{
			finalize();
			break;
		}
		
		m_now = m_videos.takeFirst();
		QUrl url = m_now;
		
		m_nextType = -1;
		for(size_t i=0;i<sizeof(m_fetchers)/sizeof(m_fetchers[0]);i++)
		{
			QRegExp re(m_fetchers[i].re);
			if(re.exactMatch(m_now))
			{
				m_nextType = i;
				break;
			}
		}
		
		if(m_nextType < 0)
		{
			m_unsupp << m_now;
			continue;
		}
		else
			qDebug() << "Detected as" << m_fetchers[m_nextType].name;
		
		QHttp* http = new QHttp(url.host());
		m_buffer = new QBuffer(http);
		
		m_buffer->open(QIODevice::ReadWrite);
		
		connect(http, SIGNAL(done(bool)), this, SLOT(downloadDone(bool)));
		http->get(url.path() + "?" + url.encodedQuery(), m_buffer);
		break;
	}
}

void VideoFetcher::download()
{
	QString text = textDecoded->toPlainText();
	if(!text.isEmpty())
	{
		MainWindow* wnd = (MainWindow*) getMainWindow();
		wnd->addTransfer(text);
	}
}

void VideoFetcher::downloadDone(bool error)
{
	QHttp* http = static_cast<QHttp*>(sender());
	
	http->deleteLater();
	
	if(error)
		m_err << m_now;
	else
	{
		QString decoded = m_fetchers[m_nextType].lpfn(m_buffer->data(), m_now);
		if(!decoded.isEmpty())
			textDecoded->append(decoded);
		else
			m_err << m_now;
	}
	
	doNext();
}

QString VideoFetcher::decodeYouTube(QByteArray data, QString url)
{
	QRegExp re(YOUTUBE_URL_T);
	if(re.indexIn(data) < 0)
	{
		QFile dump("dump.txt");
		dump.open(QIODevice::WriteOnly);
		dump.write(data);
		return QString();
	}
	
	QString id, title, t;
	int p = url.indexOf("v=");
	int e = url.indexOf("&", p);
	
	t = re.cap(1);
	
	if(p < 0)
		return QString();
	p += 2;
	
	id = url.mid(p, (e != -1) ? e-p : -1);
	
	re.setPattern(YOUTUBE_TITLE);
	if(re.indexIn(data))
	{
		title = QUrl::toPercentEncoding(re.cap(1));
		title += ".flv";
	}
	
	return QString("http://www.youtube.com/get_video?video_id=%1&t=%2#__filename=%3").arg(id).arg(t).arg(title);
}

