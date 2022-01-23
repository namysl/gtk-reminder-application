#include <stdio.h>
#include <glib.h>
#include <time.h>


typedef struct {

    int y;
    int M;
    int d;
    int h;
    int m;
    char description[500];
    char data[20];
    char czas[20];

}event_struct;


/*
static event_struct data[] =
{
  { "2021/08/12", "18:15", "Wydarzenie przeszłe" },
  { "2022/02/03", "20:21", "Nadchodzące" },
  { "2023/12/30", "07:40", "Kolos" },
  { "2024/10/09", "15:40", "Pietruszka" },
  { "2025/11/26", "21:37", "Łałoczek" },
  { "2026/05/16", "19:00", "Szwagier Smi Czosnek Baka Chleb" },
};
*/

event_struct record[30];


void load_from_file2(){
    FILE *filehandle;
    char lyne[550];

    char *item;
    int reccount = 0;
    int k;

    char str[50];
    char str2[50];
    filehandle = fopen("baza_proba.txt","r");

    while(fgets(lyne, 120, filehandle)) {
        printf("%s",lyne);

                item = strtok(lyne," ");
                record[reccount].y = atoi(item);
                strcat(str, item);
                strcat(str, "/");

                item = strtok(NULL," ");
                record[reccount].M = atoi(item);
                strcat(str, item);
                strcat(str, "/");

                item = strtok(NULL," ");
                record[reccount].d = atoi(item);
                strcat(str, item);

                printf("Czy działa? %s\n", str);
                strcpy(record[reccount].data, str);



                item = strtok(NULL," ");
                record[reccount].h = atoi(item);
                strcat(str2, item);
                strcat(str2, ":");

                item = strtok(NULL," ");
                record[reccount].m = atoi(item);
                strcat(str2, item);
                strcpy(record[reccount].czas, str2);



                item = strtok(NULL,"\n");
                strcpy(record[reccount].description, item);
                //record[reccount].description = item;

                reccount++;
                str[0] = '\0';
                str2[0] = '\0';
    }

    fclose(filehandle);

    printf("\nTablica:\n\n");
    for (k=0; k<reccount; k++) {
        printf("data: %d/%02d/%02d, godzina: %02d:%02d, opis: %s\n",
        record[k].y, record[k].M, record[k].d,
        record[k].h, record[k].m, record[k].description);

        printf("string:\ndata: %s, godzina: %s\n\n", record[k].data, record[k].czas);
    }
}

int main(){

    printf("testy\n");
    load_from_file2();
}
