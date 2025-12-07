#ifndef ACCOUNT_H
#define ACCOUNT_H

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_ROLE 20
#define MAX_USERS 100

typedef struct
{
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char role[MAX_ROLE]; // "admin" or "user"
} User;

// Core user database functions
int load_users(const char *filename, User *users, int max_users);
int save_user(const char *filename, const User *user);
int save_all_users(const char *filename, User *users, int count);
int username_exists(const char *username, User *users, int count);
int authenticate(const char *username, const char *password, User *users, int count, char *role_out);

// Server-side handlers
void handle_login(const char *payload, char *response_out);
void handle_create_account(const char *payload, char *response_out);
void handle_list_users(char *out);
void handle_delete_user(const char *payload, char *out);
void handle_set_role(const char *payload, char *out);
int find_user(const char *username);

#endif
