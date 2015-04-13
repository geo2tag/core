/*
 * Copyright 2013  Kirill Krinkin  kirill.krinkin@gmail.com
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AS IS'' AND ANY EXPRESS OR
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
 * \file MetaCache.h
 * \brief Header of MetaCache
 * \todo add comment here
 *
 * File description
 *
 * PROJ: OSLL/geo2tag
 * ---------------------------------------------------------------- */


#ifndef _MetaCache_H_E337695F_C96B_4956_A169_5053A7AF8B1A_INCLUDED_
#define _MetaCache_H_E337695F_C96B_4956_A169_5053A7AF8B1A_INCLUDED_

#include <QReadWriteLock>

#include "Channel.h"
#include "User.h"
#include "Session.h"
#include "DataMarks.h"

class QueryExecutor;

namespace Core
{
    using namespace common;
 /*!
   * Cache for meta information: Users, Sessions, Channels...
   *
   */
  class MetaCache
  {
      static QMap<QString, MetaCache*> s_cachesMap;

      QueryExecutor* m_queryExecutor;

      QList<Channel>     m_channels;
      QList<Session>     m_sessions;
      QList<BasicUser>   m_users;

      QReadWriteLock      m_cacheLock;
      QReadWriteLock      m_usersLock;
      QReadWriteLock      m_channelsLock;
      QReadWriteLock      m_SessionsLock;


      bool isDefaultMetaCache() const;

  public:

      MetaCache(const QString& dbName);

      static MetaCache* getMetaCache(const Session& session);
      static MetaCache* getMetaCache(const QString& dbName);
      static MetaCache* getDefaultMetaCache();

      static void init();
      BasicUser getUserById(const QString userId);
      QList<BasicUser> getUsers();
      QList<Channel> getChannels();

      bool alterChannel(const QString& name, const QString& field, const QString& value);
      bool addChannel(const Channel &channel, const BasicUser &user);
      bool addUser(const BasicUser &user);
      bool deleteUser(const BasicUser& user);
      void insertSession(const Session &session);
      Channel getChannel(const QString name);
      QList<Channel> getChannels(const common::BasicUser& user);
      QList<Channel> getChannelsByOwner(const BasicUser &user);

      bool addBlobToRiakBD(const QString & guid, const QString & blob);
      bool getBlobFromRiakBD(const QString & guid, QString * blob);

      void updateSession(Session &session);

      void removeSession(const Session& session);

      Session findSession(const common::BasicUser& user);
      Session findSession(const QString& token);
      Channel findChannel(const QString& name);
      QList<Channel> getSubscribedChannels(const BasicUser& user);

      bool subscribeChannel(const common::BasicUser& user, const Channel& channel);
      bool unsubscribeChannel(const common::BasicUser& user, const Channel& channel);

      // Reloading sessions from db (used after adding session)
      void reloadSessions();

      bool checkEmail(const QString& email);

      // returns true if user exists and loaded
      bool checkUser(common::BasicUser &user);

      // returns true if channed exists and user is subscribed
      bool testChannel(common::BasicUser &user, const Channel& channel);

      bool writeTag(const Tag& tag);
      QList<Tag> loadTagsFromChannel(const Channel& channel);

      BasicUser findUserByName(const QString& name);

      bool changePassword(const QString& login, const QString& newPassword);

      void changeDbName(const Session& session, const QString& dbName);

      bool isOwner(const Session& session, const QString& channel);

      void touchUserToServiceDb(const common::BasicUser& user);

      QList<Tag> loadTagsWithSubstring(const QString& field, const QString& substring,
	 const Channel &channel);

  protected:

      void initUsers();
      void initSessions();
      void initChannels();
    
  private:    
    MetaCache(const MetaCache& obj);
    MetaCache& operator=(const MetaCache& obj);

  }; // class MetaCache
  
} // namespace Core

#endif //_MetaCache_H_E337695F_C96B_4956_A169_5053A7AF8B1A_INCLUDED_
