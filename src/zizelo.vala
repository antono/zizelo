namespace Zizelo {

	Regex domain_regex;

	Gtk.Builder builder;
	Gtk.Application app;
	Gtk.Window window;

	Zizelo.Net net;
	Zizelo.Page current_page;
	Zizelo.View view;
	
	class App : Object {

		public App () {
			app = new Gtk.Application ("info.antono.Zizelo", ApplicationFlags.HANDLES_OPEN);
			app.activate.connect(() => print("activated\n"));
		}

		public int run (string[] args) {
			Zizelo.net = new Zizelo.Net();
			
			this.setup_builder ();
			this.setup_view ();
			this.setup_window ();
			this.setup_addressbar ();
			this.setup_domain_regex ();
			window.show_all ();
			return app.run(args);
		}
		
		private void setup_builder () {
			try {
				builder = new Gtk.Builder ();
				builder.add_from_file ("./zizelo.ui");
			} catch (Error e) {
				stderr.printf ("Could not load UI: %s\n", e.message);
			}
			builder.connect_signals (null);
		}
		
		private void setup_addressbar () {
			var entry = builder.get_object ("addressbar") as Gtk.Entry;
			entry.set_icon_from_stock(Gtk.EntryIconPosition.PRIMARY, Gtk.Stock.FIND);
		}

		private void setup_window () {
			window = builder.get_object ("zizelo") as Gtk.Window;
			app.add_window(window);
			window.destroy.connect (Gtk.main_quit);
		}

		private void setup_view () {
			var vbox = builder.get_object("vbox") as Gtk.VBox;
			view = new Zizelo.View (vbox);
		}

		private void setup_domain_regex () {
			try {
				domain_regex = new Regex ("""([A-Z0-9.-]+\.[A-Z]{2,4})$""",
										  RegexCompileFlags.CASELESS);
			} catch (RegexError e) {
				stderr.printf ("Wrong regex: %s\n", e.message);
			}
		}
	}
}


static int main (string[] args) {
	GtkClutter.init(ref args);
	
	var app = new Zizelo.App ();
	var status = app.run(args);
	
	Gtk.main();
	
	return status;
}
