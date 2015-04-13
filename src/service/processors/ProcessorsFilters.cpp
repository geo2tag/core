#include "DbObjectsCollection.h"
#include "ErrnoTypes.h"
#include "defines.h"
#include "SettingsStorage.h"


#include "Filtration.h"
#include "Filter.h"
#include "TimeFilter.h"
#include "ShapeFilter.h"
#include "AltitudeFilter.h"

#include "FilterDefaultResponseJSON.h"
#include "FilterCircleRequestJSON.h"
#include "FilterCylinderRequestJSON.h"
#include "FilterPolygonRequestJSON.h"
#include "FilterRectangleRequestJSON.h"
#include "FilterBoxRequestJSON.h"
#include "FilterFenceRequestJSON.h"

#include "FilterChannelRequestJSON.h"
#include "FilterChannelResponseJSON.h"

#include "FilterSubstringRequestJSON.h"
#include "FilterSubstringResponseJSON.h"

#include "MetaCache.h"

namespace common
{

QByteArray DbObjectsCollection::processFilterCircleQuery(const QByteArray& data)
{
    DEBUG() <<  ">>> processFilterCircleQuery";

    FilterCircleRequestJSON request;
    return internalProcessFilterQuery(request, data, false);
}

QByteArray DbObjectsCollection::processFilterCylinderQuery(const QByteArray& data)
{
    FilterCylinderRequestJSON request;
    return internalProcessFilterQuery(request, data, true);
}

QByteArray DbObjectsCollection::processFilterPolygonQuery(const QByteArray& data)
{
    FilterPolygonRequestJSON request;
    return internalProcessFilterQuery(request, data, false);
}

QByteArray DbObjectsCollection::processFilterRectangleQuery(const QByteArray& data)
{
    FilterRectangleRequestJSON request;
    return internalProcessFilterQuery(request, data, false);
}

QByteArray DbObjectsCollection::processFilterBoxQuery(const QByteArray& data)
{
    FilterBoxRequestJSON request;
    return internalProcessFilterQuery(request, data, true);
}

QByteArray DbObjectsCollection::processFilterFenceQuery(const QByteArray& data)
{
    FilterFenceRequestJSON request;
    return internalProcessFilterQuery(request, data, true);
}

void DbObjectsCollection::extractLastNTags(QList<Tag>& tags, qulonglong tagNumber){
	if (tagNumber > 0 && tagNumber < (qulonglong)tags.size())
		tags = tags.mid(tags.size()-tagNumber);

}

QByteArray DbObjectsCollection::internalProcessFilterQuery(FilterRequestJSON& request,
                                                           const QByteArray& data, bool is3d)
{
    Filtration filtration;
    FilterDefaultResponseJSON response;
    QByteArray answer(OK_REQUEST_HEADER);

    if (!request.parseJson(data))
    {
        response.setErrno(INCORRECT_JSON_ERROR);
        answer.append(response.getJson());
        return answer;
    }

    Session session = m_defaultCache->findSession(request.getSessionToken());

    if(!session.isValid())
    {
        response.setErrno(WRONG_TOKEN_ERROR);
        answer.append(response.getJson());
        return answer;
    }
    Core::MetaCache * cache = Core::MetaCache::getMetaCache(session);
    qulonglong tagNumber = request.getTagNumber();

    common::BasicUser user = session.getUser();

    filtration.addFilter(QSharedPointer<Filter>(new ShapeFilter(request.getShape())));
    filtration.addFilter(QSharedPointer<Filter>(new TimeFilter(request.getTimeFrom(), request.getTimeTo())));
    if(is3d)
    {
        filtration.addFilter(QSharedPointer<Filter>(new AltitudeFilter(request.getAltitude1(), request.getAltitude2())));
    }

    QList<Channel> channels = cache->getSubscribedChannels(user);
    QList<Tag> feed;

    if (request.getChannels().size() > 0)
    {
        Channel targetChannel;
        // look for ...
        for(int i = 0; i<channels.size(); i++)
        {
            if (channels.at(i).getName() == request.getChannels().at(0).getName())
            {
                targetChannel = channels.at(i);
                break;
            }
        }

        if (!targetChannel.isValid())
        {
            response.setErrno(CHANNEL_DOES_NOT_EXIST_ERROR);
            answer.append(response.getJson());
            DEBUG() << "answer: " << answer.data();
            return answer;
        }

        QList<Tag > tags = cache->loadTagsFromChannel(targetChannel);
        QList<Tag > filteredTags = filtration.filtrate(tags);

	extractLastNTags( filteredTags, tagNumber);
        feed << filteredTags;

        response.setErrno(SUCCESS);
    }
    else
    {
        for(int i = 0; i<channels.size(); i++)
        {
            Channel channel = channels.at(i);
            QList<Tag > tags = cache->loadTagsFromChannel(channel);
            QList<Tag > filteredTags = filtration.filtrate(tags);

	    extractLastNTags( filteredTags, tagNumber);
            feed << filteredTags;
        }
	DEBUG() << "Filtred tags number " << feed.size();

    m_defaultCache->updateSession(session);

        response.setErrno(SUCCESS);
    }
    response.setTags(feed);

    answer.append(response.getJson());
    DEBUG() << "answer: " << answer.data();
    return answer;
}

QByteArray DbObjectsCollection::processFilterSubstringQuery(const QByteArray &data)
{
    FilterSubstringRequestJSON request;
    FilterSubstringResponseJSON response;

    QByteArray answer(OK_REQUEST_HEADER);

    if (!request.parseJson(data))
    {
        response.setErrno(INCORRECT_JSON_ERROR);
        answer.append(response.getJson());
        return answer;
    }
    
    Session session = m_defaultCache->findSession(request.getSessionToken());

    DEBUG() << "Checking for sessions with token = " << session.getSessionToken();
    DEBUG() << "Session:" << session;

    if(!session.isValid())
    {
        response.setErrno(WRONG_TOKEN_ERROR);
        answer.append(response.getJson());
        DEBUG() << "invalid session";
        return answer;
    }

    Core::MetaCache * cache = Core::MetaCache::getMetaCache(session);
    BasicUser user = session.getUser();
    QList<Channel> channels = cache->getSubscribedChannels(user);
    QList<Tag> feed;
     
    for(int i = 0; i<channels.size(); i++)
    {
      Channel channel = channels.at(i);
      QList<Tag > tags = cache->loadTagsWithSubstring(request.getField(), request.getSubstring(), channel);

      extractLastNTags( tags, request.getTagNumber());
      feed << tags;
    }
    DEBUG() << "Filtred tags number " << feed.size();

    response.setTags(feed);
    response.setErrno(SUCCESS);
    answer.append(response.getJson());
    DEBUG() << "answer: " << answer.data();
    return answer;
}

} // namespace common
