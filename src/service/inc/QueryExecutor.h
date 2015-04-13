/*
 * Copyright 2010-2011  OSLL osll@osll.spb.ru
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
/*----------------------------------------------------------------- !
 * PROJ: OSLL/geo2tag
 * ---------------------------------------------------------------- */

#ifndef QUERYEXECUTOR_H
#define QUERYEXECUTOR_H

#include <QObject>

#include "User.h"
#include "Channel.h"
#include "DataMarks.h"
#include "Session.h"
#include "Geo2tagDatabase.h"
#include <QList>
#include <QSqlQuery>

class QueryExecutor : public QObject
{
    Q_OBJECT

    static QMap<QString, QueryExecutor*> s_executorsMap;

    //Geo2tagDatabase m_database;

    qlonglong nextKey(const QString& sequence) const;
    QueryExecutor(const QString& dbName);

    QString m_dbName;

    void retrieveTagsToList(QList<Tag>& container, QSqlQuery& query,
	 const Channel& channel=Channel());

    void loadTagsToContainerFromChannel(QList<Tag> & container, Channel channel);

public:


    qlonglong nextUserKey() const;
    qlonglong nextChannelKey() const;
    qlonglong nextTagKey() const;
    qlonglong nextSessionKey() const;

    const QString generateNewToken(const QString& email, const QString& login,const QString& password) const;
    const QString generateNewToken(const QString& accessTime, const QString& email, const QString& login,const QString& password) const;

    bool                     subscribeChannel(const common::BasicUser& user,const Channel& channel);
    bool                     unsubscribeChannel(const common::BasicUser& user,const Channel& channel);
    bool                 doesTmpUserExist(const common::BasicUser &user);
    bool                     doesUserWithGivenEmailExist(const common::BasicUser &user);
    bool                     deleteTmpUser(const common::BasicUser &user);
    bool  insertNewTmpUser(const common::BasicUser &user);
    bool                     doesRegistrationTokenExist(const QString &token);
    common::BasicUser insertTmpUserIntoUsers(const QString &token);
    bool                     deleteTmpUser(const QString &token);
    bool insertNewTag(const Tag& tag);
    bool insertNewUser(const common::BasicUser&);
    bool  insertNewChannel(const Channel &, const common::BasicUser& user);
    bool                     deleteUser(const common::BasicUser &user);
    common::BasicUser  updateUserPassword(common::BasicUser &user, const QString& password);
    // Sessions
    bool insertNewSession(const Session &session);

    bool                     updateSession(Session& session);
    bool                     deleteSession(const Session& session);
    bool isOwner(const QString& name, const QString& channel);

    QList<common::BasicUser> loadUsers();

    bool checkEmail(const QString& email);

    common::BasicUser getUser(const QString& login);
    common::BasicUser getUserById(qlonglong id);

    Channel getChannelById(qlonglong id);

    QString getTagsChannelNameByGuid(const QString & uuid);


    QList<Channel> getChannelsByOwner(const common::BasicUser& user );
    QList<Channel> getSubscribedChannels(const common::BasicUser& user );
    QList<Tag> loadTags();
    QList<Tag> loadTags(const Channel &channel);

    QList<Channel> loadChannels();
    QList<Session> loadSessions();
    void updateReflections(DataMarks&, QList<common::BasicUser>&, Channels&, Sessions&);
    qlonglong getUserIdByName(const QString& name);
    qlonglong getChannelIdByName(const QString& name);
    Channel getChannel(const QString& name);
    bool isSubscribed(const common::BasicUser& user, const Channel& channel);

    bool alterChannel(const QString& name, const QString& field, const QString& value);

    bool changePassword(const QString& login, const QString& newPassword);

    qlonglong getFactTransactionNumber();

    QSqlQuery makeQuery() const;
    static void transaction();
    static void rollback();
    static void commit();
    static QueryExecutor* getInstance(const QString& dbName);

    static bool checkDbExistance(const QString& dbName);
    static bool initDatabase(const QString& dbName);


    QList<Tag> loadTagsWithSubstring(const QString& field, const QString& substring,
	 const Channel &channel);

    

};
#endif                                  // QUERYEXECUTOR_H
