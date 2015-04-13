/*
 * Copyright 2010  Open Source & Linux Lab (OSLL) osll@osll.spb.ru
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * The advertising clause requiring mention in adverts must never be included.
 */

/*! ---------------------------------------------------------------
 * $Id$
 *
 * \file LoadTagsJSON.cpp
 * \brief LoadTagsJSON implementation
 *
 * File description
 *
 * PROJ: OSLL/geo2tag
 * ---------------------------------------------------------------- */

#include <QVariant>
#include "servicelogger.h"
#include "LoadTagsResponseJSON.h"

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "defines.h"

#include "User.h"
#include "Channel.h"

#include "JsonUser.h"
#include "JsonChannel.h"
#include "JsonDataMark.h"

#include "ErrnoTypes.h"

LoadTagsResponseJSON::LoadTagsResponseJSON(const QList<Tag>&hashMap, QObject *parent):
    JsonSerializer(parent), m_tags(hashMap)
{
}


LoadTagsResponseJSON::LoadTagsResponseJSON(QObject *parent) : JsonSerializer(parent)
{
}


bool LoadTagsResponseJSON::parseJson(const QByteArray &data)
{
    clearContainers();
    qDebug () << "LoadTagsResponseJSON::parseJson(const QByteArray &data)";

    QJson::Parser parser;
    bool ok;
    QVariantMap result = parser.parse(data, &ok).toMap();

    if (!ok) return false;

    result["errno"].toInt(&ok);
    if (!ok) return false;
    m_errno = result["errno"].toInt(&ok);

    QVariantMap rss = result["rss"].toMap();
    QVariantMap channelVariant = rss["channels"].toMap();
    QVariantList channelsList = channelVariant["items"].toList();
    int size = channelsList.size();

    for (int i = 0; i < size; i++)
    {
        QVariantMap channelDesc = channelsList.at(i).toMap();
        QVariantList markList = channelDesc["items"].toList();
        QString channelName = channelDesc["name"].toString();

        Channel channel(channelName,"dummy channel[LoadTagsResponse]");

        for(int j=0; j<markList.size(); j++)
        {
            QVariantMap markMap = markList.at(j).toMap();

            QString title = markMap["title"].toString();
            QString link = markMap["link"].toString();
            QString description = markMap["description"].toString();
            double altitude = markMap["altitude"].toString().toDouble(&ok);
            if (!ok) return false;
            double latitude = markMap["latitude"].toString().toDouble(&ok);
            if (!ok) return false;
            double longitude = markMap["longitude"].toString().toDouble(&ok);
            if (!ok) return false;

            QString userName = markMap["user"].toString();
            QString timeStr =  markMap["pubDate"].toString();
            QDateTime time = QDateTime::fromString(timeStr, "dd MM yyyy HH:mm:ss.zzz");

            m_users.push_back(common::BasicUser(userName));

            Tag tag(altitude, latitude, longitude, title,  description, link,  time);
            DEBUG() << "Tag:" << tag;
            qDebug () << "LoadTagsResponseJSON::parseJson(const QByteArray &data)";
            m_tags << tag;
        }
    }
    return true;
}


QByteArray LoadTagsResponseJSON::getJson() const
{
    QJson::Serializer serializer;
    serializer.setDoublePrecision(DOUBLE_PRECISION_RESPONSE);
    QVariantMap obj, rss, jchannel;

    QVariantList jchannels;

    QVariantList jtags;
    QVariantMap channel;

    obj["errno"]= m_errno;

    if (m_errno == SUCCESS){
	    for(int j=0; j<m_tags.size(); j++)
	    {
		Tag tag = m_tags.at(j);
		QVariantMap jtag;
		jtag["title"] = tag.getLabel();
		jtag["link"] = tag.getUrl();
		jtag["description"] = tag.getDescription();
		jtag["latitude"] = tag.getLatitude();
		jtag["altitude"] = tag.getAltitude();
		jtag["longitude"] = tag.getLongitude();
		jtag["user"] = tag.getUser().getLogin();
		jtag["pubDate"] = tag.getTime().toString("dd MM yyyy HH:mm:ss.zzz");
		jtags.append(jtag);
	    }
	    channel["items"] = jtags;
	//    channel["name"] = m_channels.at(i).getName();
	    jchannels.append(channel);

	    jchannel["items"] = jchannels;
	    rss["channels"] = jchannel;
	    obj["rss"] = rss;
    }
    return serializer.serialize(obj);
}


const QList<Tag> &LoadTagsResponseJSON::getData() const
{
    return m_tags;
}


void LoadTagsResponseJSON::setData(const QList<Tag>& d)
{
    DEBUG() << "set data " << d;
    m_tags = d ;
}


LoadTagsResponseJSON::~LoadTagsResponseJSON()
{
}


/* ===[ End of file $HeadURL$ ]=== */
