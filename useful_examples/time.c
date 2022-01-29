#include <gtk/gtk.h>

static gbooleanon_timeout (gpointer user_data)
{
  static unsigned f_times = 0;

  GtkLabel *label = GTK_LABEL (user_data);

  ++f_times;
  gchar *text = g_strdup_printf ("I have been updated %u times", f_times);
  gtk_label_set_label (label, text);
  g_free (text);

  return G_SOURCE_CONTINUE; /* or G_SOURCE_REMOVE when you want to stop */
}

int
main (int    argc,
      char **argv)
{
  gtk_init (&argc, &argv);

  GtkWidget *label = gtk_label_new ("not updated yet...");
  g_timeout_add (1000 /* milliseconds */, on_timeout, label);

  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_add (GTK_CONTAINER (window), label);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
