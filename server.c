/*UDP Echo Server*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include "connect.c"
#include <jansson.h>

#define BUFF_SIZE 1024
#define MAX_USERNAME_LENGTH 25
#define MAX_PASSWORD_LENGTH 25
#define BACKLOG 20 /* Number of allowed connections */

/* Receive and echo message to client */
void *echo(void *);
MYSQL *connection;
MYSQL_RES *result;
MYSQL_ROW row;
char *json_str;
char *json_str_fail;
typedef struct Account
{
    int id;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int thread_address;
    char name[255];
    int age;
    char phone[25];
    char address[255];
} Account;
typedef struct Location
{
    int id;
    char name[50];
    int type;
    char address[255];
    int createdBy;
} Location;

typedef struct Comment
{
    int id;
    int locationId;
    char content[1000];
    int createdBy;
} Comment;
void splitString(const char source[], char **firstSubstring, char **secondSubstring)
{
    *firstSubstring = (char *)malloc(9 * sizeof(char));
    strncpy(*firstSubstring, source, 8);
    (*firstSubstring)[8] = '\0';

    size_t length = strlen(source) - 8;
    *secondSubstring = (char *)malloc((length + 1) * sizeof(char));
    strcpy(*secondSubstring, source + 8);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("%s\n", "Wrong number of arguments!");
        exit(0);
    }
    // TCP
    int listenfd, *connfd;
    struct sockaddr_in server;  /* server's address information */
    struct sockaddr_in *client; /* client's address information */
    int sin_size;
    pthread_t tid;

    MYSQL_RES *result;
    // const char *query_create_table2 = "CREATE TABLE user (id INT PRIMARY KEY AUTO_INCREMENT, thread_address varchar(255), password varchar(55), name varchar(50), age int,phone varchar(25),address varchar(255));";

    // Initialize MySQL connection
    connection = mysql_init(NULL);
    if (connection == NULL)
    {
        fprintf(stderr, "Failed to initialize MySQL connection\n");
        return 1;
    }

    // Connect to the MySQL server
    if (mysql_real_connect(connection, "localhost", "root", "tienbk255", NULL, 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Failed to connect to the MySQL server: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }
    // Drop the database if it exists
    if (mysql_query(connection, query_drop_db) != 0)
    {
        fprintf(stderr, "Failed to drop database: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }

    // Create the database
    if (mysql_query(connection, query_create_db) != 0)
    {
        fprintf(stderr, "Failed to create database: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }

    // Use the database
    if (mysql_query(connection, query_use_db) != 0)
    {
        fprintf(stderr, "Failed to select database: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }
    // Create table
    if (mysql_query(connection, query_create_table_1) != 0 || mysql_query(connection, query_create_table_2) != 0 || mysql_query(connection, query_create_table_3) != 0 || mysql_query(connection, query_create_table_4) != 0 || mysql_query(connection, query_create_table_5) != 0 || mysql_query(connection, query_create_table_6) != 0 || mysql_query(connection, query_create_table_7) != 0)
    {
        fprintf(stderr, "Failed to create table: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }
    // Insert data to database
    if (mysql_query(connection, insert_query_1) != 0 || mysql_query(connection, insert_query_2) != 0 || mysql_query(connection, insert_query_3) != 0 || mysql_query(connection, insert_query_4) != 0 || mysql_query(connection, insert_query_5) != 0 || mysql_query(connection, insert_query_6) != 0 || mysql_query(connection, insert_query_7) != 0)
    {
        fprintf(stderr, "Insett query: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }

    printf("Tables created successfully\n");

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

    if (bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    sin_size = sizeof(struct sockaddr_in);
    client = malloc(sin_size);

    // Step 3: Communicate with clients

    while (1)
    {
        connfd = malloc(sizeof(int));
        if ((*connfd = accept(listenfd, (struct sockaddr *)client, &sin_size)) == -1)
            perror("\nError: ");
        printf("You got a connection from %s\n", inet_ntoa(client->sin_addr)); /* prints client's IP */

        /* For each client, spawns a thread, and the thread handles the new client */
        pthread_create(&tid, NULL, &echo, connfd);
    }
    mysql_free_result(result);
    // Clean up resources
    mysql_close(connection);
    printf("Database closed!\n");
    close(listenfd);
    return 0;
}

void *echo(void *arg)
{
    int connfd;
    int userId = 0;
    int bytes_sent, bytes_received;
    char buff[BUFF_SIZE + 1];
    char *keyString;
    char *objectString;
    char *query;
    char *tempStr = (char *)malloc(200 * sizeof(char));
    char numStr[25];

    connfd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());
    json_t *jsonFaild = json_object();
    json_object_set_new(jsonFaild, "status", json_integer(0));
    json_str_fail = json_dumps(jsonFaild, JSON_ENCODE_ANY);

    while (1)
    {
        printf("Waiting for request....\n");
        bytes_received = recv(connfd, buff, BUFF_SIZE - 1, 0);
        printf("Thead_add:%d\n", connfd);

        if (bytes_received <= 0)
        {
            perror("\nError: Byte recceive is 0");
            break;
        }
        else
        {
            printf("Get %d bytes: %s \n", bytes_received, buff);
            buff[bytes_received] = '\0';

            if (strcmp(buff, "exit") == 0 || strlen(buff) == 0) // Client close connection (exit)
            {
                break;
            }
            else
            {
                if (strlen(buff) < 8)
                    break;
                splitString(buff, &keyString, &objectString);
                printf("First Substring: %s\n", keyString);
                printf("Second Substring: %s\n", objectString);
                printf("Client send message:%s with length:%d\n", buff, strlen(buff));
                query = getQuerySQL(keyString, objectString);
                printf("234\n");
                if (query == NULL || strcmp(query, "") == 0)
                    break;
                printf("%s\n", query);

                // Process and send back to client
                if (strlen(keyString) == 0)
                {
                    bytes_sent = send(connfd, "", (int)strlen(query), 0); /* inform the client account not exist */
                    if (bytes_sent < 0)
                    {
                        perror("\nError: ");
                    }
                }
                else
                {
                    if (strcmp(keyString, "REQ_LOGI") == 0)
                    { // Login account
                        // username and password
                        printf("REQ_LOGI ne\n");
                        printf("%s\n", query);
                        result = selectQuery(connection, query);
                        if (result == NULL)
                        {
                            fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                        }
                        // Check if data is present
                        unsigned long num_rows = mysql_num_rows(result);
                        if (num_rows == 0)
                        {
                            // Send wrong password or username to user
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            else
                            {
                                printf("Send back Oke!\n");
                            }
                        }
                        else
                        {
                            // Update thread addres database
                            row = mysql_fetch_row(result);
                            userId = atoi(row[0]);
                            strcpy(tempStr, "UPDATE user SET thread_address =");
                            sprintf(numStr, "%d", connfd);
                            strcat(tempStr, numStr);
                            strcat(tempStr, " WHERE id=");
                            strcat(tempStr, row[0]);
                            strcat(tempStr, ";");
                            printf("%s\n", tempStr);
                            if (mysql_query(connection, tempStr))
                            {
                                fprintf(stderr, "Failed to execute SELECT query: %s\n", mysql_error(connection));
                                return 1;
                            }

                            // Send the json string to client
                            json_t *json = json_object();
                            json_object_set_new(json, "status", json_integer(1));
                            json_object_set_new(json, "id", json_integer(atoi(row[0])));
                            json_object_set_new(json, "name", json_string(row[4]));
                            json_object_set_new(json, "age", json_string(row[5]));
                            json_object_set_new(json, "phone", json_string(row[6]));
                            json_object_set_new(json, "address", json_string(row[7]));

                            json_str = json_dumps(json, JSON_ENCODE_ANY);

                            bytes_sent = send(connfd, json_str, (int)strlen(json_str), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            else
                            {
                                printf("Send back Oke!\n");
                            }
                            json_decref(json);
                        }
                    }
                    else if (strcmp(keyString, "REQ_REGI") == 0)
                    {
                        printf("Request register\n");
                        // Register account
                        long affected_rows = updateQuery(connection, query);
                        if (affected_rows == 0)
                        {
                            printf("No rows were updated\n");
                            // Send fail to client
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            else
                            {
                                printf("Send back Oke!\n");
                            }
                        }
                        else
                        {
                            long long int insertedId = mysql_insert_id(connection);
                            printf("Inserted ID: %lld\n", insertedId);
                            json_t *json = json_object();
                            json_object_set_new(json, "status", json_integer(1));
                            json_object_set_new(json, "id", json_integer(insertedId));

                            json_str = json_dumps(json, JSON_ENCODE_ANY);

                            bytes_sent = send(connfd, json_str, (int)strlen(json_str), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            else
                            {
                                printf("Send back Oke!\n");
                            }
                            json_decref(json);
                        }
                    }
                    else if (strcmp(keyString, "REQ_CPAS") == 0)
                    {
                        printf("Change password\n");
                        printf("%s\n", query);
                        // userId, oldpassword, newpassword
                        long affected_rows = updateQuery(connection, query);
                        if (affected_rows == 0)
                        {
                            // Send fail to client
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        else
                        {
                            json_t *json = json_object();
                            json_object_set_new(json, "status", json_integer(1));
                            json_str = json_dumps(json, JSON_ENCODE_ANY);

                            bytes_sent = send(connfd, json_str, (int)strlen(json_str), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            json_decref(json);
                        }
                    }
                    else if (strcmp(keyString, "REQ_LOCA") == 0)
                    {
                        // Requets get location infor
                        // locationId
                        result = selectQuery(connection, query);
                        if (result == NULL)
                        {
                            fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        unsigned long num_rows = mysql_num_rows(result);
                        if (num_rows == 0)
                        {
                            // Send wrong password or username to user
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        else
                        {
                            // Process data send to client
                            json_t *root = json_object();
                            json_t *array = json_array();
                            int index = 0;
                            while ((row = mysql_fetch_row(result)) != NULL)
                            {
                                if (index == 0)
                                {
                                    json_object_set_new(root, "id", json_integer(atoi(row[0])));
                                    json_object_set_new(root, "name", json_string(row[1]));
                                    json_object_set_new(root, "type", json_integer(atoi(row[2])));
                                    json_object_set_new(root, "address", json_string(row[1]));
                                }
                                json_t *object1 = json_object();
                                json_object_set_new(object1, "name", json_string(row[5]));
                                json_object_set_new(object1, "content", json_string(row[4]));
                                json_array_append_new(array, object1);
                                index++;
                            }
                            json_object_set_new(root, "comment", array);
                            json_str = json_dumps(root, JSON_ENCODE_ANY);
                            bytes_sent = send(connfd, json_str, (int)strlen(json_str), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            json_decref(root);
                            json_decref(array);
                        }
                    }
                    else if (strcmp(keyString, "REQ_CDET") == 0)
                    {
                        printf("Change user\n");
                        // Change user infor
                    }
                    else if (strcmp(keyString, "PUT_SHLC") == 0)
                    {
                        // Put the share location
                        // userId, name, type, address
                        long affected_rows = updateQuery(connection, query);
                        if (affected_rows == 0)
                        {
                            // Send fail to client
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            else
                            {
                                printf("Send back Oke!\n");
                            }
                        }
                        else
                        {
                            // Send messgae response success to client
                            long long int insertedId = mysql_insert_id(connection);
                            json_t *json = json_object();
                            json_object_set_new(json, "status", json_integer(1));
                            json_object_set_new(json, "locationId", json_integer(insertedId));
                            json_str = json_dumps(json, JSON_ENCODE_ANY);
                            bytes_sent = send(connfd, json_str, (int)strlen(json_str), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            json_error_t error;
                            json_t *root = json_loads(objectString, 0, &error);
                            if (!root)
                            {
                                fprintf(stderr, "JSON parsing error: %s\n", error.text);
                                return NULL;
                            }
                            int id = json_integer_value(json_object_get(root, "userId"));

                            // Send messgae to all friend client
                            strcpy(tempStr, "SELECT user2, thread_address from friend join user ON friend.user2 = user.id WHERE user.thread_address IS NOT NULL AND friend.user1 =");
                            sprintf(numStr, "%d", id);
                            strcat(tempStr, numStr);
                            strcat(tempStr, ";");
                            result = selectQuery(connection, tempStr);
                            if (result == NULL)
                            {
                                fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                                bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                                if (bytes_sent < 0)
                                {
                                    perror("\nError: ");
                                }
                            }
                            unsigned long num_rows = mysql_num_rows(result);
                            if (num_rows > 0)
                            {
                                while ((row = mysql_fetch_row(result)) != NULL)
                                {
                                    if (strcmp(row[1], "") != 0)
                                    {
                                        bytes_sent = send(atoi(row[1]), objectString, (int)strlen(objectString), 0); /* Send back to client */
                                        if (bytes_sent < 0)
                                        {
                                            perror("\nError: ");
                                        }
                                        printf("Send to other client %d with %d byte\n", atoi(row[1]), bytes_sent);
                                    }
                                }
                            }
                            json_decref(json);
                            json_decref(root);
                        }
                    }
                    else if (strcmp(keyString, "PUT_RVIE") == 0)
                    {
                        // Push comment
                        // userId, name, locationId, content
                        long affected_rows = updateQuery(connection, query);
                        if (affected_rows == 0)
                        {
                            // Send fail to client
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            else
                            {
                                printf("Send back Oke!\n");
                            }
                        }
                        else
                        {
                            // Send result to client
                            json_t *json = json_object();
                            json_object_set_new(json, "status", json_integer(1));
                            json_str = json_dumps(json, JSON_ENCODE_ANY);
                            bytes_sent = send(connfd, json_str, (int)strlen(json_str), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            json_error_t error;
                            json_t *root = json_loads(objectString, 0, &error);
                            if (!root)
                            {
                                fprintf(stderr, "JSON parsing error: %s\n", error.text);
                                return NULL;
                            }
                            int id = json_integer_value(json_object_get(root, "userId"));
                            // Send messgae to all friend client
                            strcpy(tempStr, "SELECT user2, thread_address from friend join user ON friend.user2 = user.id WHERE user.thread_address IS NOT NULL AND friend.user1 =");
                            sprintf(numStr, "%d", id);
                            strcat(tempStr, numStr);
                            strcat(tempStr, ";");
                            result = selectQuery(connection, tempStr);
                            if (result == NULL)
                            {
                                fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                                bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                                if (bytes_sent < 0)
                                {
                                    perror("\nError: ");
                                }
                            }
                            unsigned long num_rows = mysql_num_rows(result);
                            if (num_rows > 0)
                            {
                                while ((row = mysql_fetch_row(result)) != NULL)
                                {
                                    if (strcmp(row[1], "") != 0)
                                    {
                                        bytes_sent = send(atoi(row[1]), objectString, (int)strlen(objectString), 0); /* Send back to client */
                                        if (bytes_sent < 0)
                                        {
                                            perror("\nError: ");
                                        }
                                        printf("Send to other client %d with %d byte\n", atoi(row[1]), bytes_sent);
                                    }
                                }
                            }
                            json_decref(json);
                            json_decref(root);
                        }
                    }
                    else if (strcmp(keyString, "GET_FRIE") == 0)
                    {
                        // Get list of friend
                        // userId
                        // select id, name, age, phone, address  from friend join user on friend.user2 = user.id WHERE friend.user1 = 1;
                        result = selectQuery(connection, query);
                        if (result == NULL)
                        {
                            fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        unsigned long num_rows = mysql_num_rows(result);
                        if (num_rows >= 0)
                        {
                            json_t *root = json_object();
                            json_t *jsonArray = json_array();
                            json_object_set_new(root, "success", json_integer(1));
                            while ((row = mysql_fetch_row(result)) != NULL)
                            {
                                json_t *userObj = json_object();
                                json_object_set_new(userObj, "id", json_integer(atoi(row[0])));
                                json_object_set_new(userObj, "name", json_string(row[1]));
                                json_object_set_new(userObj, "age", json_integer(atoi(row[2])));
                                json_object_set_new(userObj, "phone", json_string(row[3]));
                                json_object_set_new(userObj, "address", json_string(row[4]));
                                json_array_append_new(jsonArray, userObj);
                            }
                            json_object_set_new(root, "friend", jsonArray);
                            char *jsonString = json_dumps(root, JSON_ENCODE_ANY);
                            bytes_sent = send(connfd, jsonString, (int)strlen(jsonString), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            free(jsonString);
                            json_decref(jsonArray);
                            json_decref(root);
                        }
                    }
                    else if (strcmp(keyString, "GET_SLOC") == 0)
                    {
                        // GET save location
                        //  userId
                        result = selectQuery(connection, query);
                        if (result == NULL)
                        {
                            fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        unsigned long num_rows = mysql_num_rows(result);
                        if (num_rows > 0)
                        {
                            json_t *root = json_object();
                            json_t *jsonArray = json_array();
                            json_object_set_new(root, "success", json_integer(1));
                            while ((row = mysql_fetch_row(result)) != NULL)
                            {
                                json_t *userObj = json_object();
                                json_object_set_new(userObj, "id", json_integer(atoi(row[0])));
                                json_object_set_new(userObj, "name", json_string(row[1]));
                                json_object_set_new(userObj, "type", json_integer(atoi(row[2])));
                                json_object_set_new(userObj, "address", json_string(row[3]));
                                json_array_append_new(jsonArray, userObj);
                            }
                            json_object_set_new(root, "saveLocation", jsonArray);
                            char *jsonString = json_dumps(root, JSON_ENCODE_ANY);
                            bytes_sent = send(connfd, jsonString, (int)strlen(jsonString), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            free(jsonString);
                            json_decref(jsonArray);
                            json_decref(root);
                        }
                    }
                    else if (strcmp(keyString, "GET_FLOC") == 0)
                    {
                        // GET favorite location
                        //  userId
                        result = selectQuery(connection, query);
                        if (result == NULL)
                        {
                            fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        unsigned long num_rows = mysql_num_rows(result);
                        if (num_rows >= 0)
                        {
                            json_t *root = json_object();
                            json_t *jsonArray = json_array();
                            json_object_set_new(root, "success", json_integer(1));
                            while ((row = mysql_fetch_row(result)) != NULL)
                            {
                                json_t *userObj = json_object();
                                json_object_set_new(userObj, "id", json_integer(atoi(row[0])));
                                json_object_set_new(userObj, "name", json_string(row[1]));
                                json_object_set_new(userObj, "type", json_integer(atoi(row[2])));
                                json_object_set_new(userObj, "address", json_string(row[3]));
                                json_array_append_new(jsonArray, userObj);
                            }
                            json_object_set_new(root, "favoriteLocation", jsonArray);
                            char *jsonString = json_dumps(root, JSON_ENCODE_ANY);
                            bytes_sent = send(connfd, jsonString, (int)strlen(jsonString), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            free(jsonString);
                            json_decref(jsonArray);
                            json_decref(root);
                        }
                    }
                    else if (strcmp(keyString, "GET_USER") == 0)
                    {
                        printf("Get user infor\n");
                        result = selectQuery(connection, query);
                        if (result == NULL)
                        {
                            fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        else
                        {
                            unsigned long num_rows = mysql_num_rows(result);
                            if (num_rows > 0)
                            {
                                printf("Numrow=%d\n", (int)num_rows);
                                while ((row = mysql_fetch_row(result)) != NULL)
                                {
                                    json_t *userObj = json_object();
                                    json_object_set_new(userObj, "success", json_integer(1));
                                    json_object_set_new(userObj, "id", json_integer(atoi(row[0])));
                                    json_object_set_new(userObj, "name", json_string(row[1]));
                                    json_object_set_new(userObj, "age", json_integer(atoi(row[2])));
                                    json_object_set_new(userObj, "phone", json_string(row[3]));
                                    json_object_set_new(userObj, "address", json_string(row[4]));
                                    char *json_s = json_dumps(userObj, JSON_ENCODE_ANY);
                                    bytes_sent = send(connfd, json_s, (int)strlen(json_s), 0); /* Send back to client */
                                    if (bytes_sent < 0)
                                    {
                                        perror("\nError: ");
                                    }
                                    free(json_s);
                                    json_decref(userObj);
                                    break;
                                }
                            }
                        }
                    }
                    else if (strcmp(keyString, "FIND_LOC") == 0)
                    {
                        result = selectQuery(connection, query);
                        if (result == NULL)
                        {
                            fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        unsigned long num_rows = mysql_num_rows(result);
                        if (num_rows >= 0)
                        {
                            json_t *root = json_object();
                            json_t *jsonArray = json_array();
                            json_object_set_new(root, "success", json_integer(1));
                            while ((row = mysql_fetch_row(result)) != NULL)
                            {
                                json_t *userObj = json_object();
                                json_object_set_new(userObj, "locationId", json_integer(atoi(row[0])));
                                json_object_set_new(userObj, "userId", json_integer(atoi(row[1])));
                                json_object_set_new(userObj, "userName", json_string(row[2]));
                                json_object_set_new(userObj, "locationName", json_string(row[3]));
                                json_object_set_new(userObj, "type", json_integer(atoi(row[4])));
                                json_object_set_new(userObj, "address", json_string(row[5]));
                                json_array_append_new(jsonArray, userObj);
                            }
                            json_object_set_new(root, "location", jsonArray);
                            char *jsonString = json_dumps(root, JSON_ENCODE_ANY);
                            bytes_sent = send(connfd, jsonString, (int)strlen(jsonString), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            free(jsonString);
                            json_decref(jsonArray);
                            json_decref(root);
                        }
                    }
                    else if (strcmp(keyString, "GET_FEED") == 0)
                    {
                        result = selectQuery(connection, query);
                        if (result == NULL)
                        {
                            fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
                            bytes_sent = send(connfd, json_str_fail, (int)strlen(json_str_fail), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                        }
                        unsigned long num_rows = mysql_num_rows(result);
                        if (num_rows >= 0)
                        {
                            json_t *root = json_object();
                            json_t *jsonArray = json_array();
                            json_object_set_new(root, "success", json_integer(1));
                            while ((row = mysql_fetch_row(result)) != NULL)
                            {
                                json_t *userObj = json_object();
                                json_object_set_new(userObj, "userId", json_integer(atoi(row[0])));
                                json_object_set_new(userObj, "userName", json_string(row[1]));
                                json_object_set_new(userObj, "locationId", json_integer(atoi(row[2])));
                                json_object_set_new(userObj, "locationName", json_string(row[3]));
                                json_object_set_new(userObj, "type", json_integer(atoi(row[4])));
                                json_object_set_new(userObj, "locationAdd", json_string(row[5]));
                                json_array_append_new(jsonArray, userObj);
                            }
                            json_object_set_new(root, "locationShare", jsonArray);
                            char *jsonString = json_dumps(root, JSON_ENCODE_ANY);
                            bytes_sent = send(connfd, jsonString, (int)strlen(jsonString), 0); /* Send back to client */
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                            }
                            free(jsonString);
                            json_decref(jsonArray);
                            json_decref(root);
                        }
                        
                    }
                }
            }
        }
    }
    if (userId != 0)
    {
        printf("%d\n", userId);
        sprintf(numStr, "%d", userId);
        strcpy(tempStr, "UPDATE user SET thread_address = NULL WHERE id=");
        strcat(tempStr, numStr);
        strcat(tempStr, ";");
        printf("%s\n", tempStr);
        if (mysql_query(connection, tempStr))
        {
            fprintf(stderr, "Failed to execute SELECT query: %s\n", mysql_error(connection));
        }
    }
    free(query);
    free(tempStr);
    printf("Client closed connection\n");
    close(connfd);
}
