#include "gmpmpc.h"
#include "../types.h"
#include "../network-handler.h"
#include "../error-handling.h"
#include "gmpmpc_main_window.glade.h"
#include "gmpmpc_select_server.h"
#include <boost/program_options.hpp>

#include "../audio/backend_sdlmixer.h"

using namespace std;
namespace po = boost::program_options;
network_handler* gmpmpc_network_handler; //FIXME: Global variables == 3vil

int main ( int argc, char *argv[] )
{
	fprintf(stderr, "Starting GUI..\n");
	gtk_init (&argc, &argv);
	glade_init();


	int listen_port;
	bool showhelp;
	string filename;
	po::options_description desc("Allowed options");
	desc.add_options()
			("help", po::bool_switch(&showhelp)                   , "produce help message")
			("port", po::value(&listen_port)->default_value(12345), "TCP Port")
			("file", po::value(&filename)->default_value(""), "Filename to test with")
	;
	po::variables_map vm;
 	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	if (showhelp) {
		cout << desc << "\n";
		return 1;
	}

	SDLMixerBackend sdlmbe(NULL);
	sdlmbe.test_playback(filename.c_str());

	network_handler nh(listen_port);
	gmpmpc_network_handler = &nh;
// 	nh.add_server_signal.connect(server_lister_tmp);

// 	while(1) {
// 		nh.get_available_servers();
// 		return 0;
// 	}


	uint32 retval = show_gui();
	fprintf(stderr, "Terminating GUI (%i)\n", retval);
	return retval;
}

/***
 * NOTE:
 * Keep the bindings/connections in this file, and put the actual
 * handler's routines in other files.
 */

uint32 show_gui() {
	/* load the interface from the xml buffer */
	GladeXML *main_window = glade_xml_new_from_buffer(gmpmpc_main_window_glade_definition, gmpmpc_main_window_glade_definition_size, NULL, NULL);
	if(!select_server_initialise_window()) {
		fprintf(stderr, "Error while loading server select window!\n");
		return false;
	}

	/* connect the signals in the interface */
	try_connect_signal(main_window, imagemenuitem_select_server, activate);

	/* Have the delete event (window close) end the program */
	GtkWidget* widget = glade_xml_get_widget (main_window, "gmpmpc_main_window");
	g_signal_connect (G_OBJECT (widget), "delete_event", G_CALLBACK (gtk_main_quit), NULL);

	/* start the event loop. TODO: Do this in the background? */
	//spawnThread(gtk_main(), NULL); ?
	gtk_main ();
	return true;
}
