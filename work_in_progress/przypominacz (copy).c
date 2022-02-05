#include <stdio.h>
#include <glib.h>
#include <time.h>
#include <gtk/gtk.h>
#include <libnotify/notify.h>

#define ARRAYSIZE 50
#define BUFSIZE 2000
#define FILE_DATA "data.txt"
#define FILE_DISPLAYED "displayed.txt"


GtkWidget *window, *treeview;
GtkWidget *entry_describe, *entry_day, *entry_month, *entry_year, *entry_hour, *entry_minutes;
unsigned num_of_entries;  //aktualizowane przez load_from_file()


typedef struct{
    int y, M, d;
    int h, m;
    char description[100];
    char date[11];
    char time[6];
    int displayed;
}event_struct;

event_struct record[ARRAYSIZE];


enum{
    COLUMN_DATE,
    COLUMN_TIME,
    COLUMN_DESCRIPTION,
    NUM_COLUMNS
};


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


int check_localdatetime(int y, int M, int d, int h, int m){
    //sprawdza podany czas wzgledem czasu lokalnego
    //zwraca 0, jesli sprawdzany czas == localdatetime
    //zwraca 1, jesli sprawdzany czas jest z przeszlosci
    //zwraca 2, jesli sprawdzany czas jest z przyszlosci
    int local_y = local_datetime('y');
    int local_M = local_datetime('M');
    int local_d = local_datetime('d');
    int local_h = local_datetime('h');
    int local_m = local_datetime('m');

    if(y < local_y){
       return 1;
    }
    else if(y > local_y){
       return 2;
    }
    else{  //y == local_y
        if(M<local_M){
            return 1;
        }
        else if(M>local_M){
            return 2;
        }
        else if(d<local_d){
            return 1;
        }
        else if(d>local_d){
            return 2;
        }
        else if(h<local_h){
            return 1;
        }
        else if(h>local_h){
            return 2;
        }
        else if(m<local_m){
            return 1;
        }
        else if(m>local_m){
            return 2;
        }
        else{  //m == local_m
            return 0;
        }
    }
}


int delete_line_in_file(int line, char *path){
    //usuwa wybrana linie z pliku
    FILE *source_file;
    FILE *temp_file;

    source_file  = fopen(path, "r");
    temp_file = fopen("temp_dlt.tmp", "w");

    if(source_file == NULL || temp_file == NULL){
        printf("Blad otwarcia pliku\n");
        exit(EXIT_FAILURE);
    }

    rewind(source_file);

    char buffer[BUFSIZE];
    unsigned count = 0;

    while((fgets(buffer, BUFSIZE, source_file)) != NULL)
    {
        if (line != count){  //to nie jest poszukiwana linia
            fputs(buffer, temp_file);
        }
        count++;
    }

    fclose(source_file);
    fclose(temp_file);

    remove(path);
    rename("temp_dlt.tmp", path);

    return 0;
}


int replace_line_in_file(int line, char *path, char *new_info){
    //zastepuje wybrana linie w pliku przez nowa
    FILE *source_file;
    FILE *temp_file;

    char buffer[BUFSIZE];
    unsigned count = 0;

    source_file  = fopen(path, "r");
    temp_file = fopen("temp_rplc.tmp", "w");

    if (source_file == NULL || temp_file == NULL){
        printf("Blad otwarcia pliku\n");
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
    rename("temp_rplc.tmp", path);

    return 0;
}


int load_from_file(){
    //pobiera dane z pliku i umieszcza je w strukturze
    char line[BUFSIZE];
    int count = 0;  //licznik dla wydarzen (1 linia = 1 wydarzenie)

    //temp
    char *item;
    char str_date[10];
    char str_time[5];
    char str_temp[3];

    FILE *filehandle;
    filehandle = fopen(FILE_DISPLAYED, "r");

    if(filehandle){
        while(fgets(line, BUFSIZE, filehandle)){
            //displayed
            item = strtok(line, "\n");
            record[count].displayed = atoi(item);

            count++;
        }
    }
    else{
        printf("Blad otwarcia pliku\n");
        exit(EXIT_FAILURE);
    }
    fclose(filehandle);

    line[0] = '\0';
    count = 0;

    filehandle = fopen(FILE_DATA,"r");

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

            if(strlen(item) < sizeof(record->description)){
                strcpy(record[count].description, item);
            }
            else{
                memcpy(record[count].description, item, sizeof(record->description)-1);
            }
            count++;
        }
    }
    else{
        printf("Blad otwarcia pliku\n");
        exit(EXIT_FAILURE);
    }

/*
    for(int i=0; i<count; i++){
        printf("displayed: %d, h: %d, M: %d, d: %d, h: %d, m: %d\n",
               record[i].displayed, record[i].y, record[i].M, record[i].d, record[i].h, record[i].m);
        puts(record[i].date);
        puts(record[i].time);
        puts(record[i].description);
        puts("~~~");
    }
*/

    fclose(filehandle);

    num_of_entries = count;
    return count;
}


void notification(char *message1, char *message2, char *icon){
    //krotkie powiadomienia
	notify_init("Przypominacz");
	NotifyNotification *info = notify_notification_new(message1, message2, icon);
	notify_notification_show(info, NULL);
	g_object_unref(G_OBJECT(info));
	notify_uninit();
}


void event_alert(unsigned i, char *text){
    //okno dialogu z alertem o nadchodzacym wydarzeniu
    GtkWidget *event = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                              "%s", text);

    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(event),
                                             "%s\n\n%s, %s", record[i].description, record[i].date, record[i].time);

    int response = gtk_dialog_run(GTK_DIALOG(event));

    if(response==-5){
        record[i].displayed = 1;
        replace_line_in_file(i, FILE_DISPLAYED, "1\n");  //0 -> 1
    }
    gtk_widget_destroy(event);
}


gboolean check_events(gpointer data){
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
            else if(check_localdatetime(record[i].y, record[i].M, record[i].d, record[i].h, record[i].m) == 1){
                event_alert(i, "Nieodczytane wydarzenie:");
            }
        }
    }
    return G_SOURCE_CONTINUE;
}


int find_in_struct(char *name_date, char *name_time, char *name_description){
    //zwraca miejsce elementu w tablicy struktur
    int i;
    for(i=0; i<=ARRAYSIZE; i++){
        if((strcmp(record[i].description, name_description)==0)
        && (strcmp(record[i].time, name_time)==0)
        && (strcmp(record[i].date, name_date)==0)){
            break;
        }
    }
    return i;
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

    //pobierz
    load_from_file();

    //wypelnij
    for(i=0; i<num_of_entries; i++){
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
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        //znajdz w tablicy
        int line = find_in_struct(name_date, name_time, name_description);

        //czyszczenie tablicy rekordow:
        memset(record, 0, sizeof(record));

        //usun z pliku
        delete_line_in_file(line, FILE_DATA);
        delete_line_in_file(line, FILE_DISPLAYED);

        //ponowne wypelnij tablice struktur:
        load_from_file();

        gtk_tree_path_free(path);
    }
}


void show_new_entry(GtkTreeView *treeview){
    //aktualizacja list store i wyswietlenie nowego elementu
    event_struct foo;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeModel *model;
    GtkTreeViewColumn *column;

    //zaktualizuj
    load_from_file();

    model = gtk_tree_view_get_model(treeview);

    gtk_list_store_insert(GTK_LIST_STORE(model), &iter, -1);

    gtk_list_store_set(GTK_LIST_STORE (model), &iter,
                       COLUMN_DATE, &record[num_of_entries-1].date,
                       COLUMN_TIME, &record[num_of_entries-1].time,
                       COLUMN_DESCRIPTION, &record[num_of_entries-1].description,
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
        unsigned flag = 0;

        describe = gtk_entry_get_text(GTK_ENTRY(entry_describe));
        if(strlen(describe)==0){
            describe = " ";  //nienazwane wydarzenie
        }
        else if(strlen(describe) >= sizeof(record->description)){
            strncpy(describe_cut, describe, sizeof(record->description));
            describe_cut[sizeof(record->description)] = 0;
            flag = 1;
        }

        y = gtk_entry_get_text(GTK_ENTRY(entry_year));
        M = gtk_entry_get_text(GTK_ENTRY(entry_month));
        d = gtk_entry_get_text(GTK_ENTRY(entry_day));

        h = gtk_entry_get_text(GTK_ENTRY(entry_hour));
        m = gtk_entry_get_text(GTK_ENTRY(entry_minutes));

        FILE *filehandle1;
        FILE *filehandle2;

        if((validate_date(y, M, d)==0) && (check_localdatetime(atoi(y), atoi(M), atoi(d), atoi(h), atoi(m)))==2){

            filehandle1 = fopen(FILE_DATA, "a");
            filehandle2 = fopen(FILE_DISPLAYED, "a");

            if(filehandle1 == NULL || filehandle2 == NULL){
                perror("Plik nie mogl zostac otwarty");
                exit(EXIT_FAILURE);
            }
            else{
                if(flag==1){
                    fprintf(filehandle1, "%s %s %s %s %s %s\n", y, M, d, h, m, describe_cut);
                }
                else{
                    fprintf(filehandle1, "%s %s %s %s %s %s\n", y, M, d, h, m, describe);
                }

                fclose(filehandle1);

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


void show_add_toolb(GtkWindow *parent, gpointer data){
    //dodaj w toolbarze
    parent = GTK_WINDOW(window);

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

    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
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
                                    local_datetime('y')+11, 1, 1, 1);

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


void show_edited_entry(GtkTreeView *treeview, int num){
    //aktualizacja i wyswietlenie edytowanego elementu
    event_struct foo;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeModel *model;
    GtkTreeViewColumn *column;

    //zaktualizuj
    load_from_file();

    model = gtk_tree_view_get_model(treeview);

    if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model), &iter, NULL, num))
     {
       gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
     }

    gtk_list_store_insert(GTK_LIST_STORE(model), &iter, num);

    gtk_list_store_set(GTK_LIST_STORE (model), &iter,
                       COLUMN_DATE, &record[num].date,
                       COLUMN_TIME, &record[num].time,
                       COLUMN_DESCRIPTION, &record[num].description,
                       -1);

    path = gtk_tree_model_get_path(model, &iter);

    column = gtk_tree_view_get_column(treeview, 0);
    //focus na zmodyfikowanym elemencie
    gtk_tree_view_set_cursor(treeview, path, column, FALSE);

    gtk_tree_path_free(path);
}


void edit_entry(GtkWidget *widget, gint response_id, gpointer data){
    //walidacja daty i edycja bazy, aktualizacja widoku
    GtkTreeIter iter;

    if(response_id==-5){
        const char *describe, *y, *M, *d, *h, *m;
        char describe_cut[100];
        char temp[200];
        unsigned flag = 0;
        int *num = data;

        describe = gtk_entry_get_text(GTK_ENTRY(entry_describe));
        if(strlen(describe)==0){
            describe = record[*num].description;  //nazwa wydarzenia pozostala niezmieniona
        }
        else if(strlen(describe) >= sizeof(record->description)){
            strncpy(describe_cut, describe, sizeof(record->description));
            describe_cut[sizeof(record->description)] = 0;
            flag = 1;
        }

        y = gtk_entry_get_text(GTK_ENTRY(entry_year));
        M = gtk_entry_get_text(GTK_ENTRY(entry_month));
        d = gtk_entry_get_text(GTK_ENTRY(entry_day));

        h = gtk_entry_get_text(GTK_ENTRY(entry_hour));
        m = gtk_entry_get_text(GTK_ENTRY(entry_minutes));

        if((validate_date(y, M, d)==0) && (check_localdatetime(atoi(y), atoi(M), atoi(d), atoi(h), atoi(m)))==2){
            if(flag==1){
                snprintf(temp, sizeof(temp), "%s %s %s %s %s %s\n", y, M, d, h, m, describe_cut);
            }
            else{
                snprintf(temp, sizeof(temp), "%s %s %s %s %s %s\n", y, M, d, h, m, describe);
            }

            //wyczysc
            memset(record, 0, sizeof(record));

            //zamien
            replace_line_in_file(*num, FILE_DATA, temp);
            replace_line_in_file(*num, FILE_DISPLAYED, "0\n");

            //wypelnij i pokaz
            show_edited_entry(GTK_TREE_VIEW(treeview), *num);

            //notification("Przypominacz:", "wydarzenie zostało zmodyfikowane.", "emblem-default");
        }

        else{
            notification("Wydarzenie nie mogło zostać zmodyfikowane,", "ponieważ podano nieprawidłową datę lub czas.", "emblem-important");
        }
    }
}


void show_edit_toolb(GtkWindow *parent, int line, gpointer data){
    //edytuj w toolbarze
    parent = GTK_WINDOW(window);

    GtkWidget *content_area;
    GtkWidget *dialog;
    GtkWidget *hbox;
    GtkWidget *table;

    GtkWidget *label;
    gint response;
    GtkAdjustment *adjustment;

    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_dialog_new_with_buttons("Edytuj", parent,
                                         flags,
                                         ("OK"), GTK_RESPONSE_OK,
                                         "Anuluj", GTK_RESPONSE_CANCEL,
                                          NULL);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
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
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_describe), record[line].description);

    //dzien
    adjustment = gtk_adjustment_new(record[line].d, 1, 32, 1, 1, 1);

    label = gtk_label_new("Dzień:");
    gtk_grid_attach (GTK_GRID(table), label, 1, 1, 1, 1);
    entry_day = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach (GTK_GRID(table), entry_day, 1, 2, 1, 1);

    //miesiac
    adjustment = gtk_adjustment_new(record[line].M, 1, 13, 1, 1, 1);

    label = gtk_label_new("Miesiąc:");
    gtk_grid_attach(GTK_GRID(table), label, 2, 1, 1, 1);
    entry_month = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach(GTK_GRID(table), entry_month, 2, 2, 1, 1);

    //rok
    adjustment = gtk_adjustment_new(record[line].y, local_datetime('y'),
                                    local_datetime('y')+11, 1, 1, 1);

    label = gtk_label_new("Rok:");
    gtk_grid_attach(GTK_GRID(table), label, 3, 1, 1, 1);
    entry_year = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach(GTK_GRID(table), entry_year, 3, 2, 1, 1);

    //godzina
    adjustment = gtk_adjustment_new(record[line].h, 0, 24, 1, 1, 1);

    label = gtk_label_new("Godzina:");
    gtk_grid_attach (GTK_GRID (table), label, 1, 3, 1, 1);
    entry_hour = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach (GTK_GRID (table), entry_hour, 1, 4, 1, 1);

    //minuty
    adjustment = gtk_adjustment_new(record[line].m, 0, 60, 1, 1, 1);

    label = gtk_label_new("Minuty:");
    gtk_grid_attach(GTK_GRID(table), label, 2, 3, 1, 1);
    entry_minutes = gtk_spin_button_new(adjustment, 1.0, 0);
    gtk_grid_attach(GTK_GRID(table), entry_minutes, 2, 4, 1, 1);

    gtk_widget_grab_focus(entry_day);

    g_signal_connect(dialog, "response",
                     G_CALLBACK(edit_entry), &line);

    gtk_widget_show_all(hbox);
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
}


void show_unedited_entry(GtkWidget *widget, gpointer data){
    GtkTreeIter iter;
    GtkTreeView *treeview = (GtkTreeView*)data;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);

    char *name_date;
    char *name_time;
    char *name_description;

    if (gtk_tree_selection_get_selected(selection, NULL, &iter)){

        gtk_tree_model_get(model, &iter, COLUMN_DATE, &name_date, -1);
        gtk_tree_model_get(model, &iter, COLUMN_TIME, &name_time, -1);
        gtk_tree_model_get(model, &iter, COLUMN_DESCRIPTION, &name_description, -1);

        GtkTreePath *path;
        path = gtk_tree_model_get_path(model, &iter);
        int line = find_in_struct(name_date, name_time, name_description);

        show_edit_toolb(GTK_WINDOW(window), line, NULL);

        gtk_tree_path_free(path);
    }
}


void show_options_toolb(GtkWindow *parent, char *message){
    //opcje w toolbarze
	notification("Opcje:", "jeszcze nie zaimplementowano", "face-worried-symbolic");
}


void show_info_toolb(GtkWidget *widget, gpointer data){
    //info w toolbarze
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("icon.jpg", NULL);
    GtkWidget *dialog = gtk_about_dialog_new();

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Przypominacz");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "v1.0");
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

    GdkPixbuf *icon;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    //tytul na pasku okna
    gtk_window_set_title(GTK_WINDOW(window), "Przypominacz");
    //rozmiar i umiejscowienie okna programu
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    //ikona programu
    icon = create_pixbuf("icon.jpg");
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
                     G_CALLBACK(show_unedited_entry), treeview);

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
