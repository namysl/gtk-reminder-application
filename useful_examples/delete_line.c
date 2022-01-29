/**
 * C program to delete specific line from a file.
 */

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1000


int main()
{
    FILE *srcFile;
    FILE *tempFile;

    char *path = "baza_proba.txt";

    int line;

    printf("Enter line number to remove: ");
    scanf("%d", &line);


    /* Try to open file */
    srcFile  = fopen(path, "r");
    tempFile = fopen("delete-line.tmp", "w");


    /* Exit if file not opened successfully */
    if (srcFile == NULL || tempFile == NULL)
    {
        printf("Unable to open file.\n");
        printf("Please check you have read/write previleges.\n");

        exit(EXIT_FAILURE);
    }


    // Move src file pointer to beginning
    rewind(srcFile);

    // Delete given line from file.
    char buffer[BUFFER_SIZE];
    int count = 0;

    while ((fgets(buffer, BUFFER_SIZE, srcFile)) != NULL)
    {
        /* If current line is not the line user wanted to remove */
        if (line != count)
            fputs(buffer, tempFile);

        count++;
    }

    /* Close all open files */
    fclose(srcFile);
    fclose(tempFile);


    /* Delete src file and rename temp file as src */
    remove(path);
    rename("delete-line.tmp", path);

    // Open source file and print its contents
    srcFile = fopen(path, "r");
    fclose(srcFile);

    return 0;
}

