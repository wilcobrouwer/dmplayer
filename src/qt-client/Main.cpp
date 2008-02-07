#include <QApplication>

#include "MainWindow.h"
#include "../network-handler.h"
#include <boost/bind.hpp>
#include "QtBooster.h"

using namespace std;

int main( int argc, char **argv )
{
	QApplication app(argc, argv);

	MainWindow mainwindow;
	mainwindow.show();

	qRegisterMetaType<std::vector<server_info> >("std::vector<server_info>");
	qRegisterMetaType<server_info>("server_info");
	qRegisterMetaType<std::string>("std::string");

	network_handler nh(12345);
	nh.server_list_update_signal.connect(
		QTBOOSTER(&mainwindow, MainWindow::UpdateServerList)
	);

	nh.server_list_removed_signal.connect(
		QTBOOSTER(&mainwindow, MainWindow::UpdateServerList_remove)
	);

	mainwindow.sigconnect.connect(
		boost::bind(
			&network_handler::client_connect_to_server,
			&nh, _1
		)
	);

	return app.exec();
}
