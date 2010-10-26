#include <QDebug>
#include <QCoreApplication>
#include "TrackerDaemon.h"


int main(int c, char **v)
{
  QCoreApplication app(c,v);
  qDebug() << "Application is started";
  TrackerDaemon daemon;
  return app.exec();
}
