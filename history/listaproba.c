#include <gtk/gtk.h>

static GtkWidget *window = NULL;
static GtkTreeModel *model = NULL;
static guint timeout = 0;

typedef struct
{
  const gboolean  fixed;
  const guint     number;
  const gchar    *severity;
  const gchar    *description;
}
Bug;


enum{
  COLUMN_TOGGLE,
  COLUMN_DATA,
  COLUMN_TIME,
  COLUMN_DESCRIPTION,
  COLUMN_ACTIVE,
  COLUMN_SENSITIVE,
  NUM_COLUMNS
};


static Bug data[] =
{
  { FALSE, 60482, "Normal",     "scrollable notebooks and hidden tabs" },
  { FALSE, 60620, "Critical",   "gdk_window_clear_area (gdkwindow-win32.c) is not thread-safe" },
  { FALSE, 50214, "Major",      "Xft support does not clean up correctly" },
  { TRUE,  52877, "Major",      "GtkFileSelection needs a refresh method. " },
  { FALSE, 56070, "Normal",     "Can't click button after setting in sensitive" },
  { TRUE,  56355, "Normal",     "GtkLabel - Not all changes propagate correctly" },
  { FALSE, 50055, "Normal",     "Rework width/height computations for TreeView" },
  { FALSE, 58278, "Normal",     "gtk_dialog_set_response_sensitive () doesn't work" },
  { FALSE, 55767, "Normal",     "Getters for all setters" },
  { FALSE, 56925, "Normal",     "Gtkcalender size" },
  { FALSE, 56221, "Normal",     "Selectable label needs right-click copy menu" },
  { TRUE,  50939, "Normal",     "Add shift clicking to GtkTextView" },
  { FALSE, 6112,  "Enhancement","netscape-like collapsable toolbars" },
  { FALSE, 1,     "Normal",     "First bug :=)" },
};


static GtkTreeModel *create_model(void){

  gint i = 0;
  GtkListStore *store;
  GtkTreeIter iter;

  /* create list store */
  store = gtk_list_store_new(NUM_COLUMNS,
                             G_TYPE_BOOLEAN,
                             G_TYPE_UINT,
                             G_TYPE_STRING,
                             G_TYPE_STRING,
                             G_TYPE_BOOLEAN,
                             G_TYPE_BOOLEAN);

  /* add data to the list store */
  for (i=0; i < G_N_ELEMENTS(data); i++){

    gboolean sensitive;

    if (i == 3)
      sensitive = FALSE;
    else
      sensitive = TRUE;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       COLUMN_TOGGLE, data[i].fixed,
                       COLUMN_DATA, data[i].number,
                       COLUMN_TIME, data[i].severity,
                       COLUMN_DESCRIPTION, data[i].description,
                       COLUMN_ACTIVE, FALSE,
                       COLUMN_SENSITIVE, sensitive,
                       -1);
    }

  return GTK_TREE_MODEL(store);
}

static void on_off_toggle(GtkCellRendererToggle *cell, gchar *path_str,
               		 gpointer data){

  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
  gboolean on_off;

  /* get toggled iter */
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, COLUMN_TOGGLE, &on_off, -1);

  /* do something with the value */
  on_off ^= 1;

  /* set new value */
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_TOGGLE, on_off, -1);

  /* clean up */
  gtk_tree_path_free(path);
}

static void add_columns(GtkTreeView *treeview){

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeModel *model = gtk_tree_view_get_model(treeview);

//kolumna z zaznaczaniem
  renderer = gtk_cell_renderer_toggle_new();
  g_signal_connect(renderer, "toggled", G_CALLBACK(on_off_toggle), model);

  column = gtk_tree_view_column_new_with_attributes(" ",
                                                    renderer,
                                                    "active", COLUMN_TOGGLE,
                                                    NULL);

  gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column),
                                  GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(column), 40);  //40px
  gtk_tree_view_append_column(treeview, column);

//kolumna data
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Data",
                                                    renderer,
                                                    "text",
                                                    COLUMN_DATA,
                                                    NULL);
  gtk_tree_view_column_set_sort_column_id(column, COLUMN_DATA);
  gtk_tree_view_append_column(treeview, column);

//kolumna czas
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Czas",
                                                    renderer,
                                                    "text",
                                                    COLUMN_TIME,
                                                    NULL);
  gtk_tree_view_column_set_sort_column_id(column, COLUMN_TIME);
  gtk_tree_view_append_column(treeview, column);

//kolumna opis wydarzenia
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("Opis wydarzenia",
                                                    renderer,
                                                    "text",
                                                    COLUMN_DESCRIPTION,
                                                    NULL);
  gtk_tree_view_column_set_sort_column_id(column, COLUMN_DESCRIPTION);
  gtk_tree_view_append_column(treeview, column);
}


static gboolean
window_closed (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data)
{
  model = NULL;
  window = NULL;
  if (timeout != 0)
    {
      g_source_remove (timeout);
      timeout = 0;
    }
  return FALSE;
}



GtkWidget * do_list_store (GtkWidget *do_widget)
{
  if (!window)
    {
      GtkWidget *vbox;
      GtkWidget *label;
      GtkWidget *sw;
      GtkWidget *treeview;

      /* create window, etc */
      window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_screen (GTK_WINDOW (window),
                             gtk_widget_get_screen (do_widget));
      gtk_window_set_title (GTK_WINDOW (window), "List Store");

      g_signal_connect (window, "destroy",
                        G_CALLBACK (gtk_widget_destroyed), &window);
      gtk_container_set_border_width (GTK_CONTAINER (window), 8);

      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
      gtk_container_add (GTK_CONTAINER (window), vbox);

      label = gtk_label_new ("Nadchodzące wydarzenia:");
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

      sw = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                           GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                      GTK_POLICY_NEVER,
                                      GTK_POLICY_AUTOMATIC);
      gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

      /* create tree model */
      model = create_model ();

      /* create tree view */
      treeview = gtk_tree_view_new_with_model (model);
      gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
                                       COLUMN_DESCRIPTION);

      g_object_unref (model);

      gtk_container_add (GTK_CONTAINER (sw), treeview);

      /* add columns to the tree view */
      add_columns (GTK_TREE_VIEW (treeview));

      /* finish & show */
      gtk_window_set_default_size (GTK_WINDOW (window), 280, 250);
      g_signal_connect (window, "delete-event",
                        G_CALLBACK (window_closed), NULL);
    }

  if (!gtk_widget_get_visible (window))
    {
      gtk_widget_show_all (window);
    }
  else
    {
      gtk_widget_destroy (window);
    }

  return window;
}


int main(int argc, char *argv[]){

  gtk_init(&argc, &argv);
  do_list_store(window);
  gtk_main();

  return 0;
}
