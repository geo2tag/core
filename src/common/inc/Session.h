/*
 * Copyright 2012  OSLL osll@osll.spb.ru
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
/*!
 * \file  Session.h
 * \brief Declaration of the class Session
 *
 * PROJ: OSLL/geo2tag
 * ---------------------------------------------------------------- */

#ifndef SESSION_H
#define SESSION_H

#include <QSharedPointer>
#include <QString>
#include <QDateTime>
#include <QDebug>

#include "User.h"

class Session
{
  private:
    QString             m_token;
    QDateTime           m_accessTime;
    common::BasicUser   m_user;

    QString             m_dbName;

  public:

    Session(const QString& token, const QDateTime& accessTime = QDateTime::currentDateTime(),
            const common::BasicUser& user=common::BasicUser());
    Session(const Session& obj);
    Session();
    Session& operator=(const Session& obj);
    bool operator==(const Session& obj);

    void setSessionToken(const QString& sessionToken);
    void setLastAccessTime(const QDateTime& lastAccessTime = QDateTime::currentDateTime());
    void setUser(const common::BasicUser& user);

    bool isValid() const { return !m_token.isEmpty(); }
    bool isExpired() const;

    const QString& getSessionToken() const;
    const QDateTime& getLastAccessTime() const;
    common::BasicUser getUser() const;

    static QString generateToken(const common::BasicUser& user);

    virtual ~Session();

    QString getDbName() const;
    void setDbName(const QString& dbName);
};

QDebug& operator<<(QDebug &dbg, Session const& session);


typedef ConcurrentVector<Session> Sessions;
#endif                                  // SESSION_H
