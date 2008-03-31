#include "TorrentSettings.h"
#include "TorrentDownload.h"
#include <QDir>
#include <QMessageBox>

extern const char* TORRENT_FILE_STORAGE;

TorrentSettings::TorrentSettings(QWidget* w)
{
	setupUi(w);
	
	QStringList encs;
	
	encs << tr("Disabled") << tr("Enabled") << tr("Permit only encrypted");
	
	comboAllocation->addItems( QStringList() << tr("Full") << tr("Sparse") << tr("Compact") );
	comboEncIncoming->addItems(encs);
	comboEncOutgoing->addItems(encs);
	comboEncLevel->addItems( QStringList() << tr("Plaintext") << tr("RC4") << tr("Both", "Both levels") );
	
	connect(pushCleanup, SIGNAL(clicked()), this, SLOT(cleanup()));
}

void TorrentSettings::load()
{
	spinListenStart->setValue(g_settings->value("torrent/listen_start", getSettingsDefault("torrent/listen_start")).toInt());
	spinListenEnd->setValue(g_settings->value("torrent/listen_end", getSettingsDefault("torrent/listen_end")).toInt());
	spinRatio->setValue(g_settings->value("torrent/maxratio", getSettingsDefault("torrent/maxratio")).toDouble());
	
	spinConnections->setValue(g_settings->value("torrent/maxconnections", getSettingsDefault("torrent/maxconnections")).toInt());
	spinUploads->setValue(g_settings->value("torrent/maxuploads", getSettingsDefault("torrent/maxuploads")).toInt());
	spinConnectionsLocal->setValue(g_settings->value("torrent/maxconnections_loc", getSettingsDefault("torrent/maxconnections_loc")).toInt());
	spinUploadsLocal->setValue(g_settings->value("torrent/maxuploads_loc", getSettingsDefault("torrent/maxuploads_loc")).toInt());
	
	spinFiles->setValue(g_settings->value("torrent/maxfiles", getSettingsDefault("torrent/maxfiles")).toInt());
	checkDHT->setChecked(g_settings->value("torrent/dht", getSettingsDefault("torrent/dht")).toBool());
	checkPEX->setChecked(g_settings->value("torrent/pex", getSettingsDefault("torrent/pex")).toBool());
	comboAllocation->setCurrentIndex(g_settings->value("torrent/allocation", getSettingsDefault("torrent/allocation")).toInt());
	
	comboEncIncoming->setCurrentIndex(g_settings->value("torrent/enc_incoming", getSettingsDefault("torrent/enc_incoming")).toInt());
	comboEncOutgoing->setCurrentIndex(g_settings->value("torrent/enc_outgoing", getSettingsDefault("torrent/enc_outgoing")).toInt());
	
	comboEncLevel->setCurrentIndex(g_settings->value("torrent/enc_level", getSettingsDefault("torrent/enc_level")).toInt());
	checkEncRC4Prefer->setChecked(g_settings->value("torrent/enc_rc4_prefer", getSettingsDefault("torrent/enc_rc4_prefer")).toBool());
	
	lineIP->setText(g_settings->value("torrent/external_ip").toString());
	
	comboProxyTracker->clear();
	comboProxyPeer->clear();
	comboProxySeed->clear();
	
	comboProxyTracker->addItem(tr("None", "No proxy"));
	comboProxyPeer->addItem(tr("None", "No proxy"));
	comboProxySeed->addItem(tr("None", "No proxy"));
	
	m_listProxy = Proxy::loadProxys();
	
	QUuid tracker, seed, peer;
	
	tracker = g_settings->value("torrent/proxy_tracker").toString();
	seed = g_settings->value("torrent/proxy_webseed").toString();
	peer = g_settings->value("torrent/proxy_peer").toString();
	
	for(int i=0;i<m_listProxy.size();i++)
	{
		int index;
		QString name = m_listProxy[i].toString();
		
		comboProxyTracker->addItem(name);
		index = comboProxyTracker->count()-1;
		comboProxyTracker->setItemData(index, m_listProxy[i].uuid.toString());
		
		if(m_listProxy[i].uuid == tracker)
			comboProxyTracker->setCurrentIndex(index);
		
		comboProxySeed->addItem(name);
		index = comboProxySeed->count()-1;
		comboProxySeed->setItemData(index, m_listProxy[i].uuid.toString());
		
		if(m_listProxy[i].uuid == seed)
			comboProxySeed->setCurrentIndex(index);
		
		if(m_listProxy[i].nType != Proxy::ProxyHttp)
		{
			comboProxyPeer->addItem(name);
			index = comboProxyPeer->count()-1;
			comboProxyPeer->setItemData(index, m_listProxy[i].uuid.toString());
			
			if(m_listProxy[i].uuid == peer)
				comboProxyPeer->setCurrentIndex(index);
		}
	}
	
	checkUPNP->setChecked(g_settings->value("torrent/mapping_upnp", false).toBool());
	checkNATPMP->setChecked(g_settings->value("torrent/mapping_natpmp", false).toBool());
	checkLSD->setChecked(g_settings->value("torrent/mapping_lsd", false).toBool());
}

void TorrentSettings::accepted()
{
	g_settings->setValue("torrent/listen_start", spinListenStart->value());
	g_settings->setValue("torrent/listen_end", spinListenEnd->value());
	g_settings->setValue("torrent/maxratio", spinRatio->value());
	
	g_settings->setValue("torrent/maxconnections", spinConnections->value());
	g_settings->setValue("torrent/maxuploads", spinUploads->value());
	g_settings->setValue("torrent/maxconnections_loc", spinConnectionsLocal->value());
	g_settings->setValue("torrent/maxuploads_loc", spinUploadsLocal->value());
	
	g_settings->setValue("torrent/maxfiles", spinFiles->value());
	g_settings->setValue("torrent/dht", checkDHT->isChecked());
	g_settings->setValue("torrent/pex", checkPEX->isChecked());
	g_settings->setValue("torrent/allocation", comboAllocation->currentIndex());
	
	g_settings->setValue("torrent/external_ip", lineIP->text());
	
	int index = comboProxyTracker->currentIndex();
	g_settings->setValue("torrent/proxy_tracker", comboProxyTracker->itemData(index).toString());
	index = comboProxySeed->currentIndex();
	g_settings->setValue("torrent/proxy_webseed", comboProxySeed->itemData(index).toString());
	index = comboProxyPeer->currentIndex();
	g_settings->setValue("torrent/proxy_peer", comboProxyPeer->itemData(index).toString());
	
	g_settings->setValue("torrent/enc_incoming", comboEncIncoming->currentIndex());
	g_settings->setValue("torrent/enc_outgoing", comboEncOutgoing->currentIndex());
	
	g_settings->setValue("torrent/enc_level", comboEncLevel->currentIndex());
	g_settings->setValue("torrent/enc_rc4_prefer", checkEncRC4Prefer->isChecked());
	
	g_settings->setValue("torrent/mapping_upnp", checkUPNP->isChecked());
	g_settings->setValue("torrent/mapping_natpmp", checkNATPMP->isChecked());
	g_settings->setValue("torrent/mapping_lsd", checkLSD->isChecked());
	
	TorrentDownload::applySettings();
}

void TorrentSettings::cleanup()
{
	int removed = 0;
	
	QDir dir = QDir::home();
	QStringList files, hashes, toremove;
	std::vector<libtorrent::torrent_handle> torrents;
	
	torrents = TorrentDownload::m_session->get_torrents();
	
	for(size_t i=0;i<torrents.size();i++)
	{
		const libtorrent::big_number& bn = torrents[i].info_hash();
		hashes << QByteArray((char*) bn.begin(), 20).toHex();
	}
	
	if(dir.cd(TORRENT_FILE_STORAGE))
		files = dir.entryList(QStringList("*.torrent"));
	
	foreach(QString file, files)
	{
		int pt, pos = file.lastIndexOf(" - ");
		if(pos < 0)
			continue;
		pt = file.lastIndexOf('.');
		if(pt < 0)
			continue; // shouldn't happen
		
		if(!hashes.contains(file.mid(pos+3, pt-pos-3)))
		{
			dir.remove(file);
			removed++;
		}
	}
	
	QMessageBox::information(spinListenStart, "FatRat", tr("Removed %1 files.").arg(removed));
}

