/**
 * C program to delete specific line from a file.
 */

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1000


/**
 * Function to delete a given line from file.
 */
void deleteLine(FILE *srcFile, FILE *tempFile, const int line)
{
    char buffer[BUFFER_SIZE];
    int count = 0;

    while ((fgets(buffer, BUFFER_SIZE, srcFile)) != NULL)
    {
        /* If current line is not the line user wanted to remove */
        if (line != count)
            fputs(buffer, tempFile);

        count++;
    }
}


int main()
{
    FILE *srcFile;
    FILE *tempFile;

    char path[100];

    int line;

    /* Input file path and line number */
    printf("Enter file path: ");
    scanf("%s", path);

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
    deleteLine(srcFile, tempFile, line);


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

