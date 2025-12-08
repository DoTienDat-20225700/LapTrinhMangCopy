#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include "protocol.h"
#include <arpa/inet.h>

#define PORT 1255
#define MAXLINE 4096

void send_and_receive(int sockfd, struct sockaddr_in *servaddr, const char *message, char *response)
{
    socklen_t len = sizeof(*servaddr);
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)servaddr, len);
    int n = recvfrom(sockfd, response, MAXLINE, 0, NULL, NULL);
    if (n < 0)
    {
        response[0] = '\0';
    }
    else
    {
        response[n] = '\0';
    }
}

// Chuyển từ int sang void để tránh warning
void handle_authentication(int sockfd, struct sockaddr_in *servaddr, char *role_out)
{
    char buffer[MAXLINE], response[MAXLINE];
    char username[50], password[50];
    int choice;
    int c; // Biến dùng để clear buffer

    while (1)
    {
        printf("\nWelcome!\n1. Login\n2. Register\n3. Exit\n> ");
        scanf("%d", &choice);
        while ((c = getchar()) != '\n' && c != EOF)
            ; // Clear buffer sau scanf

        if (choice == 1)
        {
            printf("Username: ");
            scanf("%s", username);
            printf("Password: ");
            scanf("%s", password);
            sprintf(buffer, "LOGIN username=%s password=%s", username, password);
            send_and_receive(sockfd, servaddr, buffer, response);

            // Server trả về: "SUCCESS admin" hoặc "SUCCESS user"
            if (strstr(response, "SUCCESS"))
            {
                printf("Login successful!\n");
                // Tách lấy role từ chuỗi response
                char *token = strtok(response, " "); // Lấy "SUCCESS"
                token = strtok(NULL, " ");           // Lấy role (ví dụ "admin" hoặc "user")
                if (token)
                {
                    strcpy(role_out, token);
                    // Xóa ký tự xuống dòng nếu có
                    role_out[strcspn(role_out, "\n")] = 0;
                    role_out[strcspn(role_out, "\r")] = 0;
                }
                else
                {
                    strcpy(role_out, "user"); // Mặc định nếu lỗi
                }
                break;
            }
            else
            {
                printf("Login failed.\n");
            }
        }
        else if (choice == 2)
        {
            printf("Choose a username: ");
            scanf("%s", username);
            printf("Choose a password: ");
            scanf("%s", password);
            sprintf(buffer, "CREATE_ACCOUNT username=%s password=%s role=user", username, password);
            send_and_receive(sockfd, servaddr, buffer, response);
            if (strstr(response, "SUCCESS"))
            {
                printf("Account created!\n");
            }
            else
            {
                printf("Failed to create account.\n");
            }
        }
        else if (choice == 3)
        {
            printf("Goodbye!\n");
            exit(0);
        }
        else
        {
            printf("Invalid choice. Try again.\n");
        }
    }
}

void show_admin_menu(int sockfd, struct sockaddr_in *servaddr)
{
    int choice;
    char buffer[MAXLINE], response[MAXLINE];
    int c;

    while (1)
    {
        printf("\n--- ADMIN MENU ---\n");
        printf("1. Add Movie\n");
        printf("2. Delete Movie\n");
        printf("3. Update Movie Info\n");
        printf("4. Add Schedule\n");
        printf("5. Delete Schedule\n");
        printf("6. View All Movies (Check)\n");
        printf("7. Reset Seatmap\n");
        printf("8. List Users\n");
        printf("9. Delete User\n");
        printf("10. Set User Role\n");
        printf("0. Exit\n> ");
        scanf("%d", &choice);
        while ((c = getchar()) != '\n' && c != EOF)
            ; // Clear buffer sau scanf

        if (choice == 0)
            break;

        switch (choice)
        {
        case 1:
        { // Add Movie
            char title[100], genre[50];
            int duration;
            printf("Title: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = 0;
            printf("Genre: ");
            fgets(genre, sizeof(genre), stdin);
            genre[strcspn(genre, "\n")] = 0;
            printf("Duration (mins): ");
            scanf("%d", &duration);
            sprintf(buffer, "ADD_MOVIE title=\"%s\" genre=\"%s\" duration=%d", title, genre, duration);
            break;
        }
        case 2:
        { // Delete Movie
            char title[100];
            printf("Title to delete: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = 0;
            sprintf(buffer, "DELETE_MOVIE title=\"%s\"", title);
            break;
        }
        case 3:
        { // Update Movie
            char title[100], new_genre[50];
            int new_duration;
            printf("Title to update: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = 0;
            printf("New Genre: ");
            fgets(new_genre, sizeof(new_genre), stdin);
            new_genre[strcspn(new_genre, "\n")] = 0;
            printf("New Duration: ");
            scanf("%d", &new_duration);
            sprintf(buffer, "UPDATE_MOVIE title=\"%s\" new_genre=\"%s\" new_duration=%d", title, new_genre, new_duration);
            break;
        }
        case 4:
        { // Add Schedule
            char title[100], day[20], time[100];
            printf("Title: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = 0;
            printf("Day (e.g., Thứ 2): ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = 0;
            printf("Time (e.g., 18h): ");
            fgets(time, sizeof(time), stdin);
            sprintf(buffer, "ADD_SCHEDULE title=\"%s\" day=\"%s\" time=\"%s\"", title, day, time);
            break;
        }
        case 5:
        { // Delete Schedule
            char title[100], day[20], time[100];
            printf("Title: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = 0;
            printf("Day (e.g., Thứ 2): ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = 0;
            printf("Time (e.g., 18h): ");
            fgets(time, sizeof(time), stdin);
            sprintf(buffer, "DELETE_SCHEDULE title=\"%s\" day=\"%s\" time=\"%s\"", title, day, time);
            break;
        }
        case 6:
        {
            strcpy(buffer, "LIST_MOVIES");
            break;
        }
        case 7:
        { // Reset Seatmap
            char title[100], day[20], time[10];
            printf("Title: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = 0;
            printf("Day: ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = 0;
            printf("Time: ");
            scanf("%s", time);
            sprintf(buffer, "RESET_SEATMAP title=\"%s\" day=\"%s\" time=%s", title, day, time);
            break;
        }
        case 8:
        { // List Users
            strcpy(buffer, "LIST_USERS");
            break;
        }
        case 9:
        { // Delete User
            char username[50];
            printf("Username to delete: ");
            scanf("%s", username);
            sprintf(buffer, "DELETE_USER username=%s", username);
            break;
        }
        case 10:
        { // Set Role
            char username[50], role[20];
            printf("Username: ");
            scanf("%s", username);
            printf("New Role (admin/user): ");
            scanf("%s", role);
            sprintf(buffer, "SET_ROLE username=%s role=%s", username, role);
            break;
        }
        default:
            printf("Invalid choice.\n");
            continue;
        }

        send_and_receive(sockfd, servaddr, buffer, response);
        printf("Server: %s\n", response);
    }
}

void show_user_menu(int sockfd, struct sockaddr_in *servaddr)
{
    int choice;
    char buffer[MAXLINE], response[MAXLINE];
    int c;

    while (1)
    {
        printf("\n--- USER MENU ---\n");
        printf("1. Display all movies\n");
        printf("2. Search movie by name\n");
        printf("3. Filter movies\n");
        printf("4. Purchase tickets\n");
        printf("5. Exit\n> ");
        scanf("%d", &choice);
        while ((c = getchar()) != '\n' && c != EOF)
            ; // Clear buffer

        switch (choice)
        {
        case 1:
            strcpy(buffer, "LIST_MOVIES");
            send_and_receive(sockfd, servaddr, buffer, response);
            printf("%s\n", response);
            break;

        case 2:
        {
            char title[100];
            printf("Enter movie name: ");
            if (fgets(title, sizeof(title), stdin))
            {
                title[strcspn(title, "\n")] = '\0';
            }
            sprintf(buffer, "SEARCH title=%s", title);
            send_and_receive(sockfd, servaddr, buffer, response);
            printf("%s\n", response);
            break;
        }

        case 3:
            printf("Filter options:\n1. By genre\n2. By time\n> ");
            int filter_choice;
            scanf("%d", &filter_choice);
            while ((c = getchar()) != '\n' && c != EOF)
                ; // flush newline

            if (filter_choice == 1)
            {
                strcpy(buffer, "LIST_GENRES");
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("Available genres:\n%s\n", response);

                char genre[50];
                printf("Enter genre: ");
                fgets(genre, sizeof(genre), stdin);
                genre[strcspn(genre, "\n")] = '\0';
                printf("Filtering genre: %s\n", genre);
                sprintf(buffer, "FILTER_GENRE genre=\"%s\"", genre);
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("%s\n", response);
            }
            else if (filter_choice == 2)
            {
                char begin[10], end[10], day[20];
                printf("Enter day (e.g., Thứ 2): ");
                fgets(day, sizeof(day), stdin);
                day[strcspn(day, "\n")] = '\0';
                printf("Enter begin time (e.g., 12h): ");
                scanf("%s", begin);
                printf("Enter end time (e.g., 22h): ");
                scanf("%s", end);
                sprintf(buffer, "FILTER_TIME day=\"%s\" begin=%s end=%s", day, begin, end);
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("%s\n", response);
            }
            else
            {
                printf("Invalid filter option.\n");
            }
            break;

        case 4:
        {
            char title[100], time[10], day[20];
            printf("Enter movie title: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = '\0';

            printf("Enter day (e.g., Thứ 2): ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = '\0';

            printf("Enter start time (e.g., 12h): ");
            scanf("%s", time);
            while ((c = getchar()) != '\n' && c != EOF)
                ; // flush after scanf

            // Request seatmap once
            sprintf(buffer, "GET_SEATMAP title=\"%s\" day=\"%s\" start=%s", title, day, time);
            send_and_receive(sockfd, servaddr, buffer, response);

            // Kiểm tra nếu server báo lỗi (không tìm thấy phim/giờ/ngày)
            if (strstr(response, "not found") != NULL || strstr(response, "Invalid") != NULL)
            {
                printf("Server Error: %s\n", response);
                break;
            }

            printf("Seatmap:\n%s\n", response);
            printf("|x| = booked, | | = available\n");

            int book_choice;
            do
            {
                int row, col;
                printf("Choose seat:\nRow (1-3): ");
                scanf("%d", &row);
                printf("Column (1-5): ");
                scanf("%d", &col);
                while ((c = getchar()) != '\n' && c != EOF)
                    ;

                sprintf(buffer, "BOOK_SEAT title=\"%s\" day=\"%s\" time=%s row=%d col=%d", title, day, time, row, col);
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("%s\n", response);

                // Show updated seatmap
                sprintf(buffer, "GET_SEATMAP title=\"%s\" day=\"%s\" start=%s", title, day, time);
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("Updated Seatmap:\n%s\n", response);

                printf("Do you want to book another seat?\n");
                printf("1. Yes\n");
                printf("2. No\n");
                printf("> ");
                scanf("%d", &book_choice);
                while ((c = getchar()) != '\n' && c != EOF)
                    ;

            } while (book_choice == 1);

            break;
        }

        case 5:
            printf("Goodbye!\n");
            return;

        default:
            printf("Invalid choice. Try again.\n");
        }
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    // servaddr.sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    char role[20] = "";
    handle_authentication(sockfd, &servaddr, role);

    printf("Logged in as: %s\n", role);
    if (strcmp(role, "admin") == 0 || strcmp(role, "role=admin") == 0)
    {
        show_admin_menu(sockfd, &servaddr);
    }
    else
    {
        show_user_menu(sockfd, &servaddr);
    }

    close(sockfd);
    return 0;
}