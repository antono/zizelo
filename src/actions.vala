namespace Zizelo {
	namespace Actions {
		namespace Menu {
			[CCode (cname = "G_MODULE_EXPORT zizelo_actions_menu_on_about_clicked")]
			public void on_about_clicked (Gtk.MenuItem item) {
				var dialog = builder.get_object("zizelo_about") as Gtk.Dialog;
				dialog.run();
				dialog.hide();
			}
		}
		
		namespace Addressbar {
			[CCode (cname = "G_MODULE_EXPORT zizelo_actions_addressbar_on_changed")]
			public void on_changed (Gtk.Entry entry) {
				debug (entry.get_text() + "\n");
				if (domain_regex.match(entry.get_text())) {
					entry.set_icon_from_stock(Gtk.EntryIconPosition.PRIMARY, Gtk.Stock.OPEN);
				} else {
					entry.set_icon_from_stock(Gtk.EntryIconPosition.PRIMARY, Gtk.Stock.FIND);
				}
			}

			[CCode (cname = "G_MODULE_EXPORT zizelo_actions_addressbar_on_activate")]
			public void on_activate (Gtk.Entry entry) {
				Zizelo.net.fetch(entry.get_text(), (dis) => {
						try {
							var page = new Zizelo.Page (dis);
							view.set_current_page (page);
						} catch (Error e) {
							debug ("Error: %s", e.message);
						}
					});
			}
		}
	}
}