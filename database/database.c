#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <wait.h>

//mendefinisikan port server
#define PORT 8000
#define SO_REUSEPORT 15

typedef struct column {
    char name[1024];
    char type[8];
    char contents[100][1024];
} column;

int table_row, table_column;
char database[1024] = "";
char username[1024] = "";
char password[1024] = "";
char buffer[1024] = "";
char *server_path = "/home/kali/FP/database/databases";
char query_result[1024] = "";

void logging(const char *command) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char error[100] = "";
    bzero(error, sizeof(error));
    FILE *fptr;
    char msglog[100];
    fptr = fopen("db.log", "a");
    if(fptr == NULL)
    {
        strcpy(error, strerror(errno));
                // return;
    }
    sprintf(msglog, "%4d-%2d-%2d %2d:%2d:%2d:", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    strcat(msglog, username);
    strcat(msglog, ":");
    strcat(msglog, command);
    fprintf(fptr, "%s\n", msglog);

    fclose(fptr);
}

void count_column(char *db, char *table) {
    char file[1024], buf[1024];
    strcpy(file, db);
    strcat(file, "/");
    strcat(file, table);
    // sprintf(file, "%s/%s", db, table);

    FILE *fptr;
    fptr = fopen(file, "r");
    if(fptr == NULL)
    {
        perror("error : ");
    }

    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    rewind(fptr);

    fread(buf, 1, fsize, fptr);

    int col = 0;
    for (char *itr = buf; *itr != '\n'; itr++) {
        if (*itr == '\t') ++col;
    }

    table_column = col + 1;
    
    fclose(fptr);
}

char *getLine(column *cols, int row, int col) {
    char *buff;
    buff = (char*)malloc(sizeof(char)*1024);
    // memset(buff, 0, sizeof(buff));
    bzero(buff, sizeof(buff));
    char temp[1024];
    if (row == 0) {
        for (int c = 0; c < col; ++c) {
            bzero(temp, sizeof(temp));
            if (c < col - 1) {
                bzero(temp, sizeof(temp));
                sprintf(buff, "%s%s|%s\t", buff, cols[c].name, cols[c].type);
            } else {
                bzero(temp, sizeof(temp));
                sprintf(buff, "%s%s|%s", buff, cols[c].name, cols[c].type);
            }
        }
        return buff;
    } else {
        for (int k = 0; k < table_row; ++k) {
            if (k == row) {
                bzero(temp, sizeof(temp));
                for (int c = 0; c < col; ++c) {
                    bzero(temp, sizeof(temp));
                    if (c < col - 1) {
                        bzero(temp, sizeof(temp));
                        sprintf(buff, "%s%s\t", buff, cols[c].contents[k]);
                    } else {
                        sprintf(buff, "%s%s", buff, cols[c].contents[k]);
                    }
                }
            }
        }
    }

    return buff;
}

char *getPartialLine(column *cols, int argc, char *argv[], int row, int col) {
    char *buff = (char*)malloc(sizeof(char)*1024);
    // memset(buff, 0, strlen(buff));
    bzero(buff, sizeof(buff));
    char temp[1024];
    if (row == 0) {
        bzero(temp, sizeof(temp));
        for (int c = 0; c < col; ++c) {
            bzero(temp, sizeof(temp));
            for (int i = 0; i < argc; i++) {
                bzero(temp, sizeof(temp));
                if (strcmp(cols[c].name, argv[i]) == 0) {
                    bzero(temp, sizeof(temp));
                    if (c < col - 1) {
                        bzero(temp, sizeof(temp));
                        sprintf(buff, "%s%s|%s\t", buff, cols[c].name, cols[c].type);
                    } else {
                        bzero(temp, sizeof(temp));
                        sprintf(buff, "%s%s|%s", buff, cols[c].name, cols[c].type);
                    }
                }
            }
        }
    } else {
        for (int k = 0; k < table_row; ++k) {
            bzero(temp, sizeof(temp));
            if (k == row) {
                for (int c = 0; c < col; ++c) {
                    bzero(temp, sizeof(temp));
                    for (int i = 0; i < argc; i++) {
                        bzero(temp, sizeof(temp));
                        if (strcmp(cols[c].name, argv[i]) == 0) {
                            if (c < col - 1) {
                                bzero(temp, sizeof(temp));
                                sprintf(buff, "%s%s\t", buff, cols[c].contents[k]);
                            } else {
                                bzero(temp, sizeof(temp));
                                sprintf(buff, "%s%s", buff, cols[c].contents[k]);
                            }
                        }
                    }
                }
            }
        }
    }

    return buff;
}

column* parse_db(char *db, char *table) {
    char file[1024];
    sprintf(file, "%s/%s", db, table);

    FILE *fptr;
    fptr = fopen(file, "r");
    if(fptr == NULL)
    {
        perror("error : ");
    }

    char buf[4096];
    bzero(buf, sizeof(buf));
    // memset(buf, 0, sizeof(buf));
    char temp[1024];
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    rewind(fptr);

    fread(buf, 1, fsize, fptr);

    char *token;
    token = strchr(buf, '\n');

    int col = 0, i, row = 0;
    char line[1024];
    bzero(line, sizeof(line));
    strcpy(line, "");
    bzero(line, sizeof(line));
    sprintf(line, "%.*s", token - buf, buf);

    count_column(db, table);
    bzero(temp, sizeof(temp));
    column *cols;
    cols = (column*)malloc(sizeof(column) * table_column);
    
    token = strtok(buf, "\n");
    bzero(temp, sizeof(temp));
    while (token != NULL) {
        i = 0;
        char *mline, value[1024];
        mline = (char*)malloc(sizeof(char)*1024);
        sprintf(mline, "%s", token);
        memset(value, 0, sizeof(value));

        char *delim, first[1024];
        for (char *itr = mline; *itr != '\0'; itr++) {
            if (*itr == '\t') {
                bzero(temp, sizeof(temp));
                if (row == 0) {
                    strcpy(first, value);
                    // sprintf(first, "%s", value);

                    delim = strchr(first, '|');
                    sprintf(cols[i].name, "%.*s", delim - first, first);
                    sprintf(cols[i].type, "%s", delim+1);
                } else {
                    strcpy(cols[i].contents[row], value);
                }

                i++;
                bzero(value, sizeof(value));
                // memset(value, 0, sizeof(value));
            } else {
                strncat(value, itr, 1);
            }
        }

        if (row == 0) {
            bzero(temp, sizeof(temp));
            sprintf(first, "%s", value);                    
            delim = strchr(first, '|');
            bzero(temp, sizeof(temp));
            sprintf(cols[i].name, "%.*s", delim - first, first);
            sprintf(cols[i].type, "%s", delim+1);
        } else {
            strcat(cols[i].contents[row], value);
        }

        ++row;
        token = strtok(NULL, "\n");
    }
    table_row = row;

    fclose(fptr);

    return cols;
}


char* db_select(char *db, char *table, int argc, char *argv[], char *left, char *right) {
    char file[2048];
    sprintf(file, "%s/%s", db, table);

    FILE *fptr;
    fptr = fopen(file, "r");

    if(fptr == NULL)
    {
        perror("error : ");
    }

    char temp[1024];

    char buf[1024] = "";

    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    rewind(fptr);
    bzero(temp, sizeof(temp));
    fread(buf, 1, fsize, fptr);

    char *token;
    token = strchr(buf, '\n');

    int col = 0;
    char line[1024];
    bzero(line, sizeof(line));

    sprintf(line, "%.*s", token - buf, buf);

    char *tab;
    tab = strtok(line, "\t");
    bzero(temp, sizeof(temp));
    while (tab != NULL) {
        ++col;
        tab = strtok(NULL, "\t");
    }
    table_column = col;

    int i;
    i = 0;
    column *cols;
    cols = parse_db(db, table);

    char *buff;
    buff = (char*)malloc(sizeof(char)*1024);
    bzero(buff, sizeof(buff));
    // memset(buff, 0, strlen(buff));

    if (strlen(left) && strlen(right)) {
        if (argc == 1 && strcmp(argv[0], "*") == 0) {
            bzero(temp, sizeof(temp));
            sprintf(buff, "%s\n", getLine(cols, 0, col));
            for (int k = 1; k < table_row; ++k) {
                for (int c = 0; c < col; ++c) {
                    bzero(temp, sizeof(temp));
                    if (strcmp(cols[c].name, left) == 0 && strcmp(cols[c].contents[k], right) == 0) {
                        sprintf(buff, "%s%s\n", buff, getLine(cols, k, col));
                    }
                }
            }
        } else {
            sprintf(buff, "%s\n", getPartialLine(cols, argc, argv, 0, col));
            for (int k = 1; k < table_row; ++k) {
                for (int c = 0; c < col; ++c) {
                    if (strcmp(cols[c].name, left) == 0 && strcmp(cols[c].contents[k], right) == 0) {
                        sprintf(buff, "%s%s\n", buff, getPartialLine(cols, argc, argv, k, col));
                    }
                }
            }
        }
    } else if (argc == 1 && strcmp(argv[0], "*") == 0) {
        bzero(temp, sizeof(temp));
        sprintf(buff, "%s\n", getLine(cols, 0, col));
        sprintf(buff, "%s", buf);
    } else {    
        sprintf(buff, "%s\n", getPartialLine(cols, argc, argv, 0, col));
        for (int k = 1; k < table_row; ++k) {
            bzero(temp, sizeof(temp));
            sprintf(buff, "%s%s\n", buff, getPartialLine(cols, argc, argv, k, col));
        }
    }

    fclose(fptr);
    return buff;
}


void db_execute(char *command) {
    if (*(command + strlen(command) - 1) != ';') {
        strcpy(query_result, "Missing ;");
        return;
    }

    logging((const char *)command);
    
    if (strncmp(command, "CREATE USER", 11) == 0) {
        if (strcmp(username, "root")) {
            sprintf(query_result, "You can't do that!");
            return;
        }

        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd, command+11);
        
        char *token = strtok(cmd, " ");
        char user[1024];
        bzero(user, sizeof(user));
        strcpy(user, token);

        token = strtok(NULL, " ");
        token = strtok(NULL, " ");
        token = strtok(NULL, " ;");
        char pwd[1024];
        bzero(pwd, sizeof(pwd));
        strcpy(pwd, token);

        FILE *fptr;
        fptr = fopen("root/users", "a");
        if(fptr == NULL)
        {
            perror("error : ");
        }

        fprintf(fptr, "%s\t%s\n", user, pwd);
        fclose(fptr);
    } else if (strncmp(command, "USE", 3) == 0) {
        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd, command);

        char *token = strtok(cmd, " ");
        token = strtok(NULL, " ;");

        char *buff;
        buff = (char*)malloc(sizeof(char)*1024);

        char *argv[] = {"database"};
        char *res = db_select("root", "db_permission", 1, argv, "username", username);
        sprintf(buff, "%s", res);

        DIR *dir;
        dir = opendir(token);

        if (!dir) {
            strcpy(query_result, "invalid database ");
            strcat(query_result, token);
            // sprintf(query_result, "Invalid db %s!", token);
            return;
        }

        closedir(dir);
        
        char *line = strtok(buff, "\n");
        line = strtok(NULL, "\n");
        char temp[1024];
        int found = 0;
        
        while (line != NULL) {
            if (strcmp(token, line) == 0) {
                bzero(temp, sizeof(temp));
                found = 1;
                break;
            }

            line = strtok(NULL, "\n");
        }
        
        if (found == 1) {
            bzero(temp, sizeof(temp));
            strcpy(database, token);
            // sprintf(database, "%s", token);
        } else {
            bzero(temp, sizeof(temp));
            strcpy(query_result, "permission error for database ");
            strcat(query_result, token);
            // sprintf(query_result, "You can't open %s", token);
        }
    } else if (strncmp(command, "GRANT PERMISSION", 16) == 0) {
        if (strncmp(username, "root", 4)) {
            strcpy(query_result, "permission error ");
            // sprintf(query_result, "You can't do that!");
            return;
        }

        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd, command + 16);
        // sprintf(cmd, "%s", command + 16);

        char *token = strtok(cmd, " ");
        
        char db[1024];
        bzero(db, sizeof(db));
        strcpy(db, token);
        // sprintf(db, "%s", token);

        token = strtok(NULL, " ");
        token = strtok(NULL, " ;");

        char user[1024];
        bzero(user, sizeof(user));
        strcpy(user, token);
        // sprintf(user, "%s", token);

        FILE *fptr;
        fptr = fopen("root/db_permission", "a");
        if(fptr == NULL)
        {
            perror("error : ");
        }
        fprintf(fptr, "%s\t%s\n", user, db);
        fclose(fptr);
    } else if (strncmp(command, "CREATE DATABASE", 15) == 0) {
        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy  (cmd, command + 15);

        char *db = strtok(cmd, " ;");

        char buat_directory[1024];
        bzero(buat_directory, sizeof(buat_directory));
        strcpy(buat_directory, "mkdir -p ");
        strcat(buat_directory, db);
        // sprintf(buat_directory, "mkdir -p %s", db);
        system(buat_directory);

        FILE *fptr;
        fptr = fopen("root/db_permission", "a");
        if(fptr == NULL)
        {
            perror(" error : ");
        }
        fprintf(fptr, "%s\t%s\n", username, db);
        fprintf(fptr, "root\t%s\n", db);
        fclose(fptr);
    } else if (strncmp(command, "CREATE TABLE", 12) == 0) {
        if (strlen(database) == 0) {
            strcpy(query_result, "Please use database first!");
            return;
        }

        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd, command + 12);

        char *token = strtok(cmd, " ");
        
        char table[1024];
        bzero(table, sizeof(table));
        strcpy(table, token);

        column columns[10];
        char temp[1024];
        char ct[1024];
        bzero(ct, sizeof(ct));
        // sprintf(cmd, "%s", command);
        strcpy(cmd, command);
        char *ob = strchr(cmd, '(');
        char *cb = strchr(cmd, ')');
        sprintf(ct, "%.*s", cb - ob - 1, ob + 1);

        int j = 0;
        token = strtok(ct, ", ");
        while (token != NULL) {
            bzero(temp, sizeof(temp));
            sprintf(columns[j].name, "%s", token);
            token = strtok(NULL, ", ");

            sprintf(columns[j].type, "%s", token);
            token = strtok(NULL, ", ");

            ++j;
        }

        char file[1024];
        bzero(file, sizeof(file));
        strcpy(file, database);
        strcat(file, "/");
        strcat(file, table);
        // sprintf(file, "%s/%s", database, table);

        FILE *fptr;
        fptr = fopen(file, "w");
        if(fptr == NULL)
        {
            sprintf(query_result, "error : %s", strerror(errno));
            return;
        }

        for (int i = 0; i < j; i++) {
            bzero(temp, sizeof(temp));
            if (i < j - 1) {
                bzero(temp, sizeof(temp));
                fprintf(fptr, "%s|%s\t", columns[i].name, columns[i].type);
            } else {
                bzero(temp, sizeof(temp));
                fprintf(fptr, "%s|%s\n", columns[i].name, columns[i].type);
            }
        }

        fclose(fptr);
    } else if (strncmp(command, "DROP DATABASE", 13) == 0) {
        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd, command + 13);
        // sprintf(cmd, "%s", command + 13);

        char *db;
        db = strtok(cmd, " ;\0");

        char dbs[1024];
        bzero(dbs, sizeof(dbs));
        char *argv[] = {"database"};
        char temp[1024];
        bzero(temp, sizeof(temp));
        char *res = db_select("root", "db_permission", 1, argv, "username", username);
        strcpy(dbs, res);
        // sprintf(dbs, "%s", res);

        char *token;
        token = strtok(dbs, "\n");
        token = strtok(NULL, "\n");
        while (token != NULL) {
            if (strcmp(token, db) == 0) {
                char del[1024];
                bzero(del, sizeof(del));
                strcpy(del, "rm -rf ");
                strcat(del, db);
                // sprintf(del, "rm -rf %s", db);
                system(del);
                bzero(database, sizeof(database));
                // memset(database, 0, sizeof(database));
                return;
            }
            token = strtok(NULL, "\n");
        }
    } else if (strncmp(command, "DROP TABLE", 10) == 0) {
        if (strlen(database) == 0) {
            strcpy(query_result, "Please use database first!");
            return;
        }

        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd, command+11);

        char *token = strtok(cmd, " ;");

        char file[1024];
        bzero(file, sizeof(file));
        strcpy(file, database);
        strcat(file, "/");
        strcat(file, token);
        // sprintf(file, "%s/%s", database, token);
        remove(file);
    } else if (strncmp(command, "DROP COLUMN", 11) == 0) {
        if (strlen(database) == 0) {
            strcpy(query_result, "Please use database first!");
            return;
        }
        char cmd[1024];
        bzero(cmd,sizeof(cmd));
        strcpy(cmd, command+11);
        
        char *col;
        col = strtok(cmd, " ");
        printf("%s",col);
        strtok(NULL, " ");

        char *table;
        table = strtok(NULL, " ;");

        char file[1024];
        bzero(file, sizeof(file));
        strcpy(file, database);
        strcat(file, "/");
        strcat(file, table);
        // sprintf(file, "%s/%s", database, table);
        
        column *cols = parse_db(database, table);

        FILE *fptr;
        fptr = fopen(file, "w");
        if(fptr == NULL)
        {
            strcpy(query_result, strerror(errno));
            return;
        }
        char buf[1024];
        // memset(buf, 0, sizeof(buf));
        bzero(buf, sizeof(buf));
        char temp[1024];
        
        for (int c = 0; c < table_column; c++) {
            bzero(temp, sizeof(temp));
            if (strcmp(cols[c].name, col) != 0) {
                bzero(temp, sizeof(temp));
                // strcpy(buf, cols[c].name);
                // strcat(buf,"|");
                // bzero(temp, sizeof(temp));
                // strcat(buf, cols[c].type);
                // strcat(buf, "\t");
                sprintf(buf, "%s%s|%s\t",buf, cols[c].name, cols[c].type);
            }
        }
        buf[strlen(buf) - 1] = '\n';

        for (int r = 1; r < table_row; r++) {
            bzero(temp, sizeof(temp));
            for (int c = 0; c < table_column; c++) {
                bzero(temp, sizeof(temp));
                if (strcmp(cols[c].name, col) != 0) {
                    bzero(temp, sizeof(temp));
                    sprintf(buf, "%s%s\t", buf, cols[c].contents[r]);
                }
            }
            buf[strlen(buf) - 1] = '\n';
        }

        fprintf(fptr, "%s\n", buf);
        fclose(fptr);
    } else if (strncmp(command, "INSERT INTO", 11) == 0) {
        if (strlen(database) == 0) {
            strcpy(query_result, "Please use database first!");
            return;
        }

        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd,command + 12);

        char *token = strtok(cmd, "( ");

        char table[1024];
        bzero(table, sizeof(table));
        strcpy(table, token);
        // sprintf(table, "%s", token);
        char temp[1024];
        int i = 0;
        char *argv[10];
        for (int j = 0; j < 10; ++j) {
            bzero(temp, sizeof(temp));
            argv[j] = (char*)malloc(sizeof(char)*1024);
        }

        while (token != NULL) {
            token = strtok(NULL, "(),; ");
            if (token == NULL) break;
            bzero(temp, sizeof(temp));
            sprintf(argv[i], "%s", token);
            i++;
        }

        char file[1024];    
        bzero(file, sizeof(file));
        strcpy(file, database);
        strcat(file, "/");
        strcat(file, table);
        puts("");
        // sprintf(file, "%s/%s", database, table);

        FILE *fptr;
        fptr = fopen(file, "a");
        if(fptr == NULL)
        {
            strcpy(query_result, strerror(errno));
            return;
        }

        count_column(database, table);
        for (int j = 0; j < table_column; j++) {
            if (j < i - 1) {
                fprintf(fptr, "%s\t", argv[j]);
            } else {
                fprintf(fptr, "%s\n", argv[j]);
            }
        }

        fclose(fptr);
    } else if (strncmp(command, "UPDATE", 6) == 0) {
        if (strlen(database) == 0) {
            strcpy(query_result, "Please use database first!");
            return;
        }

        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd, command+6);

        char *token = strtok(cmd, " ");

        char table[1024];
        bzero(table, sizeof(table));
        strcpy(table, token);
        // sprintf(table, "%s", token);

        token = strtok(NULL, " ");

        token = strtok(NULL, " ;=");
        char set_left[1024];
        bzero(set_left, sizeof(set_left));
        strcpy(set_left, token);
        // sprintf(set_left, "%s", token);

        token = strtok(NULL, " ;=");
        char set_right[1024];
        bzero(set_right, sizeof(set_right));
        strcpy(set_right, token);
        // sprintf(set_right, "%s", token);

        char left[1024] = "";
        char right[1024] = "";
        
        strcpy(cmd, command+6);
        // sprintf(cmd, "%s", command+6);
        if (strstr(cmd, "WHERE") != NULL) {
            token = strtok(NULL, " ");

            token = strtok(NULL, " =;");
            strcpy(left, token);
            // sprintf(left, "%s", token);

            token = strtok(NULL, " =;");
            strcpy(right, token);
            // sprintf(right, "%s", token);
        }

        char file[1024];
        bzero(file, sizeof(file));
        strcpy(file, database);
        strcat(file, "/");
        strcat(file, table);
        // sprintf(file, "%s/%s", database, table);

        column *cols = parse_db(database, table);
        if (strlen(left) && strlen(right)) {
            int idx_left;
            for (int c = 0; c < table_column; c++) {
                if (strcmp(cols[c].name, left) == 0) {
                    idx_left = c;
                    break;
                }
            }
            for (int r = 1; r < table_row; r++) {
                for (int c = 0; c < table_column; c++) {
                    if (strcmp(cols[c].name, set_left) == 0) {
                        if (strcmp(cols[idx_left].contents[r], right) == 0) {
                            strcpy(cols[c].contents[r], set_right);
                        }
                    }
                }
            }

            FILE *fptr;
            fptr = fopen(file, "w");
            if(fptr == NULL)
            {
                strcpy(query_result, strerror(errno));
                return;
            }

            

            for (int r = 0; r < table_row; ++r) {
                char get[1024];
                bzero(get,sizeof(get));
                strcpy(get, getLine(cols, r, table_column));
                fprintf(fptr, "%s\n", get);
            }

            fclose(fptr);
        } else {
            for (int c = 0; c < table_column; ++c) {
                if (strcmp(cols[c].name, set_left) == 0) {
                    for (int r = 1; r < table_row; ++r) {
                        strcpy(cols[c].contents[r] , set_right);
                        // sprintf(cols[c].contents[r], "%s", set_right);
                    }
                }
            }

            FILE *fptr;
            fptr = fopen(file, "w");
            if(fptr == NULL)
            {
                strcpy(query_result, strerror(errno));
                return;
            }

            for (int r = 0; r < table_row; ++r) {
                char get[1024];
                bzero(get, sizeof(get));
                strcpy(get, getLine(cols, r, table_column));
                fprintf(fptr, "%s\n", get);
            }

            fclose(fptr);
        }
    } else if (strncmp(command, "DELETE FROM", 11) == 0) {
        if (strlen(database) == 0) {
            strcpy(query_result, "Please use database first!");
            return;
        }
        
        char cmd[1024];
        bzero(cmd,sizeof(cmd));
        strcpy(cmd, command+11);
        // sprintf(cmd, "%s", command+11);

        char *token = strtok(cmd, "; ");

        char table[1024];
        bzero(table, sizeof(table));
        strcpy(table, token);
        // sprintf(table, "%s", token);

        char left[1024] = "";
        char right[1024] = "";

        strcpy(cmd, command+11);
        // sprintf(cmd, "%s", command+11);
        if (strstr(cmd, "WHERE") != NULL) {
            token = strtok(NULL, " ");

            token = strtok(NULL, "=; ");
            sprintf(left, "%s", token);

            token = strtok(NULL, "=; ");
            sprintf(right, "%s", token);
        }

        char file[1024];
        bzero(file, sizeof(file));
        strcpy(file, database);
        strcat(file, "/");
        strcat(file, table);
        // sprintf(file, "%s/%s", database, table);

        FILE *fptr;
        fptr = fopen(file, "r");
        if(fptr == NULL)
        {
            strcpy(query_result, strerror(errno));
            return;
        }

        column *cols = parse_db(database, table);

        if (strlen(left) && strlen(right)) {
            char buf[1024] = "";
            bzero(buf, sizeof(buf));
            for (int c = 0; c < table_column; ++c) {
                if (c < table_column - 1) {
                    sprintf(buf, "%s%s|%s\t", buf, cols[c].name, cols[c].type);
                } else {
                    sprintf(buf, "%s%s|%s\n", buf, cols[c].name, cols[c].type);
                }
            }
            
            for (int r = 1; r < table_row; ++r) {
                int found;
                found = 0;

                for (int c = 0; c < table_column; ++c) {
                    if (strcmp(cols[c].name, left) == 0 && strcmp(cols[c].contents[r], right) == 0) {
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    sprintf(buf, "%s%s\n", buf, getLine(cols, r, table_column));
                }
            }

            buf[strlen(buf) - 1] = '\0';

            fclose(fptr);

            fptr = fopen(file, "w");
            if(fptr == NULL)
            {
                strcpy(query_result, strerror(errno));
                return;
            }

            fprintf(fptr, "%s\n", buf);
            fclose(fptr);
        } else {
            char buf[1024];
            bzero(buf, sizeof(buf));
            fseek(fptr, 0, SEEK_END);
            long fsize = ftell(fptr);
            rewind(fptr);

            fread(buf, 1, sizeof(buf), fptr);

            char *token = strtok(buf, "\n");
            char line[1024];
            bzero(line, sizeof(line));
            strcpy(line, token);
            // sprintf(line, "%s", token);

            line[strlen(line) - 1] = '\0';
            fclose(fptr);

            fptr = fopen(file, "w");
            if(fptr == NULL)
            {
                strcpy(query_result, strerror(errno));
                return;
            }
            fprintf(fptr, "%s\n", line);
            fclose(fptr);
        }
    } else if (strncmp(command, "SELECT", 6) == 0) {
        if (strlen(database) == 0) {
            strcpy(query_result, "Please use database first!");
            return;
        }
        char cmd[1024];
        bzero(cmd, sizeof(cmd));
        strcpy(cmd, command);
        // sprintf(cmd, "%s", command);

        int argc = 0;
        char *argv[10];
        for (int i = 0; i < 10; ++i) {
            argv[i] = (char*)malloc(sizeof(char) * 1024);
        }

        char *token = strtok(cmd, " ");
        token = strtok(NULL, ", ");

        while (strcmp(token, "FROM") != 0) {
            sprintf(argv[argc], "%s", token);
            
            argc++;
            token = strtok(NULL, ", ;");
        }

        token = strtok(NULL, ", ;");
        char table[1024];
        bzero(table, sizeof(table));
        strcpy(table, token);
        // sprintf(table, "%s", token);
        strcpy(cmd, command);
        // sprintf(cmd, "%s", command);
        char left[1024] = "";
        char right[1024] = "";
        bzero(left, sizeof(left));
        bzero(right, sizeof(right));

        if (strstr(cmd, "WHERE") != NULL) {
            token = strtok(NULL, " =");

            token = strtok(NULL, " ;=");
            sprintf(left, "%s", token);

            token = strtok(NULL, ", ;=");
            sprintf(right, "%s", token);
        }
        
        char *res = db_select(database, table, argc, argv, left, right);
        sprintf(query_result, "%s", res);
    } else {
        sprintf(query_result, "Command not found!");
    }
}

void dump(char *command) {
    char cmd[1024];
    bzero(cmd,sizeof(cmd));
    strcpy(cmd, command);
    // sprintf(cmd, "%s", command);

    char *token = strtok(cmd, " ");
    token = strtok(NULL, " ");

    char db[1024];
    bzero(db, sizeof(db));
    strcpy(db, token);
    // sprintf(db, "%s", token);

    if (strcmp(db, "*") == 0) {
        DIR *dir = opendir(server_path);
        struct dirent *dp;

        while ((dp = readdir(dir)) != NULL) {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                DIR *try = opendir(dp->d_name);
                if (try) {
                    char file[1024];
                    sprintf(file, "dump %s", dp->d_name);
                    dump(file);

                    // char file[1024];
                    // bzero(file, sizeof(file));
                    sprintf(file, "%s/%s.backup", server_path, dp->d_name);

                    FILE *fptr;
                    fptr = fopen(file, "a+");
                    if(fptr == NULL)
                    {
                        strcpy(query_result, strerror(errno));
                        return;
                    }

                    fprintf(fptr, "%s", query_result);
                    memset(query_result, 0, sizeof(query_result));

                    fclose(fptr);
                    closedir(try);
                }
            }
        }
    } else {
        char db_to_dump[1024];
        bzero(db_to_dump, sizeof(db_to_dump));
        strcpy(db_to_dump, server_path);
        strcat(db_to_dump, "/");
        strcat(db_to_dump, db);
        // sprintf(db_to_dump, "%s/%s", server_path, db);

        char *buff;
        buff = (char*)malloc(sizeof(char)*1024);
        char *argv[] = {"database"};
        sprintf(buff, "%s", db_select("root", "db_permission", 1, argv, "username", username));

        strcpy(cmd, command);
        // sprintf(cmd, "%s", command);
        DIR *dir;
        dir = opendir(db);
        if (!dir) {
            strcpy(query_result, "invalid database ");
            strcat(query_result, db);
            // sprintf(query_result, "Invalid db %s!", db);
            return;
        }
        closedir(dir);

        char *line = strtok(buff, "\n");
        line = strtok(NULL, "\n");

        int found;
        found = 0;
        
        while (line != NULL) {
            if (strcmp(db, line) == 0) {
                found = 1;
                break;
            }

            line = strtok(NULL, "\n");
        }
        
        if (found == 1) {
            strcpy(database, db);
            // sprintf(database, "%s", db);
            puts("");
        } else {
            sprintf(query_result, "You can't open %s", db);
            return;
        }

        if (strlen(database) == 0) {
            sprintf(query_result, "You can't open %s!", db);
            return;
        }

        DIR *dir_open = opendir(db_to_dump);
        struct dirent *dp;

        if (!dir_open) {
            sprintf(query_result, "Invalid db %s!", db);
        }

        while ((dp = readdir(dir_open)) != NULL) {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                char file[1024];
                sprintf(file, "%s/%s", db_to_dump, dp->d_name);

                FILE *fptr;
                fptr = fopen(file, "r");
                if(fptr == NULL)
                {
                    strcpy(query_result, strerror(errno));
                    return;
                }
                sprintf(query_result, "%s\nDROP TABLE %s;\nCREATE TABLE %s (", query_result, dp->d_name, dp->d_name);

                char buf[1024];
                bzero(buf, sizeof(buf));

                fgets(buf, 1024, fptr);
                
                char *token;
                token = strtok(buf, "\t");
                while (token != NULL) {
                    char *tab;
                    tab = strstr(token, "|");

                    sprintf(query_result, "%s%.*s %s, ", query_result, tab - token, token, tab + 1);

                    token = strtok(NULL, "\t");
                }
                query_result[strlen(query_result) - 3] = ')';
                query_result[strlen(query_result) - 2] = ';';
                query_result[strlen(query_result) - 1] = '\0';

                sprintf(query_result, "%s\n", query_result);

                while (fgets(buf, 1024, fptr) != NULL) {
                    sprintf(query_result, "%sINSERT INTO %s (", query_result, dp->d_name);
                    token = strtok(buf, "\t");
                    while (token != NULL) {
                        sprintf(query_result, "%s%s, ", query_result, token);

                        token = strtok(NULL, "\t");
                    }
                    query_result[strlen(query_result) - 3] = ')';
                    query_result[strlen(query_result) - 2] = ';';
                    query_result[strlen(query_result) - 1] = '\0';

                    sprintf(query_result, "%s\n", query_result);
                }

                fclose(fptr);
            }
        }

        closedir(dir);
    }

}

int main() {
    system("mkdir -p databases/root");
    system("touch databases/root/users");
    system("du databases/root/users | if [ `cut -f1` -eq 0 ]; then echo 'user|string\tpassword|string\nroot\troot' > databases/root/users; fi");
    system("touch databases/root/db_permission");
    system("du databases/root/db_permission | if [ `cut -f1` -eq 0 ]; then echo 'username|string\tdatabase|string\nroot\troot' > databases/root/db_permission; fi");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int valsend;
    int valread;

    // msg for client
    char *login_success = "login success";
    char *login_fail = "login failed";
    char *wrong_username = "wrong username";
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    pid_t pid, sid;

    pid = fork();
    
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if (chdir("/home/kali/FP/database/databases") < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int logged_in = 0;
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, username, 1024);
        if(valread < 0)
        {
            printf("read error");
        }

        valread = read(new_socket, password, 1024);
        if(valread< 0)
        {
            printf("read error");
        }

        char *argv[] = {"password"};
        char *res = db_select("root", "users", 1, argv, "user", username);
        if (res == NULL) {
            valsend = send(new_socket, wrong_username, strlen(wrong_username), 0);
            if(valsend < 0)
            {
                printf("send error");
            }
        } else {
            char *token;
            token = strtok(res, "\n");
            token = strtok(NULL, "\n");
            if (strcmp(token, password) == 0) {
                valsend = send(new_socket, login_success, strlen(login_success), 0);
                if(valsend < 0)
                {
                    printf("send error");
                }
                logged_in = 1;
            } else {
                send(new_socket, login_fail, strlen(login_fail), 0);
                if(valsend < 0)
                {
                    printf("send error");
                }
            }
        }

        // memset(buffer, 0, sizeof(buffer));
        bzero(buffer, sizeof(buffer));

        if (logged_in == 1) {
            while (1) {
                read(new_socket, buffer, 1024);
                if(read < 0)
                {
                    printf("error recv");
                }
                if (strcmp(buffer, "exit") == 0) {
                    bzero(database, sizeof(database));
                    bzero(username, sizeof(username));
                    bzero(password, sizeof(password));
                    // memset(database, 0, sizeof(database));
                    // memset(username, 0, sizeof(username));
                    // memset(password, 0, sizeof(password));
                    break;
                } else if (strncmp(buffer, "dump", 4) == 0) {
                    dump(buffer);
                    valsend = send(new_socket, query_result, strlen(query_result), 0);
                    if(valsend < 0)
                    {
                        printf("error send");
                    }
                    // memset(query_result, 0, sizeof(query_result));
                    bzero(query_result, sizeof(query_result));
                    break;
                }

                db_execute(buffer);
                if (strlen(query_result)) {
                    valsend = send(new_socket, query_result, strlen(query_result), 0);
                    if(valsend < 0)
                    {
                        printf("send error");
                    }
                    // memset(query_result, 0, sizeof(query_result));
                    bzero(query_result, sizeof(query_result));
                } else {
                    char *yes = "yes";
                    valsend = send(new_socket, yes, strlen(yes), 0);
                    if(valsend < 0)
                    {
                        printf("error send");
                    }
                    // memset(query_result, 0, sizeof(query_result));
                    bzero(query_result, sizeof(query_result));
                }

                // memset(buffer, 0, sizeof(buffer));
                bzero(buffer, sizeof(buffer));
            }
        }

        close(new_socket);
    }

    return 0;
}