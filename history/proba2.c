#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <time.h>

//static GtkWidget *window = NULL;

GdkPixbuf *create_pixbuf(const gchar *filename) {
  //image loading library

  GdkPixbuf *pixbuf;
  GError *error = NULL;
  pixbuf = gdk_pixbuf_new_from_file(filename, &error);

  if (!pixbuf) {
    fprintf(stderr, "%s\n", error->message);
    g_error_free(error);
  }
  return pixbuf;
}

int local_datetime(char choice){
  time_t T = time(NULL);
  struct tm tm = *localtime(&T);

  //printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900);
  //printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
  int result;
  switch(choice){
    case 'd':
      result = tm.tm_mday;
      break;
    case 'M':
      result = tm.tm_mon+1;
      break;
    case 'y':
      result = tm.tm_year+1900;
      break;

    case 'h':
      result = tm.tm_hour;
      break;
    case 'm':
      result = tm.tm_min;
      break;
  }
}


void show_add_toolb(GtkWindow *parent, gpointer user_data){

  GtkWidget *content_area;
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *table;

  GtkWidget *local_entry_day, *local_entry_month, *local_entry_year;
  GtkWidget *local_entry_hour, *local_entry_minutes;
  GtkWidget *local_entry_describe;

  GtkWidget *label;
  gint response;
  GtkAdjustment *adjustment;

  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

  dialog = gtk_dialog_new_with_buttons("Dodaj", parent, //parent ?= GTK_WINDOW(window)
					flags,
					("OK"), GTK_RESPONSE_OK,
					"Anuluj", GTK_RESPONSE_CANCEL,
					NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_container_set_border_width(GTK_CONTAINER (hbox), 30);

  gtk_box_pack_start(GTK_BOX(content_area), hbox, FALSE, FALSE, 0);

  table = gtk_grid_new();
  gtk_grid_set_row_spacing (GTK_GRID(table), 20); //odleglosc od obiektow wierszami
  gtk_grid_set_column_spacing (GTK_GRID(table), 25); //kolumnami
  gtk_box_pack_start (GTK_BOX(hbox), table, TRUE, TRUE, 0);


//Wpis
  label = gtk_label_new("Wydarzenie:");
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
  local_entry_describe = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(table), local_entry_describe, 1, 0, 4, 1);
                                                       //c, r, w, h
//Data
  //dzien
  adjustment = gtk_adjustment_new(local_datetime('d'), 1, 32, 1, 1, 1);

  label = gtk_label_new("Dzień:");
  gtk_grid_attach (GTK_GRID (table), label, 1, 1, 1, 1);
  local_entry_day = gtk_spin_button_new(adjustment, 1.0, 0);
  //gtk_entry_set_text (GTK_ENTRY (local_entry1), gtk_entry_get_text (GTK_ENTRY (entry1)));
  gtk_grid_attach (GTK_GRID (table), local_entry_day, 1, 2, 1, 1);

  //miesiac
  adjustment = gtk_adjustment_new(local_datetime('M'), 1, 13, 1, 1, 1);

  label = gtk_label_new("Miesiąc:");
  gtk_grid_attach(GTK_GRID(table), label, 2, 1, 1, 1);
  local_entry_month = gtk_spin_button_new(adjustment, 1.0, 0);
  gtk_grid_attach(GTK_GRID(table), local_entry_month, 2, 2, 1, 1);

  //rok
  adjustment = gtk_adjustment_new(local_datetime('y'), local_datetime('y'),
				  local_datetime('y')+10, 1, 1, 1);

  label = gtk_label_new("Rok:");
  gtk_grid_attach(GTK_GRID(table), label, 3, 1, 1, 1);
  local_entry_year = gtk_spin_button_new(adjustment, 1.0, 0);
  gtk_grid_attach(GTK_GRID(table), local_entry_year, 3, 2, 1, 1);

//Czas
  //godzina
  adjustment = gtk_adjustment_new(local_datetime('h'), 0, 24, 1, 1, 1);

  label = gtk_label_new("Godzina:");
  gtk_grid_attach (GTK_GRID (table), label, 1, 3, 1, 1);
  local_entry_hour = gtk_spin_button_new(adjustment, 1.0, 0);
  //gtk_entry_set_text (GTK_ENTRY (local_entry2), gtk_entry_get_text (GTK_ENTRY (entry2)));
  gtk_grid_attach (GTK_GRID (table), local_entry_hour, 1, 4, 1, 1);

  //minuty
  adjustment = gtk_adjustment_new(local_datetime('m'), 0, 60, 1, 1, 1);

  label=gtk_label_new("Minuty:");
  gtk_grid_attach(GTK_GRID(table), label, 2, 3, 1, 1);
  local_entry_minutes = gtk_spin_button_new(adjustment, 1.0, 0);
  gtk_grid_attach(GTK_GRID(table), local_entry_minutes, 2, 4, 1, 1);


  g_signal_connect_swapped(dialog, "response",
			   G_CALLBACK(gtk_widget_destroy), dialog);

  gtk_widget_show_all (hbox);
  response = gtk_dialog_run(GTK_DIALOG(dialog));

  gtk_widget_destroy(dialog);
}


//EDIT
void show_edit_toolb(GtkWindow *parent, gchar *message){

  GtkWidget *dialog, *label, *content_area;
  GtkDialogFlags flags;

// Create the widgets
  flags = GTK_DIALOG_DESTROY_WITH_PARENT;
  dialog = gtk_dialog_new_with_buttons("Message", parent, flags, ("OK"),
           GTK_RESPONSE_NONE, NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  label = gtk_label_new(message);

// Ensure that the dialog box is destroyed when the user responds
  g_signal_connect_swapped(dialog, "response",
                          G_CALLBACK (gtk_widget_destroy), dialog);

// Add the label, and show everything we’ve added
  gtk_container_add(GTK_CONTAINER(content_area), label);
  gtk_widget_show_all(dialog);
}


//OPTIONS
// Function to open a dialog box with a message
void quick_message (GtkWindow *parent, char *message){

 GtkWidget *dialog, *label, *content_area;
 GtkDialogFlags flags;

 // Create the widgets
 flags = GTK_DIALOG_DESTROY_WITH_PARENT;
 dialog = gtk_dialog_new_with_buttons ("Message",
                                       parent,
                                       flags,
                                       ("_OK"),
                                       GTK_RESPONSE_NONE,
                                       NULL);
 content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
 label = gtk_label_new (message);

 // Ensure that the dialog box is destroyed when the user responds

 g_signal_connect_swapped (dialog,
                           "response",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog);

 // Add the label, and show everything we’ve added

 gtk_container_add(GTK_CONTAINER(content_area), label);
 gtk_widget_show_all(dialog);
}



//INFO
void show_info_toolb(GtkWidget *widget, gpointer data){

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("ikona.jpeg", NULL);
  GtkWidget *dialog = gtk_about_dialog_new();

  gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Przypominacz");
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "v0.1");
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),"autor: Ewa Namysł");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
     "Proste narzędzie do przypominania o nadchodzących wydarzeniach.");
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog),
     "http://www.github.com/namysl");
  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);

  g_object_unref(pixbuf), pixbuf = NULL;
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}


int main(int argc, char *argv[]){

  GtkWidget *window;
  GdkPixbuf *icon;

  GtkWidget *box;
  GtkWidget *toolbar;
  GtkToolItem *add_toolb;
  GtkToolItem *edit_toolb;
  GtkToolItem *delete_toolb;
  GtkToolItem *separator1;
  GtkToolItem *separator2;
  GtkToolItem *options_toolb;
  GtkToolItem *info_toolb;
  GtkToolItem *exit_toolb;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

//tytul na pasku okna
  gtk_window_set_title(GTK_WINDOW(window), "Przypominacz");
//rozmiar i umiejscowienie okna programu
  gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
//ikona programu
  icon = create_pixbuf("ikona.jpeg");
  gtk_window_set_icon(GTK_WINDOW(window), icon);


//container dla paska narzedzi
  box = gtk_box_new(FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE (box), GTK_ORIENTATION_VERTICAL);
  gtk_container_add(GTK_CONTAINER(window), box);
  toolbar = gtk_toolbar_new();
//tylko ikony w pasku narzedzi
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);


//dodaj
  add_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name
              ("list-add-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), add_toolb, -1);
  gtk_tool_item_set_tooltip_markup(add_toolb, "Dodaj");
//edytuj
  edit_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name
               ("edit-copy-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), edit_toolb, -1);
  gtk_tool_item_set_tooltip_markup(edit_toolb, "Edytuj");
//usuń
  delete_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name
                 ("edit-delete-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), delete_toolb, -1);
  gtk_tool_item_set_tooltip_markup(delete_toolb, "Usuń");
//separator1
  separator1 = gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator1, -1);
//opcje
  options_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name
                  ("content-loading-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), options_toolb, -1);
  gtk_tool_item_set_tooltip_markup(options_toolb, "Opcje");
//separator2
  separator2 = gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator2, -1);
//info
  info_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name
               ("dialog-information-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), info_toolb, -1);
  gtk_tool_item_set_tooltip_markup(info_toolb, "Info");
//wyjście
  exit_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name
               ("application-exit-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit_toolb, -1);
  gtk_tool_item_set_tooltip_markup(exit_toolb, "Wyjdź");


  gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, FALSE, 5);


//sygnały
  g_signal_connect(G_OBJECT(add_toolb), "clicked",
                   G_CALLBACK(show_add_toolb), NULL);// window

  g_signal_connect(G_OBJECT(edit_toolb), "clicked",
		   G_CALLBACK(show_edit_toolb), NULL);

  g_signal_connect(G_OBJECT(options_toolb), "clicked",
		   G_CALLBACK(quick_message), NULL);

  g_signal_connect(G_OBJECT(info_toolb), "clicked",
                   G_CALLBACK(show_info_toolb), NULL);// (gpointer) window);

  g_signal_connect(G_OBJECT(exit_toolb), "clicked",
                   G_CALLBACK(gtk_main_quit), NULL);

  g_signal_connect(G_OBJECT(window), "destroy",
                   G_CALLBACK(gtk_main_quit), NULL);


  gtk_widget_show_all(window);
  g_object_unref(icon);
  gtk_main();

  return 0;
}
