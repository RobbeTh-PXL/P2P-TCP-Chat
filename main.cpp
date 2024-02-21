#include "mainwindow.h"

#include <QApplication>

//TCP
#include "p2p_network.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	//TCP
	p2p_network network;

	network.connection_listener();

	//network.join_network();

    MainWindow w;
    w.show();
    return a.exec();
}
