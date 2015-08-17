#include "mainwindow.h"
#include <QApplication>
#include <QtWidgets>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("Malti-View"));
    QCoreApplication::setApplicationVersion(QStringLiteral("alpha"));


    QCommandLineParser parser;
       parser.setApplicationDescription(QCoreApplication::translate("main",
            "Light weight minimum browser."));
       parser.addHelpOption();
       parser.addVersionOption();

       parser.addPositionalArgument(QStringLiteral("url"),
            QCoreApplication::translate("main","The URL to load at start up"));

       parser.process(a);

    const QStringList args = parser.positionalArguments();
    QUrl url;

   if (args.size() > 1) {
       fprintf(stderr, "%s\n", qPrintable(QCoreApplication::translate("main",
            "Error: Must specify one URL argument.")));
       parser.showHelp(1);
       return -1;
    }
   else if (args.size() == 1)
       url = QUrl::fromUserInput(args.at(0));
   else
   {
       //ToDo Add config file for home page
   }

   if (!url.isValid()) {
       fprintf(stderr, "%s\n", qPrintable(QCoreApplication::translate("main",
            "Error: %1 is an Invalid URL").arg(args.at(0))));
       parser.showHelp(1);
       return -1;
    }

    MainWindow w(url);
    w.show();

    return a.exec();
}
