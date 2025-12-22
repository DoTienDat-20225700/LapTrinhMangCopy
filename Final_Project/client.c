#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include "protocol.h"
#include <arpa/inet.h>

#define PORT 1255
#define MAXLINE 4096

void clear_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int get_int(const char *prompt, int *value)
{
    printf("%s", prompt);
    if (scanf("%d", value) != 1)
    {
        // Nếu nhập sai (không phải số)
        printf("\n>>> Invalid input! Please enter a number.\n");

        // Xóa sạch bộ đệm để tránh trôi lệnh
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;

        return 0; // Báo thất bại
    }

    // Nếu nhập đúng, cũng cần xóa bộ đệm (ký tự \n còn sót)
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    return 1; // Báo thành công
}

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

void handle_authentication(int sockfd, struct sockaddr_in *servaddr, char *role_out)
{
    char buffer[MAXLINE], response[MAXLINE];
    char username[50], password[50];
    int choice;

    while (1)
    {
        printf("\nWelcome!\n1. Login\n2. Register\n3. Exit\n> ");
        scanf("%d", &choice);
        clear_buffer();

        if (choice == 1)
        {
            printf("Username: ");
            scanf("%s", username);
            printf("Password: ");
            scanf("%s", password);
            sprintf(buffer, "LOGIN username=%s password=%s", username, password);
            send_and_receive(sockfd, servaddr, buffer, response);

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
                printf("Failed to create account. Username exists.\n");
            }
        }
        else if (choice == 3)
        {
            sprintf(buffer, "EXIT");
            send_and_receive(sockfd, servaddr, buffer, response);
            printf("%s\n", response);
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
        printf("11. Exit\n");
        if (!get_int("> ", &choice))
            continue;

        if (choice == 11)
        {
            sprintf(buffer, "EXIT");
            send_and_receive(sockfd, servaddr, buffer, response);
            printf("%s\n", response);
            break;
        }

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
            if (!get_int("Duration (mins): ", &duration))
                continue;
            sprintf(buffer, "ADD_MOVIE title=\"%s\" genre=\"%s\" duration=%d", title, genre, duration);
            break;
        }
        case 2:
        { // Delete Movie
            int id;
            if (!get_int("Enter Movie ID to delete: ", &id))
                continue;
            sprintf(buffer, "DELETE_MOVIE id=%d", id);
            break;
        }
        case 3:
        { // Update Movie
            int id, new_duration;
            char new_genre[50];
            if (!get_int("Enter Movie ID to update: ", &id))
                continue;

            printf("New Genre: ");
            fgets(new_genre, sizeof(new_genre), stdin);
            new_genre[strcspn(new_genre, "\n")] = 0;

            if (!get_int("New Duration: ", &new_duration))
                continue;

            sprintf(buffer, "UPDATE_MOVIE id=%d new_genre=\"%s\" new_duration=%d", id, new_genre, new_duration);
            break;
        }
        case 4:
        { // Add Schedule
            int id;
            char day[20], time[100];

            if (!get_int("Enter Movie ID: ", &id))
                continue;

            printf("Day (e.g., Thu 2): ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = 0;

            printf("Time (e.g., 9h, 14h): ");
            fgets(time, sizeof(time), stdin);
            time[strcspn(time, "\n")] = 0;

            sprintf(buffer, "ADD_SCHEDULE id=%d day=\"%s\" time=\"%s\"", id, day, time);
            break;
        }
        case 5:
        { // Delete Schedule
            int id;
            char day[20], time[100];
            if (!get_int("Enter Movie ID: ", &id))
                continue;

            printf("Day (e.g., Thu 2): ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = 0;

            printf("Time to delete (e.g., 9h): ");
            fgets(time, sizeof(time), stdin);
            time[strcspn(time, "\n")] = 0;

            sprintf(buffer, "DELETE_SCHEDULE id=%d day=\"%s\" time=\"%s\"", id, day, time);
            break;
        }
        case 6:
        {
            strcpy(buffer, "LIST_MOVIES");
            break;
        }
        case 7:
        { // Reset Seatmap
            int id;
            char day[20], time[20];
            if (!get_int("Enter Movie ID: ", &id))
                continue;

            printf("Day: ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = 0;

            printf("Time: ");
            scanf("%s", time);

            sprintf(buffer, "RESET_SEATMAP id=%d day=\"%s\" time=%s", id, day, time);
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
        printf("%s\n", response);
    }
}

void show_user_menu(int sockfd, struct sockaddr_in *servaddr)
{
    int choice;
    char buffer[MAXLINE], response[MAXLINE];

    while (1)
    {
        printf("\n--- USER MENU ---\n");
        printf("1. Display all movies\n");
        printf("2. Search movie by name\n");
        printf("3. Filter movies\n");
        printf("4. Purchase tickets\n");
        printf("5. Exit\n");
        if (!get_int("> ", &choice))
            continue;
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
        {
            int filter_choice;
            if (!get_int("Filter options:\n1. By genre\n2. By time\n> ", &filter_choice))
                break;

            if (filter_choice == 1)
            {
                // 1. Lấy danh sách thể loại (Server đã in sẵn số 1, 2, 3...)
                strcpy(buffer, "LIST_GENRES");
                send_and_receive(sockfd, servaddr, buffer, response);
                printf("%s\n", response);

                // 2. Yêu cầu nhập số ID
                int genre_id;
                if (!get_int("Filtering genre ID: ", &genre_id))
                    break;

                // 3. Gửi ID lên server
                sprintf(buffer, "FILTER_GENRE id=%d", genre_id);
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
        }
        case 4:
        {
            int mid, col, book_choice;
            char row_input[10];
            char time[20], day[50];
            char ticket_info[MAXLINE];

            // Yêu cầu nhập ID phim
            if (!get_int("Enter Movie ID: ", &mid))
                break;

            printf("Enter day (e.g., Thu 2): ");
            fgets(day, sizeof(day), stdin);
            day[strcspn(day, "\n")] = 0;

            printf("Enter time (e.g., 7h): ");
            scanf("%s", time);
            clear_buffer();

            // Hiện sơ đồ ghế ban đầu
            sprintf(buffer, "GET_SEATMAP id=%d day=\"%s\" start=%s", mid, day, time);
            send_and_receive(sockfd, servaddr, buffer, response);
            printf("%s\n", response);

            if (strstr(response, "not found") || strstr(response, "Invalid"))
                break;

            do
            {
                // --- SỬA ĐỔI Ở ĐÂY ---
                // Không dùng get_int nữa mà dùng scanf để nhận cả chữ và số
                printf("Row (1-3 or A-C): ");
                scanf("%s", row_input);
                clear_buffer(); // Xóa bộ đệm sau khi nhập chuỗi

                if (!get_int("Col (1-5): ", &col))
                    break;

                // 1. Gửi lệnh Book vé
                // Lưu ý: row=%s (gửi chuỗi) thay vì row=%d
                sprintf(buffer, "BOOK_SEAT id=%d day=\"%s\" time=%s row=%s col=%d", mid, day, time, row_input, col);
                send_and_receive(sockfd, servaddr, buffer, ticket_info);

                // 2. Gửi lệnh lấy Seatmap mới để cập nhật hiển thị
                sprintf(buffer, "GET_SEATMAP id=%d day=\"%s\" start=%s", mid, day, time);
                send_and_receive(sockfd, servaddr, buffer, response);

                // 3. In Seatmap
                printf("Updated Seatmap:\n%s\n", response);

                // 4. In kết quả đặt vé
                printf("%s\n", ticket_info);

                if (!get_int("Do you want to book more?\n 1.Yes\n 2.No\n> ", &book_choice))
                    break;

            } while (book_choice == 1);
            break;
        }

        case 5:
            sprintf(buffer, "EXIT");
            send_and_receive(sockfd, servaddr, buffer, response);
            printf("%s\n", response);
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