using Gee;
using Clutter;

namespace Zizelo {
	public class Page : Object {

		private DataInputStream _dis;
		private string _raw_page;
		private bool _is_menu;
		private Color text_color;

		public ArrayList<Actor> parts;
		private BoxLayout layout;
		public Box actor;

		public Page(DataInputStream dis, bool? is_menu = true) {
			_dis = dis;
			_is_menu = is_menu;
			
			parts = new ArrayList<Actor>();

			layout = new BoxLayout();
			actor = new Box(layout);
			text_color = { 0xff, 0xff, 0x00, 0xff };

			this.render();
		}

		public void render () {
			if (_is_menu) {
				render_menu ();
			} else {
				render_plain_text ();
			}
		}
		
		public void add_text (string text) {
			Text text_actor;
			text_actor = create_text_actor (text);
			parts.add(text_actor);
			layout.pack(text_actor,
						true, // expand
						true, // x-fill
						true, // y-fill
						Clutter.BoxAlignment.CENTER,
						Clutter.BoxAlignment.CENTER);
		}
		
		private Text create_text_actor (string text) {
			Clutter.Text text_actor = new Clutter.Text();
			text_actor.set_markup (@"<tt>$text</tt>");
			text_actor.set_color (text_color);
			text_actor.set_selectable (true);
			return text_actor;
		}
		
		private void render_menu () {
			string line;
			string desc;
			char type;

			try {
				while ((line = _dis.read_line(null)) != null && line != ".") {
					type = line[0];
					desc = this.get_gophermap_segment (line, 0);
					
					switch (type) {
					case 'i':
						add_text (line + "\n");
						break;
					case '1':
					case 'I':
						// add_link ();
						break;
					}
					
				}
			} catch (Error e) {
				debug (@"Cannot read line $(e.message)");
			}
		}
		
		private void render_plain_text () {
			string line;
			string text = "";
			try {
				while ((line = _dis.read_line(null)) != null) {
					text += line + "\n";
				}
			} catch (Error e) {
				debug (@"Cannot read line $(e.message)");
			}
			add_text(text);
		}
		
		private string get_gophermap_segment (string line, int num) {
			var segment = line.split("\t")[num];
			return segment;
		}
	}
}
