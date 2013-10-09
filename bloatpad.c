#include <stdlib.h>
#include <gtk/gtk.h>

static void
activate_toggle (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	GVariant *state;

	state = g_action_get_state (G_ACTION (action));
	g_action_change_state (G_ACTION (action), g_variant_new_boolean (!g_variant_get_boolean (state)));
	g_variant_unref (state);
}

static void
change_busy_state (GSimpleAction *action, GVariant *state, gpointer user_data)
{
	GtkWindow *window = user_data;
	GApplication *application = G_APPLICATION (gtk_window_get_application (window));

	/* do this twice to test multiple busy counter increases */
	if (g_variant_get_boolean (state)) {
		g_application_mark_busy (application);
	} else {
		g_application_unmark_busy (application);
	}

	g_simple_action_set_state (action, state);
}

static GActionEntry win_entries[] = {
	{ "busy", activate_toggle, NULL, "false", change_busy_state },
};

static void
new_window (GApplication *app)
{
	GtkWidget *window, *grid, *scrolled, *view;

	window = gtk_application_window_new (GTK_APPLICATION (app));
	gtk_window_set_default_size ((GtkWindow*)window, 640, 480);
	g_action_map_add_action_entries (G_ACTION_MAP (window), win_entries, G_N_ELEMENTS (win_entries), window);
	gtk_window_set_title (GTK_WINDOW (window), "Bloatpad");

	grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (window), grid);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_hexpand (scrolled, TRUE);
	gtk_widget_set_vexpand (scrolled, TRUE);
	view = gtk_text_view_new ();

	g_object_set_data ((GObject*)window, "bloatpad-text", view);

	gtk_container_add (GTK_CONTAINER (scrolled), view);

	gtk_grid_attach (GTK_GRID (grid), scrolled, 0, 1, 1, 1);

	gtk_widget_show_all (GTK_WIDGET (window));
}

static void
bloat_pad_activate (GApplication *application)
{
	new_window (application);
}

typedef struct
{
	GtkApplication parent_instance;
} BloatPad;

typedef GtkApplicationClass BloatPadClass;

G_DEFINE_TYPE (BloatPad, bloat_pad, GTK_TYPE_APPLICATION)

static void
bloat_pad_startup (GApplication *application)
{
	GtkBuilder *builder;

	G_APPLICATION_CLASS (bloat_pad_parent_class)->startup (application);

	builder = gtk_builder_new ();
	gtk_builder_add_from_string (builder,
		"<interface>"
		"  <menu id='menubar'>"
		"    <submenu>"
		"      <attribute name='label' translatable='yes'>_View</attribute>"
		"      <section>"
		"        <item>"
		"          <attribute name='label' translatable='yes'>_Look Busy</attribute>"
		"          <attribute name='action'>win.busy</attribute>"
		"        </item>"
		"      </section>"
		"    </submenu>"
		"  </menu>"
		"</interface>", -1, NULL);
	gtk_application_set_menubar (GTK_APPLICATION (application), G_MENU_MODEL (gtk_builder_get_object (builder, "menubar")));
	g_object_unref (builder);
}

static void
bloat_pad_init (BloatPad *app)
{
}

static void
bloat_pad_class_init (BloatPadClass *class)
{
	GApplicationClass *application_class = G_APPLICATION_CLASS (class);

	application_class->startup = bloat_pad_startup;
	application_class->activate = bloat_pad_activate;
}

BloatPad *
bloat_pad_new (void)
{
	BloatPad *bloat_pad;

	g_set_application_name ("Bloatpad");

	bloat_pad = g_object_new (bloat_pad_get_type (),
		"application-id", "org.gtk.Test.bloatpad",
		"flags", G_APPLICATION_FLAGS_NONE,
		"register-session", TRUE,
		NULL);

	return bloat_pad;
}

int
main (int argc, char **argv)
{
	BloatPad *bloat_pad;
	int status;

	bloat_pad = bloat_pad_new ();

	status = g_application_run (G_APPLICATION (bloat_pad), argc, argv);

	g_object_unref (bloat_pad);

	return status;
}
