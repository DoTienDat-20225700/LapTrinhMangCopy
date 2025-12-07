#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include "protocol.h"
#include "Movie/movie.h"
#include "Account/account.h"

#define PORT 1255
#define MAXLINE 4096

User users[MAX_USERS];
int user_count = 0;

Movie movie_cache[100];
int movie_count = 0;

void log_message(const char *direction, const char *msg)
{
    FILE *fp = fopen("log.txt", "a");
    if (!fp)
        return;
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0';
    fprintf(fp, "[%s] %s: %s\n", timestamp, direction, msg);
    fclose(fp);
}

void handle_message(Message msg, char *response_out)
{
    switch (msg.type)
    {
    case LOGIN:
        handle_login(msg.payload, response_out);
        break;
    case CREATE_ACCOUNT:
        handle_create_account(msg.payload, response_out);
        break;
    case LIST_MOVIES:
        handle_list_movies(response_out);
        break;
    case SEARCH:
        handle_search(msg.payload, response_out);
        break;
    case LIST_GENRES:
        handle_list_genres(response_out);
        break;
    case FILTER_GENRE:
        handle_filter_genre(msg.payload, response_out);
        break;
    case FILTER_TIME:
        handle_filter_time(msg.payload, response_out);
        break;
    case GET_SEATMAP:
        handle_get_seatmap(msg.payload, response_out);
        break;
    case BOOK_SEAT:
        handle_book_seat(msg.payload, response_out);
        break;

    case ADD_MOVIE:
        handle_add_movie(msg.payload, response_out);
        break;

    case DELETE_MOVIE:
        handle_delete_movie(msg.payload, response_out);
        break;

    case UPDATE_MOVIE:
        handle_update_movie(msg.payload, response_out);
        break;

    case ADD_SCHEDULE:
        handle_add_schedule(msg.payload, response_out);
        break;

    case DELETE_SCHEDULE:
        handle_delete_schedule(msg.payload, response_out);
        break;

    case RESET_SEATMAP:
        handle_reset_seatmap(msg.payload, response_out);
        break;

    case LIST_USERS:
        handle_list_users(response_out);
        break;

    case DELETE_USER:
        handle_delete_user(msg.payload, response_out);
        break;

    case SET_ROLE:
        handle_set_role(msg.payload, response_out);
        break;
    default:
        strcpy(response_out, "UNKNOWN_COMMAND");
    }
}

int main()
{
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server running on port %d...\n", PORT);

    movie_count = load_movies("movies.txt", movie_cache, 100);
    user_count = load_users("users.txt", users, MAX_USERS);
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    int maxfd = sockfd;

    while (1)
    {
        fd_set tempfds = readfds;
        int activity = select(maxfd + 1, &tempfds, NULL, NULL, NULL);

        if (activity < 0)
        {
            perror("select error");
            continue;
        }

        if (FD_ISSET(sockfd, &tempfds))
        {
            int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0';
            log_message("RECV", buffer);

            Message msg;
            parse_message(buffer, &msg);

            char reply[MAXLINE] = "";
            handle_message(msg, reply);

            sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr *)&cliaddr, len);
            log_message("SEND", reply);
        }
    }

    close(sockfd);
    return 0;
}
