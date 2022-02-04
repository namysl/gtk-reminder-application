#include <stdio.h>
#include <glib.h>
#include <time.h>
#include <gtk/gtk.h>
#include <libnotify/notify.h>

#define STRUCTSIZE 50
#define BUFSIZE 1000
#define FILE_DATA "data.txt"
#define FILE_DISPLAYED "displayed.txt"


FILE *filehandle;
FILE *filehandle2;
GtkWidget *treeview;
GtkWidget *entry_describe, *entry_day, *entry_month, *entry_year, *entry_hour, *entry_minutes;
int num_of_entries;  //aktualizowane przez load_from_file()


typedef struct{
    int y, M, d;
    int h, m;
    char description[100];
    char date[11];
    char time[6];
    int displayed;
}event_struct;

event_struct record[STRUCTSIZE];


enum{
    COLUMN_DATE,
    COLUMN_TIME,
    COLUMN_DESCRIPTION,
    NUM_COLUMNS
};


void notification(char *message1, char *message2, char *icon){
    //krotkie powiadomienia
	notify_init("Przypominacz");
	NotifyNotification *info = notify_notification_new(message1, message2, icon);
	notify_notification_show(info, NULL);
	g_object_unref(G_OBJECT(info));
	notify_uninit();
}


int local_datetime(char choice){
    //czas lokalny
    time_t T = time(NULL);
    struct tm tm = *localtime(&T);

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

    return result;
}


int validate_date(const char *y, const char *M, const char *d){
    //czy istnieje taki dzien w miesiacu i/lub roku
    int yy = atoi(y);
    int mm = atoi(M);
    int dd = atoi(d);

    if((dd>=1 && dd<=31) && (mm==1 || mm==3 || mm==5 || mm==7 || mm==8 || mm==10 || mm==12)){
        return 0;
    }
    else if((dd>=1 && dd<=30) && (mm==4 || mm==6 || mm==9 || mm==11)){
        return 0;
    }
    else if((dd>=1 && dd<=28) && (mm==2)){
        return 0;
    }
    else if(dd==29 && mm==2 && (yy%400==0 || (yy%4==0 && yy%100!=0))){
        return 0;
    }
    else{
        //nieprawidlowa data
        return 1;
    }
}


int delete_line(int line, char *path){
    //usuwa wybrana linie z pliku
    FILE *source_file;
    FILE *temp_file;

    source_file  = fopen(path, "r");
    temp_file = fopen("temp_baza.tmp", "w");

    if(source_file == NULL || temp_file == NULL){
        printf("Blad otwarcia pliku");
        exit(EXIT_FAILURE);
    }

    rewind(source_file);

    char buffer[BUFSIZE];
    int count = 0;

    while((fgets(buffer, BUFSIZE, source_file)) != NULL)
    {
        if (line != count){ //to nie jest poszukiwana linia
            fputs(buffer, temp_file);
        }
        count++;
    }

    fclose(source_file);
    fclose(temp_file);

    remove(path);
    rename("temp_baza.tmp", path);

    return 0;
}


int check_localdatetime(int y, int M, int d, int h, int m){
    //sprawdza podany czas wzgledem czasu lokalnego
    //zwraca 1, jesli sprawdzany czas jest starszy od lokalnego
    int value = 1;

    //przypadku y < local_datetime('y') nie da sie wprowadzic z poziomu gui
    if(y > local_datetime('y')){
       value = 0;
    }
    else if(y == local_datetime('y')){
        if (M < local_datetime('M')){
            value = 1;
        }
        else if(M > local_datetime('M')){
            value = 0;
        }
        else{  // MM == local MM
            if(d < local_datetime('d')){
                value = 1;
            }
            else if(d > local_datetime('d')){
                value = 0;
            }
            else{ //dd == local dd
                if(m < local_datetime('m')){
                    value = 1;
                }
                else{ //mm <= local mm
                    value = 0;
                }
            }
        }
    }
    return value;
}


int replace_line(int line, char *path, char *new_info){
    //zastepuje wybrana linie w pliku przez nowa
    FILE *source_file;
    FILE *temp_file;

    char buffer[BUFSIZE];
    int count = 0;

    source_file  = fopen(path, "r");
    temp_file = fopen("replace.tmp", "w");

    if (source_file == NULL || temp_file == NULL){
        printf("\nUnable to open file.\n");
        printf("Please check whether file exists and you have read/write privilege.\n");
        exit(EXIT_FAILURE);
    }

    while((fgets(buffer, BUFSIZE, source_file)) != NULL){
        if(count == line){
            fputs(new_info, temp_file);
        }
        else{
            fputs(buffer, temp_file);
        }
        count++;
    }

    fclose(source_file);
    fclose(temp_file);

    remove(path);
    rename("replace.tmp", path);

    return 0;
}


void event_alert(unsigned i, char* text){
    //okno dialogu z alertem o nadchodzacym wydarzeniu
    GtkWidget* event = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                              "%s", text);

    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(event),
                                             "%s\n\n%s, %s", record[i].description, record[i].date, record[i].time);

    int response = gtk_dialog_run(GTK_DIALOG(event));

    if(response==-5){
        record[i].displayed = 1;
        replace_line(i, "baza_clicked.txt", "1\n");  //0 -> 1
    }

    gtk_widget_destroy(event);
}

static gboolean check_events(gpointer user_data){
    //sprawdza status wydarzen
    for(int i=0; i<num_of_entries; i++){
        if(record[i].displayed != 1){
            //czas wydarzenia == czas lokalny
            if(record[i].y == local_datetime('y')
            && record[i].M == local_datetime('M')
            && record[i].d == local_datetime('d')
            && record[i].h == local_datetime('h')
            && record[i].m == local_datetime('m')){
                event_alert(i, "Nadchodzące wydarzenie:");
            }

            //wydarzenie z przeszlosci, ktore nie zostalo odczytane
            if(check_localdatetime(record[i].y, record[i].M, record[i].d, record[i].h, record[i].m) == 1){
                event_alert(i, "Nieodczytane wydarzenie:");
            }
        }
    }
    return G_SOURCE_CONTINUE;
}

int load_from_file(){
    //pobiera dane z pliku i umieszcza je w strukturze
    char line[BUFSIZE];
    int count = 0; //licznik dla wydarzen (1 linia = 1 wydarzenie)

    //temp
    char *item;
    char str_date[10];
    char str_time[5];
    char str_temp[3];

    filehandle = fopen("baza_clicked.txt", "r");
    if(filehandle){
        while(fgets(line, BUFSIZE, filehandle)){
            //clicked
            item = strtok(line, "\n");
            record[count].displayed = atoi(item);

            count++;
        }
    }
    else{
        printf("Blad otwarcia pliku");
        exit(EXIT_FAILURE);
    }
    fclose(filehandle);

    line[0] = '\0';
    count = 0;

    filehandle = fopen("baza_proba.txt","r");

    if(filehandle){
        while(fgets(line, BUFSIZE, filehandle)){
            str_date[0] = '\0';
            str_time[0] = '\0';
            str_temp[0] = '\0';

            //data YYYY/MM/HH
            item = strtok(line, " ");
            record[count].y = atoi(item);
            strcat(str_date, item);

            strcat(str_date, "/");

            item = strtok(NULL, " ");
            record[count].M = atoi(item);

            if(strlen(item)==1){
                str_temp[0] = '\0';
                sprintf(str_temp, "%02d", atoi(item));
                strcat(str_date, str_temp);
            }
            else{
                strcat(str_date, item);
            }

            strcat(str_date, "/");

            item = strtok(NULL, " ");
            record[count].d = atoi(item);

            if(strlen(item)==1){
                str_temp[0] = '\0';
                sprintf(str_temp, "%02d", atoi(item));
                strcat(str_date, str_temp);
            }
            else{
                strcat(str_date, item);
            }

            strcpy(record[count].date, str_date);

            //czas HH:MM
            item = strtok(NULL, " ");
            record[count].h = atoi(item);

            if(strlen(item)==1){
                str_temp[0] = '\0';
                sprintf(str_temp, "%02d", atoi(item));
                strcat(str_time, str_temp);
            }
            else{
                strcat(str_time, item);
            }

            strcat(str_time, ":");

            item = strtok(NULL, " ");
            record[count].m = atoi(item);

            if(strlen(item)==1){
                str_temp[0] = '\0';
                sprintf(str_temp, "%02d", atoi(item));
                strcat(str_time, str_temp);
            }
            else{
                strcat(str_time, item);
            }

            strcpy(record[count].time, str_time);

            //opis wydarzenia
            item = strtok(NULL, "\n");

            printf("STRLEN: %zu\n", strlen(item));
            printf("SIZESTRUCT: %lu\n", sizeof(record->description));

            if(strlen(item) < sizeof(record->description)){
                strcpy(record[count].description, item);
            }
            else{
                memcpy(record[count].description, item, sizeof(record->description)-1);
            }

            printf("potem: %zu\n", strlen(record[count].description));
            count++;
        }
    }
    else{
        printf("Blad otwarcia pliku");
        exit(EXIT_FAILURE);
    }


    for(int i=0; i<count; i++){
        printf("%d, %d, %d, %d, %d, %d\n", record[i].displayed, record[i].y, record[i].M, record[i].d, record[i].h, record[i].m);
        puts(record[i].date);
        puts(record[i].time);
        puts(record[i].description);
    }


    fclose(filehandle);

    num_of_entries = count;
    return count;
}


GtkTreeModel *create_model(void){
    gint i = 0;
    GtkListStore *store;
    GtkTreeIter iter;

    //list store
    store = gtk_list_store_new(NUM_COLUMNS,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING);

    int count = load_from_file();
    for (i=0; i<count; i++){ //tutaj
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           COLUMN_DATE, record[i].date,
                           COLUMN_TIME, record[i].time,
                           COLUMN_DESCRIPTION, record[i].description,
                           -1);
    }

    return GTK_TREE_MODEL(store);
}


void add_columns(GtkTreeView *treeview){
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);

    //kolumna daty
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Data",
                                                      renderer,
                                                      "text",
                                                      COLUMN_DATE,
                                                      NULL);

    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(column), 120); //120 px
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_DATE);
    gtk_tree_view_append_column(treeview, column);

    //kolumna czasu
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Czas",
                                                      renderer,
                                                      "text",
                                                      COLUMN_TIME,
                                                      NULL);

    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(column), 60);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_TIME);
    gtk_tree_view_append_column(treeview, column);

    //kolumna opisu wydarzenia
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Opis wydarzenia",
                                                      renderer,
                                                      "text",
                                                      COLUMN_DESCRIPTION,
                                                      NULL);

    gtk_tree_view_column_set_sort_column_id(column, COLUMN_DESCRIPTION);
    gtk_tree_view_append_column(treeview, column);
}


GdkPixbuf *create_pixbuf(const gchar *filename) {
    //wczytywanie obrazow
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file(filename, &error);

    if (!pixbuf){
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
    }

    return pixbuf;
}

int find_in_struct(char *name_date, char *name_time, char *name_description){
    //zwraca zwraca miejsce elementu w tablicy struktur
        int j;
        for(j=0; j<=STRUCTSIZE; j++){
            if((strcmp(record[j].description, name_description)==0)
            && (strcmp(record[j].time, name_time)==0)
            && (strcmp(record[j].date, name_date)==0)){
                break;
            }
        }
        return j;
}


void delete_entry(GtkWidget *widget, gpointer data){
    //usuwanie wybranego wydarzenia
    GtkTreeIter iter;
    GtkTreeView *treeview = (GtkTreeView*)data;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);

    char *name_date;
    char *name_time;
    char *name_description;

    if (gtk_tree_selection_get_selected(selection, NULL, &iter)){
        int i;

        gtk_tree_model_get(model, &iter, COLUMN_DATE, &name_date, -1);
        gtk_tree_model_get(model, &iter, COLUMN_TIME, &name_time, -1);
        gtk_tree_model_get(model, &iter, COLUMN_DESCRIPTION, &name_description, -1);

        GtkTreePath *path;

        path = gtk_tree_model_get_path(model, &iter);
        i = gtk_tree_path_get_indices(path)[0];
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        //znajdz w tablicy
        int line = find_in_struct(name_date, name_time, name_description);

        //czyszczenie tablicy rekordow:
        memset(record, 0, sizeof(record));

        //usun z pliku
        delete_line(line, "baza_proba.txt");
        delete_line(line, "baza_clicked.txt");

        //ponowne wypelnij tablice struktur:
        load_from_file();


        gtk_tree_path_free(path);
    }
}


void show_new_entry(GtkTreeView *treeview){
    //aktualizacja i wyswietlenie nowego elementu
    event_struct foo;
    GtkTreeIter current, iter;
    GtkTreePath *path;
    GtkTreeModel *model;
    GtkTreeViewColumn *column;

    int load_all = load_from_file();
    model = gtk_tree_view_get_model(treeview);

    gtk_list_store_insert(GTK_LIST_STORE(model), &iter, -1);

    gtk_list_store_set(GTK_LIST_STORE (model), &iter,
                       COLUMN_DATE, &record[load_all-1].date,
                       COLUMN_TIME, &record[load_all-1].time,
                       COLUMN_DESCRIPTION, &record[load_all-1].description,
                       -1);

    path = gtk_tree_model_get_path(model, &iter);
    column = gtk_tree_view_get_column(treeview, 0);
    //focus na nowym elemencie
    gtk_tree_view_set_cursor(treeview, path, column, FALSE);

    gtk_tree_path_free(path);
}


void add_new_entry(GtkWidget *widget, gint response_id, gpointer data){
    //walidacja nowego wydarzenia i dopisanie do bazy
    if(response_id==-5){
        const char *describe, *y, *M, *d, *h, *m;
        char describe_cut[100];
        int flag = 0;

        describe = gtk_entry_get_text(GTK_ENTRY(entry_describe));
        if(strlen(describe)==0){
            describe = " ";  //nienazwane wydarzenie
        }
        else if(strlen(describe) >= sizeof(record->description)){
            puts("elo");
            printf("size desc %lu\n", sizeof(record->description));
            strncpy(describe_cut, describe, sizeof(record->description));
            describe_cut[sizeof(record->description)] = 0;
            puts("po elo");
            flag = 1;
            printf("NAPIS:::: %zu\n", strlen(describe_cut));
        }

        y = gtk_entry_get_text(GTK_ENTRY(entry_year));
        M = gtk_entry_get_text(GTK_ENTRY(entry_month));
        d = gtk_entry_get_text(GTK_ENTRY(entry_day));

        h = gtk_entry_get_text(GTK_ENTRY(entry_hour));
        m = gtk_entry_get_text(GTK_ENTRY(entry_minutes));


        if((validate_date(y, M, d)==0) && (check_localdatetime(atoi(y), atoi(M), atoi(d), atoi(h), atoi(m)))==0){

            filehandle = fopen("baza_proba.txt", "a");
            filehandle2 = fopen("baza_clicked.txt", "a");

            if(filehandle == NULL || filehandle2 == NULL){
                perror("Plik nie mogl zostac otwarty");
                exit(EXIT_FAILURE);
            }
            else{
                if(flag==1){
                    fprintf(filehandle, "%s %s %s %s %s %s\n", y, M, d, h, m, describe_cut);
                }
                else{
                    fprintf(filehandle, "%s %s %s %s %s %s\n", y, M, d, h, m, describe);
                }

                fclose(filehandle);

                fprintf(filehandle2, "0\n");
                fclose(filehandle2);

                //zaktualizuj
                show_new_entry(GTK_TREE_VIEW(treeview));

                //notification("Przypominacz:", "nowe wydarzenie zostało zapisane.", "emblem-default");
            }
        }
        else{
            notification("Wydarzenie nie mogło zostać zapisane,", "ponieważ podano nieprawidłową datę lub czas.", "emblem-important");
        }
    }
}


void show_add_toolb(GtkWindow *parent, gpointer user_data){
    //dodaj w toolbarze
    GtkWidget *content_area;
    GtkWidget *dialog;
    GtkWidget *hbox;
    GtkWidget *table;

    GtkWidget *label;
    gint response;
    GtkAdjustment *adjustment;

    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Dodaj", parent,
                                         flags,
                                         ("OK"), GTK_RESPONSE_OK,
                                         "Anuluj", GTK_RESPONSE_CANCEL,
                                          NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 30);

    gtk_box_pack_start(GTK_BOX(content_area), hbox, FALSE, FALSE, 0);

    table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 20); //odleglosc od obiektow wierszami
    gtk_grid_set_column_spacing(GTK_GRID(table), 25); //kolumnami
    gtk_box_pack_start(GTK_BOX(hbox), table, TRUE, TRUE, 0);

    //wpis
    label = gtk_label_new("Wydarzenie:");
    gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
    entry_describe = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(table), entry_describe, 1, 0, 4, 1);

    //dzien
    adjustment = gtk_adjustment_new(local_datetime('d'), 1, 32, 1, 1, 1);

    label = gtk_label_new("Dzień:");
    gtk_grid_attach (GTK_GRID(table), label, 1, 1, 1, 1);
    entry_day = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach (GTK_GRID(table), entry_day, 1, 2, 1, 1);

    //miesiac
    adjustment = gtk_adjustment_new(local_datetime('M'), 1, 13, 1, 1, 1);

    label = gtk_label_new("Miesiąc:");
    gtk_grid_attach(GTK_GRID(table), label, 2, 1, 1, 1);
    entry_month = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach(GTK_GRID(table), entry_month, 2, 2, 1, 1);

    //rok
    adjustment = gtk_adjustment_new(local_datetime('y'), local_datetime('y'),
                                    local_datetime('y')+10, 1, 1, 1);

    label = gtk_label_new("Rok:");
    gtk_grid_attach(GTK_GRID(table), label, 3, 1, 1, 1);
    entry_year = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach(GTK_GRID(table), entry_year, 3, 2, 1, 1);

    //godzina
    adjustment = gtk_adjustment_new(local_datetime('h'), 0, 24, 1, 1, 1);

    label = gtk_label_new("Godzina:");
    gtk_grid_attach (GTK_GRID (table), label, 1, 3, 1, 1);
    entry_hour = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach (GTK_GRID (table), entry_hour, 1, 4, 1, 1);

    //minuty
    adjustment = gtk_adjustment_new(local_datetime('m')+1, 0, 60, 1, 1, 1);

    label = gtk_label_new("Minuty:");
    gtk_grid_attach(GTK_GRID(table), label, 2, 3, 1, 1);
    entry_minutes = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach(GTK_GRID(table), entry_minutes, 2, 4, 1, 1);


    g_signal_connect_swapped(dialog, "response",
                             G_CALLBACK(add_new_entry), dialog);

    gtk_widget_show_all(hbox);
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
}


void show_edit_toolb(GtkWindow *parent, gchar *message){
    //edytuj w toolbarze
    GtkWidget *dialog, *label, *content_area;
    GtkDialogFlags flags;

    flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    dialog = gtk_dialog_new_with_buttons("Message", parent, flags, ("OK"),
                                         GTK_RESPONSE_NONE, NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new(message);


    g_signal_connect_swapped(dialog, "response",
                             G_CALLBACK(gtk_widget_destroy), dialog);

    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show_all(dialog);
}


void show_options_toolb(GtkWindow *parent, char *message){
    //opcje w toolbarze, nieskonczone
	notification("Opcje:", "jeszcze nie zaimplementowano", "face-worried-symbolic");
}


void show_info_toolb(GtkWidget *widget, gpointer data){
    //info w toolbarze
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

    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *label;
    GtkTreeModel *model;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    //tytul na pasku okna
    gtk_window_set_title(GTK_WINDOW(window), "Przypominacz");
    //rozmiar i umiejscowienie okna programu
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
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
    add_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name("list-add-symbolic",
                                    GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), add_toolb, -1);
    gtk_tool_item_set_tooltip_markup(add_toolb, "Dodaj");
    //edytuj
    edit_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name("edit-copy-symbolic",
                                     GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), edit_toolb, -1);
    gtk_tool_item_set_tooltip_markup(edit_toolb, "Edytuj");
    //usuń
    delete_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name("edit-delete-symbolic",
                                       GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), delete_toolb, -1);
    gtk_tool_item_set_tooltip_markup(delete_toolb, "Usuń");
    //separator1
    separator1 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator1, -1);
    //opcje
    options_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name("content-loading-symbolic",
                                        GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), options_toolb, -1);
    gtk_tool_item_set_tooltip_markup(options_toolb, "Opcje");
    //separator2
    separator2 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator2, -1);
    //info
    info_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name("dialog-information-symbolic",
                                     GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), info_toolb, -1);
    gtk_tool_item_set_tooltip_markup(info_toolb, "Info");
    //wyjście
    exit_toolb = gtk_tool_button_new(gtk_image_new_from_icon_name("application-exit-symbolic",
                                     GTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit_toolb, -1);
    gtk_tool_item_set_tooltip_markup(exit_toolb, "Wyjdź");


    gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, FALSE, 5);

    //lista
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);


    //tree model
    model = create_model();

    //tree view
    treeview = gtk_tree_view_new_with_model(model);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW (treeview), COLUMN_DESCRIPTION);

    g_object_unref(model);

    gtk_container_add(GTK_CONTAINER(sw), treeview);

    //kolumny w tree view
    add_columns(GTK_TREE_VIEW(treeview));

    //co 5 sekund sprawdza czy zapisane wydarzenie == czas lokalny
    g_timeout_add(5000, check_events, NULL);

    //sygnały
    g_signal_connect(G_OBJECT(add_toolb), "clicked",
                     G_CALLBACK(show_add_toolb), NULL);

    g_signal_connect(G_OBJECT(edit_toolb), "clicked",
                     G_CALLBACK(show_edit_toolb), NULL);

    g_signal_connect(G_OBJECT(delete_toolb), "clicked",
                     G_CALLBACK(delete_entry), treeview);

    g_signal_connect(G_OBJECT(options_toolb), "clicked",
                     G_CALLBACK(show_options_toolb), NULL);

    g_signal_connect(G_OBJECT(info_toolb), "clicked",
                     G_CALLBACK(show_info_toolb), NULL);

    g_signal_connect(G_OBJECT(exit_toolb), "clicked",
                     G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);


    gtk_widget_show_all(window);
    g_object_unref(icon);
    gtk_main();

    return 0;
}
