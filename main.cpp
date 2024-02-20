#include "mainwindow.h"

#include <QApplication>

//TCP
#include "tcp_network.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	//TCP
	tcp_network peernetwork;

    MainWindow w;
    w.show();
    return a.exec();
}
