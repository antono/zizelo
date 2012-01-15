namespace Zizelo {
	public class View : Object {
		
		GtkClutter.Embed clutter_widget;
		Clutter.Actor stage;
		Clutter.Group viewport;
		Zizelo.Page current_page;
		Clutter.Color stage_color;
		
		public View (Gtk.VBox vbox) {
			stage_color = { 0x00, 0x00, 0x00, 0xff };
			
			this.attach_clutter_embed (vbox);
			this.setup_stage ();
			this.setup_viewport ();
		}
		
		public void set_current_page (Zizelo.Page page) {
			if (current_page != null) {
				viewport.remove_actor (current_page.actor);
			}
			
			current_page = page;
			
			viewport.add (current_page.actor);
			current_page.actor.set_position (20, 20);
		}
		
		public void attach_clutter_embed (Gtk.VBox vbox) {
			clutter_widget = new GtkClutter.Embed();
			
			// Packing widget to main window
			vbox.pack_start(clutter_widget, true, true, 0);
			vbox.reorder_child(clutter_widget, 1);
		}
		
		private void setup_stage () {
			// Getting Clutter Stage
			stage = clutter_widget.get_stage();
			(stage as Clutter.Stage).set_color(stage_color);
		}
		
		private void setup_viewport () {
			// Setting up scrollable viewport
			viewport = new Clutter.Group();
			viewport.set_reactive(true);
			var constraint = new Clutter.AlignConstraint(stage,
														 Clutter.AlignAxis.Y_AXIS,
														 0.5f);
			viewport.add_constraint(constraint);
			viewport.set_clip_to_allocation(true);
			(stage as Clutter.Container).add_actor(viewport);
		}
	}
}
