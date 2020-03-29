#ifndef LOG_H
#define LOG_H
#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>
#include <QFile>
#include <QTextStream>
#include <QtDebug>
#include <QtMessageHandler>
#include <QProcessEnvironment>
#include <QDateTime>
#include <QIODevice>
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#endif // LOG_H
