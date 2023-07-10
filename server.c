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
    MYSQL_ROW row;
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

    Account acc;
    acc.id = 1;
    acc.age = 23;
    acc.thread_address = 16356426;
    strcpy(acc.name, "Hoang Tien");
    strcpy(acc.username, "tien");
    strcpy(acc.password, "123456");
    strcpy(acc.address, "Thanh Nhan, Hai Ba Trung");
    strcpy(acc.phone, "0926636524");

    json_t *json = json_object();
    json_object_set_new(json, "id", json_integer(acc.id));
    json_object_set_new(json, "age", json_integer(acc.age));
    json_object_set_new(json, "name", json_string(acc.name));
    json_object_set_new(json, "username", json_string(acc.username));
    json_object_set_new(json, "address", json_string(acc.address));
    json_object_set_new(json, "phone", json_string(acc.phone));
    json_str = json_dumps(json, JSON_ENCODE_ANY);
    printf("%s\n", json_str);

    char *temp; // userId, name, address, phone, age
    temp = getQuerySQL("GET_LOCA", "{\"userId\": 1, \"content\": \"Tuey voi, Ha Long Bay\", \"locationId\": 4}");
    printf("Query :\n");
    printf("%s\n", temp);

    if (mysql_query(connection, temp))
    {
        fprintf(stderr, "Failed to execute SELECT query: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }

    // Retrieve and process the result set
    result = mysql_store_result(connection);
    if (result == NULL)
    {
        fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }

    // Fetch each row from the result set
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        // Process each field in the row
        for (unsigned int i = 0; i < mysql_num_fields(result); i++)
        {
            if (row[i] != NULL)
            {
                printf("Field %u: %s\n", i, row[i]);
            }
            else
            {
                printf("Field %u: NULL\n", i);
            }
        }
        printf("\n");
    }

    // Clean up resources
    mysql_free_result(result);

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
    // Clean up resources
    mysql_close(connection);
    printf("Database closed!\n");
    close(listenfd);
    return 0;
}

void *echo(void *arg)
{
    int connfd;
    char temp[BUFF_SIZE];
    int split_result;
    int bytes_sent, bytes_received;
    char buff[BUFF_SIZE + 1];
    char *keyString;
    char *objectString;
    int keyNumber;
    char *query;
    char *response = (char *)malloc(250 * sizeof(char));
    char *tempStr = (char *)malloc(150 * sizeof(char));
    char numStr[15];

    connfd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());
    json_t *jsonFaild = json_object();
    json_object_set_new(jsonFaild, "status", json_integer(0));
    json_str_fail = json_dumps(jsonFaild, JSON_ENCODE_ANY);

    while (1)
    {
        // strcpy(response, "");
        bytes_received = recv(connfd, buff, BUFF_SIZE - 1, 0);
        printf("Thead_add:%d\n", connfd);

        if (bytes_received < 0)
            perror("\nError: ");
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
                splitString(buff, &keyString, &objectString);
                printf("First Substring: %s\n", keyString);
                printf("Second Substring: %s\n", objectString);
                printf("Client send message:%s with length:%d\n", buff, strlen(buff));
                query = getQuerySQL(keyString, objectString);
                printf("%s\n", query);

                // Process and send back to client
                if (strlen(keyString) == 0)
                {
                    bytes_sent = send(connfd, "", (int)strlen(query), 0); /* inform the client account not exist */
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
                        printf("317\n");
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
                                mysql_close(connection);
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
                            free(tempStr);
                        }
                        free(query);
                    }
                    else if (strcmp(keyString, "REQ_REGI") == 0)
                    {
                        printf("Request login\n");
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
                            printf("%ld row(s) were updated\n", affected_rows);
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
                        }
                        free(query);
                    }
                    else if (strcmp(keyString, "REQ_CPAS") == 0)
                    {
                        // userId, newpassword
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
                            else
                            {
                                printf("Send back Oke!\n");
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
                            else
                            {
                                printf("Send back Oke!\n");
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
                            else
                            {
                                printf("Send back Oke!\n");
                            }
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
                    }
                    else if (strcmp(keyString, "PUT_RVIE") == 0)
                    {
                        // Push comment
                        // userId, locationId, content
                    }
                    else if (strcmp(keyString, "GET_LOCA") == 0)
                    {
                        // Get the location infor
                        // locationId
                    }
                    else if (strcmp(keyString, "GET_FRIE") == 0)
                    {
                        // Get list of friend
                        // userId
                        // select id, name, age, phone, address  from friend join user on friend.user2 = user.id WHERE friend.user1 = 1;
                    }
                    else if (strcmp(keyString, "GET_SLOC") == 0)
                    {
                        // GET save location
                        //  userId
                    }
                    else if (strcmp(keyString, "GET_FLOC") == 0)
                    {
                        // GET favorite location
                        //  userId
                    }
                }
            }
        }
    }
    printf("Client closed connection\n");

    close(connfd);
}
