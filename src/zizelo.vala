namespace Zizelo {
	
	class App : Object {
		
		Gtk.Application app;
		
		public Gtk.Window  window;
		public Gtk.Builder builder;
		
		public App (string application_id) {
			app = new Gtk.Application (application_id,
									   ApplicationFlags.HANDLES_OPEN);
			app.activate.connect(() => print(""));
		}
		
		public int run (string[] args) {
			try {
				builder = new Gtk.Builder ();
				builder.add_from_file ("../data/zizelo.ui");
				builder.connect_signals (null);
				window = builder.get_object ("zizelo") as Gtk.Window;
				app.add_window(window);
				window.show_all ();
				window.destroy.connect (Gtk.main_quit);
			} catch (Error e) {
				stderr.printf ("Could not load UI: %s\n", e.message);
			}
			return app.run(args);
		}
		
		private void on_addressbar_changed () {
			print ("Addr changed");
		}
	}
}


static int main (string[] args) {     
	Gtk.init(ref args);
	var app = new Zizelo.App ("info.antono.Zizelo");
	var status = app.run(args);
	Gtk.main();
	return status;
}
