#include <stdio.h> /* These are the usual header files */
#include <jansson.h>
#include <string.h>

#include <mysql/mysql.h>
const char *db_name = "socket";
const char *query_create_db = "CREATE DATABASE IF NOT EXISTS socket";
const char *query_drop_db = "DROP DATABASE IF EXISTS socket";
const char *query_use_db = "USE socket";
const char *query_create_table_1 = "CREATE TABLE user (id int PRIMARY KEY AUTO_INCREMENT, username varchar(255) UNIQUE,thread_address varchar(255), password varchar(55), name varchar(50), age int,phone varchar(25), address varchar(255) );";
const char *query_create_table_2 = "CREATE TABLE friend (user1 INT, user2 INT, FOREIGN KEY (user1) REFERENCES user(id), FOREIGN KEY (user2) REFERENCES user(id));";
const char *query_create_table_3 = "CREATE TABLE locationType (id int PRIMARY KEY AUTO_INCREMENT, name varchar(125));";
const char *query_create_table_4 = "CREATE TABLE location (id int PRIMARY KEY AUTO_INCREMENT,createdBy int, name varchar(255), type int, address varchar(255), FOREIGN KEY (createdBy) REFERENCES user(id), FOREIGN KEY (type) REFERENCES locationType(id));";
const char *query_create_table_5 = "CREATE TABLE review (id int PRIMARY KEY AUTO_INCREMENT,createdBy int,locationId int,content varchar(1000),FOREIGN KEY (createdBy) REFERENCES user(id),FOREIGN KEY (locationId) REFERENCES location(id));";
const char *query_create_table_6 = "CREATE TABLE saveLocation (id int PRIMARY KEY AUTO_INCREMENT, locationId int, userId int, FOREIGN KEY (userId) REFERENCES user(id), FOREIGN KEY (locationId) REFERENCES location(id));";
const char *query_create_table_7 = "CREATE TABLE favoriteLocation (id int PRIMARY KEY AUTO_INCREMENT,locationId int,userId int,FOREIGN KEY (userId) REFERENCES user(id),FOREIGN KEY (locationId) REFERENCES location(id));";
const char *insert_query_1 = "INSERT INTO user (username, name, password, age, phone, address) VALUES ('tien','Tien Chuot', '123456', 23, '0852250815', '1 Dai CO Viet, Ha Noi'), ('ngoctu','Nguyen Ngoc Tu', '123456', 22,  '012345654', 'Hai Ba Trung');";
const char *insert_query_2 = "INSERT INTO friend (user1, user2) VALUES (1, 2),(2, 1);";
const char *insert_query_3 = "INSERT INTO locationType (name) VALUES ('School'), ('Coffe'), ('Restaurant'), ('Park'), ('Mall'), ('Market'),('Hospital'),('Others');";
const char *insert_query_4 = "INSERT INTO location (createdBy, name, type, address) VALUES (1, 'Dai hoc Bach Khoa Ha Noi', 1, '1 Dai Co Viet, Hai Ba Trung, Ha Noi'), (1, 'Gongtea', 2, '23 Vu Trong Phung, Thanh Xuan, Ha Noi'), (2, 'Lau Phan', 3, '15 Pho Hue, Hai Ba Trung, Ha Noi'), (1, 'Dong Xuan', 6, 'Hang Buom, Hoan Kiem, Ha Noi'), (2, 'Bach Mai', 7, '15 Giai Phong, Hai Ba Trung, Ha Noi');";
const char *insert_query_5 = "INSERT INTO review (createdBy, locationId, content) VALUES (2, 1, 'Rat tuyet voi, sinh vien than thien, canh quan truong rat nhieu cay xanh'), (1, 3, 'Quan an rat ngon, gia ca hop ly'), (2, 3, 'Quan te, nhan vien phuc vu kem');";
const char *insert_query_6 = "INSERT INTO saveLocation (locationId, userId) VALUES (2, 1), (5, 1), (3, 2);";
const char *insert_query_7 = "INSERT INTO favoriteLocation (locationId, userId) VALUES (1, 1),(3, 1),(4, 2);";
char *getQuerySQL(char *key, char *json_str)
{
    printf("%s\n", key);
    printf("%s\n", json_str);
    char *username = (char *)malloc(150 * sizeof(char));
    char *password = (char *)malloc(150 * sizeof(char));
    char *string = (char *)malloc(150 * sizeof(char));
    int userId;
    int locationId;
    int number;
    char *temp = malloc(sizeof(char) * 250);
    json_error_t error;
    json_t *root = json_loads(json_str, 0, &error);
    if (!root)
    {
        fprintf(stderr, "JSON parsing error: %s\n", error.text);
        return NULL;
    }
    char numStr[10];
    if (strcmp(key, "REQ_LOGI") == 0)
    { // Login account
        // username and password
        username = json_string_value(json_object_get(root, "username"));
        password = json_string_value(json_object_get(root, "password"));
        strcpy(temp, "");
        strcat(temp, "SELECT * from user WHERE username = '");
        strcat(temp, username);
        strcat(temp, "' AND password = '");
        strcat(temp, password);
        strcat(temp, "';");
    }
    else if (strcmp(key, "REQ_REGI") == 0)
    {
        // Register account
        // username, password
        username = json_string_value(json_object_get(root, "username"));
        password = json_string_value(json_object_get(root, "password"));
        strcpy(temp, "");
        strcat(temp, "INSERT INTO user (username, password) VALUES ('");
        strcat(temp, username);
        strcat(temp, "', '");
        strcat(temp, password);
        strcat(temp, "');");
    }
    else if (strcmp(key, "REQ_CPAS") == 0)
    {
        // userId, oldpassword, newpassword
        userId = json_integer_value(json_object_get(root, "id"));
        password = json_string_value(json_object_get(root, "newpassword"));
        string = json_string_value(json_object_get(root, "oldpassword"));
        printf("Password:%s\n", password);
        printf("String: %s\n", string);
        strcpy(temp, "");
        strcat(temp, "UPDATE user SET password = '");
        strcat(temp, password);
        strcat(temp, "' WHERE id= ");
        sprintf(numStr, "%d", userId);
        strcat(temp, numStr);
        strcat(temp, " AND password='");
        strcat(temp, string);
        strcat(temp, "';");
    }
    else if (strcmp(key, "REQ_LOCA") == 0)
    {
        // Requets get location infor
        // locationId
        locationId = json_integer_value(json_object_get(root, "locationId"));
        strcpy(temp, "");
        strcat(temp, "Select location.id, location.name, location.type, location.address, review.content, user.name from location join review on review.locationId = location.id join user ON review.createdBy= user.id  WHERE location.id =");
        sprintf(numStr, "%d", locationId);
        strcat(temp, numStr);
        strcat(temp, ";");
    }
    else if (strcmp(key, "REQ_CDET") == 0)
    {
        // Change user infor
        //  userId, name, address, phone, age
        strcpy(temp, "");
        strcat(temp, "UPDATE user SET name ='");
        username = json_string_value(json_object_get(root, "name"));
        strcat(temp, username);
        strcat(temp, "', phone='");
        string = json_string_value(json_object_get(root, "phone"));
        strcat(temp, string);
        strcat(temp, "', address='");
        string = json_string_value(json_object_get(root, "address"));
        strcat(temp, string);
        strcat(temp, "', age=");
        number = json_integer_value(json_object_get(root, "age"));
        sprintf(numStr, "%d", number);
        strcat(temp, numStr);
        strcat(temp, " WHERE id=");
        userId = json_integer_value(json_object_get(root, "userId"));
        sprintf(numStr, "%d", userId);
        strcat(temp, numStr);
        strcat(temp, ";");
    }
    else if (strcmp(key, "PUT_SHLC") == 0)
    {
        // Put the share location
        // userId, name, type, address
        strcpy(temp, "");
        userId = json_integer_value(json_object_get(root, "userId"));
        sprintf(numStr, "%d", userId);
        strcat(temp, "INSERT INTO location(createdBy, name, type, address) VALUES (");
        strcat(temp, numStr);
        strcat(temp, ", '");
        string = json_string_value(json_object_get(root, "name"));
        strcat(temp, string);
        strcat(temp, "',");
        number = json_integer_value(json_object_get(root, "type"));
        sprintf(numStr, "%d", number);
        strcat(temp, numStr);
        strcat(temp, ", '");
        string = json_string_value(json_object_get(root, "address"));
        strcat(temp, string);
        strcat(temp, "');");
    }
    else if (strcmp(key, "PUT_RVIE") == 0)
    {
        // Push comment
        // userId, locationId, content
        strcpy(temp, "");
        userId = json_integer_value(json_object_get(root, "userId"));
        sprintf(numStr, "%d", userId);
        strcat(temp, "INSERT INTO review(createdBy, locationId, content) VALUES (");
        strcat(temp, numStr);
        strcat(temp, ",");
        number = json_integer_value(json_object_get(root, "locationId"));
        sprintf(numStr, "%d", number);
        strcat(temp, numStr);
        strcat(temp, ",'");
        string = json_string_value(json_object_get(root, "content"));
        strcat(temp, string);
        strcat(temp, "');");
    }
    else if (strcmp(key, "GET_FRIE") == 0)
    {
        // Get list of friend
        // userId
        strcpy(temp, "");
        userId = json_integer_value(json_object_get(root, "userId"));
        sprintf(numStr, "%d", userId);
        strcat(temp, "SELECT id, name, age, phone, address  FROM friend JOIN user on friend.user2 = user.id WHERE friend.user1 =");
        strcat(temp, numStr);
        strcat(temp, ";");
    }
    else if (strcmp(key, "GET_SLOC") == 0)
    {
        // GET save location
        //  userId
        strcpy(temp, "");
        userId = json_integer_value(json_object_get(root, "userId"));
        sprintf(numStr, "%d", userId);
        strcat(temp, "SELECT location.id, name, type, address FROM location JOIN saveLocation ON location.id  = saveLocation.locationId WHERE saveLocation.userId = ");
        strcat(temp, numStr);
        strcat(temp, ";");
    }
    else if (strcmp(key, "GET_FLOC") == 0)
    {
        // GET favorite location
        //  userId
        strcpy(temp, "");
        userId = json_integer_value(json_object_get(root, "userId"));
        sprintf(numStr, "%d", userId);
        strcat(temp, "SELECT location.id, name, type, address FROM location JOIN favoriteLocation ON location.id  = favoriteLocation.locationId WHERE favoriteLocation.userId = ");
        strcat(temp, numStr);
        strcat(temp, ";");
    }
    else if(strcmp(key, "GET_USER") == 0){
        strcpy(temp, "");
        userId = json_integer_value(json_object_get(root, "userId"));
        sprintf(numStr, "%d", userId);
        strcat(temp, "SELECT id, name, age, phone, address FROM user WHERE id = ");
        strcat(temp, numStr);
        strcat(temp, ";");
    }
    else
    {
        printf("Not messeage type detected!\n");
    }

    free(string);
    free(password);
    free(username);

    printf("Temp=%s\n", temp);
    printf("Het ham connect\n");
    // free(temp);
    return temp;
}

MYSQL_RES *selectQuery(MYSQL *connection, char *query)
{
    MYSQL_RES *resultQuery;
    printf("Select query function:\n");
    printf("%s\n", query);
    if (mysql_query(connection, query))
    {
        fprintf(stderr, "Failed to execute SELECT query: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 1;
    }
    resultQuery = mysql_store_result(connection);
    if (resultQuery == NULL)
    {
        fprintf(stderr, "Failed to retrieve result set: %s\n", mysql_error(connection));
        mysql_close(connection);
    }
    return resultQuery;
}
long updateQuery(MYSQL *connection, char *query){
    long affected_rows;
    if (mysql_query(connection, query))
    {
        fprintf(stderr, "Failed to execute UPDATE query: %s\n", mysql_error(connection));
        mysql_close(connection);
        return 0;
    }
    affected_rows = mysql_affected_rows(connection);
    return affected_rows;
}

