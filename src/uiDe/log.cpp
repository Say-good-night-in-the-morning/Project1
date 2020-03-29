#include "log.h"

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txtMessage;

    switch (type)
    {
        case QtDebugMsg:    //调试信息提示
            txtMessage = QString("Debug: %1").arg(msg);
            break;

        case QtWarningMsg:    //一般的warning提示
            txtMessage = QString("Warning: %1").arg(msg);
            break;

        case QtCriticalMsg:    //严重错误提示
            txtMessage = QString("Critical: %1").arg(msg);
            break;

        case QtFatalMsg:    //致命错误提示
            txtMessage = QString("Fatal: %1").arg(msg);
            abort();
    }

    //保存输出相关信息到指定文件
    QFile outputFile("Log.txt");
    outputFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&outputFile);
    textStream << txtMessage << endl;
}
