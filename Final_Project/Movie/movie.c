#define _GNU_SOURCE

#include "movie.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

extern Movie movie_cache[100];
extern int movie_count;

// --- CÁC HÀM TIỆN ÍCH (HELPER FUNCTIONS) ---

// Hàm phụ trợ: Lấy tên ngày từ index (để lưu vào file)
const char *get_day_name(int index)
{
    switch (index)
    {
    case 0:
        return "Thứ 2";
    case 1:
        return "Thứ 3";
    case 2:
        return "Thứ 4";
    case 3:
        return "Thứ 5";
    case 4:
        return "Thứ 6";
    case 5:
        return "Thứ 7";
    case 6:
        return "Chủ Nhật";
    default:
        return "Unknown";
    }
}

// Hàm trả về 1 nếu giống nhau(không phân biệt hoa thường), trả về 0 nếu khác nhau
int string_equals_ignore_case(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        // So sánh từng ký tự sau khi chuyển về chữ thường
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2))
        {
            return 0; // Khác nhau
        }
        s1++;
        s2++;
    }
    // Kiểm tra xem cả 2 chuỗi đã kết thúc cùng lúc chưa (độ dài bằng nhau)
    return *s1 == *s2;
}

// Hàm xóa khoảng trắng đầu và cuối chuỗi
void trim(char *s)
{
    char *p = s;
    int l = strlen(p);

    while (l > 0 && isspace(p[l - 1]))
        p[--l] = 0;
    while (*p && isspace(*p))
        ++p, --l;

    memmove(s, p, l + 1);
}

// Hàm tìm chuỗi con không phân biệt hoa thường
char *stristr(const char *haystack, const char *needle)
{
    if (!*needle)
        return (char *)haystack;
    for (; *haystack; ++haystack)
    {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle))
        {
            const char *h, *n;
            for (h = haystack, n = needle; *h && *n; ++h, ++n)
            {
                if (tolower((unsigned char)*h) != tolower((unsigned char)*n))
                    break;
            }
            if (!*n)
                return (char *)haystack;
        }
    }
    return NULL;
}

// Hàm ánh xạ ngày (Thứ 2 -> index 0) - Không phân biệt hoa thường
int map_day_to_index(const char *day)
{
    if (strcasecmp(day, "Thứ 2") == 0)
        return 0;
    if (strcasecmp(day, "Thứ 3") == 0)
        return 1;
    if (strcasecmp(day, "Thứ 4") == 0)
        return 2;
    if (strcasecmp(day, "Thứ 5") == 0)
        return 3;
    if (strcasecmp(day, "Thứ 6") == 0)
        return 4;
    if (strcasecmp(day, "Thứ 7") == 0)
        return 5;
    if (strcasecmp(day, "Chủ nhật") == 0)
        return 6;
    return -1;
}

// Hàm chuẩn hóa giờ (ví dụ "7" -> "07h")
void normalize_time(char *out, const char *in)
{
    int hour;
    if (sscanf(in, "%dh", &hour) == 1)
    {
        sprintf(out, "%02dh", hour);
    }
    else
    {
        out[0] = '\0';
    }
}

// --- CÁC HÀM TÌM KIẾM CORE ---
int find_movie_by_id(int id)
{
    for (int i = 0; i < movie_count; i++)
    {
        if (movie_cache[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

int find_movie_by_title(const char *title)
{
    for (int i = 0; i < movie_count; i++)
    {
        // SỬA: Dùng hàm mới thay vì strcmp
        if (string_equals_ignore_case(movie_cache[i].title, title))
        {
            return i; // Tìm thấy
        }
    }
    return -1; // Không tìm thấy
}

int find_time_slot(int movie_index, int day_index, const char *time)
{
    int u_time = atoi(time); // Chuyển input user thành số (vd: "7" -> 7)
    for (int i = 0; i < MAX_SLOTS; i++)
    {
        char *db_time_str = movie_cache[movie_index].schedule[day_index][i];
        if (strlen(db_time_str) == 0)
            continue;

        int db_time = atoi(db_time_str); // Chuyển giờ DB thành số (vd: "7h" -> 7)
        if (db_time == u_time)
        {
            return i;
        }
    }
    return -1;
}

// --- CÁC HÀM FILE I/O ---

int load_movies(const char *filename, Movie *movies, int max_movies)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        perror("Cannot open file");
        return 0;
    }

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max_movies)
    {
        line[strcspn(line, "\r\n")] = '\0';

        char *saveptr1;
        char *id_str = strtok_r(line, "|", &saveptr1);
        char *title = strtok_r(line, "|", &saveptr1);
        char *genre = strtok_r(NULL, "|", &saveptr1);
        char *duration_str = strtok_r(NULL, "|", &saveptr1);
        char *schedule_str = strtok_r(NULL, "", &saveptr1);

        if (!id_str || !title || !genre || !duration_str || !schedule_str)
            continue;
        movies[count].id = atoi(id_str);

        strncpy(movies[count].title, title, sizeof(movies[count].title) - 1);
        strncpy(movies[count].genre, genre, sizeof(movies[count].genre) - 1);
        movies[count].title[sizeof(movies[count].title) - 1] = '\0';
        movies[count].genre[sizeof(movies[count].genre) - 1] = '\0';
        movies[count].duration = atoi(duration_str);

        // Reset schedule & seatmap
        for (int d = 0; d < MAX_DAYS; d++)
        {
            for (int s = 0; s < MAX_SLOTS; s++)
            {
                movies[count].schedule[d][s][0] = '\0';
                // Mặc định ghế trống (' ')
                memset(movies[count].seatmap[d][s], ' ', sizeof(movies[count].seatmap[d][s]));
            }
        }

        if (schedule_str && strlen(schedule_str) > 0)
        {
            char schedule_copy[2048];
            strncpy(schedule_copy, schedule_str, sizeof(schedule_copy) - 1);
            schedule_copy[sizeof(schedule_copy) - 1] = '\0';
            char *saveptr2;
            char *day_block = strtok_r(schedule_copy, ";", &saveptr2);
            while (day_block)
            {
                char *colon = strchr(day_block, ':');
                if (colon)
                {
                    *colon = '\0';
                    int d_idx = map_day_to_index(day_block);
                    if (d_idx >= 0)
                    {
                        char *times = colon + 1;
                        char *saveptr3;
                        char *t = strtok_r(times, ",", &saveptr3);
                        int s_idx = 0;
                        while (t && s_idx < MAX_SLOTS)
                        {
                            while (*t == ' ')
                                t++;
                            if (strlen(t) > 0)
                            {
                                strncpy(movies[count].schedule[d_idx][s_idx], t, 9);
                                s_idx++;
                            }
                            t = strtok_r(NULL, ",", &saveptr3);
                        }
                    }
                }
                day_block = strtok_r(NULL, ";", &saveptr2);
            }
        }

        count++;
    }
    fclose(fp);
    return count;
}

int save_all_movies(const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
        return 0;

    for (int i = 0; i < movie_count; i++)
    {
        Movie *m = &movie_cache[i];
        fprintf(fp, "%d|%s|%s|%d|", m->id, m->title, m->genre, m->duration);

        int has_schedule = 0;
        for (int d = 0; d < MAX_DAYS; d++)
        {
            int has_day = 0;
            for (int s = 0; s < MAX_SLOTS; s++)
            {
                if (strlen(m->schedule[d][s]) > 0)
                {
                    if (!has_day)
                    {
                        if (has_schedule)
                            fprintf(fp, ";");
                        fprintf(fp, "%s:", get_day_name(d));
                        has_day = 1;
                        has_schedule = 1;
                    }
                    else
                    {
                        fprintf(fp, ",");
                    }
                    fprintf(fp, "%s", m->schedule[d][s]);
                }
            }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return 1;
}

// Hàm lưu toàn bộ ghế đã đặt ra file bookings.txt
void save_bookings()
{
    FILE *fp = fopen("bookings.txt", "w");
    if (!fp)
        return;

    for (int i = 0; i < movie_count; i++)
    {
        Movie *m = &movie_cache[i];
        for (int d = 0; d < MAX_DAYS; d++)
        {
            for (int s = 0; s < MAX_SLOTS; s++)
            {
                // Nếu không có lịch chiếu thì bỏ qua
                if (strlen(m->schedule[d][s]) == 0)
                    continue;

                // Quét ma trận ghế 3x5
                for (int r = 0; r < 3; r++)
                {
                    for (int c = 0; c < 5; c++)
                    {
                        if (m->seatmap[d][s][r][c] == 'x')
                        {
                            // Format: Tên Phim|Ngày|Giờ|Hàng|Cột
                            fprintf(fp, "%s|%s|%s|%d|%d\n",
                                    m->title, get_day_name(d), m->schedule[d][s], r + 1, c + 1);
                        }
                    }
                }
            }
        }
    }
    fclose(fp);
}

// Hàm đọc file bookings.txt và đánh dấu ghế x vào RAM
void load_bookings()
{
    FILE *fp = fopen("bookings.txt", "r");
    if (!fp)
        return; // Nếu chưa có file thì thôi

    char line[512];
    while (fgets(line, sizeof(line), fp))
    {
        char title[100], day[50], time[20];
        int row, col;

        // Xóa xuống dòng
        line[strcspn(line, "\r\n")] = 0;

        // Parse dữ liệu: Title|Day|Time|Row|Col
        // Dùng sscanf cẩn thận với chuỗi có dấu cách
        char *token = strtok(line, "|");
        if (!token)
            continue;
        strcpy(title, token);

        token = strtok(NULL, "|");
        if (!token)
            continue;
        strcpy(day, token);
        token = strtok(NULL, "|");
        if (!token)
            continue;
        strcpy(time, token);
        token = strtok(NULL, "|");
        if (!token)
            continue;
        row = atoi(token);
        token = strtok(NULL, "|");
        if (!token)
            continue;
        col = atoi(token);

        // Tìm lại vị trí trong RAM để tick 'x'
        int mi = find_movie_by_title(title);
        if (mi >= 0)
        {
            int di = map_day_to_index(day);
            if (di >= 0)
            {
                int ti = find_time_slot(mi, di, time);
                if (ti >= 0)
                {
                    if (row >= 1 && row <= 3 && col >= 1 && col <= 5)
                    {
                        movie_cache[mi].seatmap[di][ti][row - 1][col - 1] = 'x';
                    }
                }
            }
        }
    }
    fclose(fp);
}

// --- CÁC HÀM XỬ LÝ (HANDLERS) ---

void handle_list_movies(char *response_out)
{
    strcpy(response_out, "");
    for (int i = 0; i < movie_count; i++)
    {
        char line[256];
        sprintf(line, "[ID: %d] Title: %s\nGenre: %s\nDuration: %d mins\n\n",
                movie_cache[i].id, movie_cache[i].title, movie_cache[i].genre, movie_cache[i].duration);
        if (strlen(response_out) + strlen(line) < MAXLINE)
            strcat(response_out, line);
    }
}

void handle_search(const char *payload, char *response_out)
{
    char title[MAX_TITLE];
    sscanf(payload, "SEARCH title=%[^\n]", title);

    strcpy(response_out, "");
    int found_count = 0;

    for (int i = 0; i < movie_count; i++)
    {
        if (stristr(movie_cache[i].title, title) != NULL)
        {
            char match_info[MAXLINE];
            sprintf(match_info, "Title: %s\nGenre: %s\nDuration: %d mins\n\n",
                    movie_cache[i].title, movie_cache[i].genre, movie_cache[i].duration);
            if (strlen(response_out) + strlen(match_info) < MAXLINE)
            {
                strcat(response_out, match_info);
                found_count++;
            }
        }
    }
    if (found_count == 0)
    {
        strcpy(response_out, "NOT_FOUND");
    }
}

void handle_list_genres(char *response_out)
{
    char genres[100][MAX_GENRE];
    int count = 0;
    strcpy(response_out, "");

    for (int i = 0; i < movie_count; i++)
    {
        char temp_genre[MAX_GENRE];
        strcpy(temp_genre, movie_cache[i].genre);
        char *token = strtok(temp_genre, ",");
        while (token != NULL)
        {
            trim(token);
            int found = 0;
            for (int j = 0; j < count; j++)
            {
                if (strcmp(genres[j], token) == 0)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                strcpy(genres[count++], token);
            }
            token = strtok(NULL, ",");
        }
    }
    for (int i = 0; i < count; i++)
    {
        if (strlen(response_out) + strlen(genres[i]) + 2 < MAXLINE)
        {
            strcat(response_out, genres[i]);
            strcat(response_out, "\n");
        }
    }
}

void handle_filter_genre(const char *payload, char *response_out)
{
    char genre[MAX_GENRE];
    sscanf(payload, "FILTER_GENRE genre=\"%[^\"]\"", genre);

    printf("Filtering genre: %s\n", genre);
    strcpy(response_out, "");
    int found_count = 0;
    for (int i = 0; i < movie_count; i++)
    {
        if (stristr(movie_cache[i].genre, genre) != NULL)
        {
            char line[MAXLINE];
            sprintf(line, "Title: %s\nDuration: %d mins\n\n",
                    movie_cache[i].title, movie_cache[i].duration);

            if (strlen(response_out) + strlen(line) < MAXLINE)
            {
                strcat(response_out, line);
                found_count++;
            }
        }
    }
    if (found_count == 0)
    {
        strcpy(response_out, "No movies found for this genre.");
    }
}

void handle_filter_time(const char *payload, char *response_out)
{
    char day[50], begin_str[10], end_str[10];

    // Parse input
    sscanf(payload, "FILTER_TIME day=\"%[^\"]\" begin=%s end=%s", day, begin_str, end_str);

    int day_index = map_day_to_index(day);
    if (day_index == -1)
    {
        sprintf(response_out, "Invalid day input: %s", day);
        return;
    }

    int u_begin = atoi(begin_str);
    int u_end = atoi(end_str);

    printf("Filtering time: Day=%s, Range: %dh - %dh\n", day, u_begin, u_end);

    strcpy(response_out, "");
    int found_any_movie = 0;

    for (int i = 0; i < movie_count; i++)
    {
        char time_list[MAXLINE] = "";
        int found_slots = 0;
        for (int s = 0; s < MAX_SLOTS; s++)
        {
            char *time_str = movie_cache[i].schedule[day_index][s];
            if (strlen(time_str) == 0)
                continue;
            int m_start_h = atoi(time_str);
            if (m_start_h >= u_begin && m_start_h <= u_end)
            {
                int duration = movie_cache[i].duration;
                int m_end_h = m_start_h + (duration / 60);
                int m_end_m = duration % 60;
                char slot_info[50];
                sprintf(slot_info, "%02dh00 - %02dh%02d", m_start_h, m_end_h, m_end_m);

                if (found_slots > 0)
                    strcat(time_list, ", ");
                strcat(time_list, slot_info);

                found_slots++;
            }
        }
        if (found_slots > 0)
        {
            char movie_entry[MAXLINE];
            sprintf(movie_entry, "Title: %s\nGenre: %s\nDuration: %d mins\nSchedule: %s\n\n",
                    movie_cache[i].title,
                    movie_cache[i].genre,
                    movie_cache[i].duration,
                    time_list);
            if (strlen(response_out) + strlen(movie_entry) < MAXLINE)
            {
                strcat(response_out, movie_entry);
                found_any_movie++;
            }
        }
    }
    if (found_any_movie == 0)
    {
        strcpy(response_out, "No movies found in this time range.");
    }
}

void handle_get_seatmap(const char *payload, char *response_out)
{
    int movie_id;
    char day[50], time[20];

    // Parse input: hỗ trợ dấu ngoặc kép
    sscanf(payload, "GET_SEATMAP id=%d day=\"%[^\"]\" start=%s", &movie_id, day, time);
    int movie_index = find_movie_by_id(movie_id);
    if (movie_index == -1)
    {
        sprintf(response_out, "Movie ID not found: %d", movie_id);
        return;
    }

    int day_index = map_day_to_index(day);
    if (day_index < 0 || day_index >= MAX_DAYS)
    {
        sprintf(response_out, "Invalid day: %s", day);
        return;
    }

    int time_index = find_time_slot(movie_index, day_index, time);
    if (time_index == -1)
    {
        sprintf(response_out, "Time slot not found: %s", time);
        return;
    }

    strcpy(response_out, "");
    for (int r = 0; r < 3; r++)
    {
        strcat(response_out, "|");
        for (int c = 0; c < 5; c++)
        {
            char seat[10];
            sprintf(seat, "%c|", movie_cache[movie_index].seatmap[day_index][time_index][r][c] == 'x' ? 'x' : ' ');
            strcat(response_out, seat);
        }
        strcat(response_out, "\n");
    }
}

void handle_book_seat(const char *payload, char *response_out)
{
    int row, col, movie_id;
    char day[50], time[20];

    // Parse input: hỗ trợ dấu ngoặc kép
    sscanf(payload, "BOOK_SEAT id=%d day=\"%[^\"]\" time=%s row=%d col=%d", &movie_id, day, time, &row, &col);

    int movie_index = find_movie_by_id(movie_id);
    if (movie_index == -1)
    {
        sprintf(response_out, "Movie ID not found: %d", movie_id);
        return;
    }

    int day_index = map_day_to_index(day);
    if (day_index < 0 || day_index >= MAX_DAYS)
    {
        sprintf(response_out, "Invalid day: %s", day);
        return;
    }

    int time_index = find_time_slot(movie_index, day_index, time);
    if (time_index == -1)
    {
        sprintf(response_out, "Time slot not found: %s", time);
        return;
    }

    if (row < 1 || row > 3 || col < 1 || col > 5)
    {
        sprintf(response_out, "Invalid seat position (Row 1-3, Col 1-5)");
        return;
    }

    if (movie_cache[movie_index].seatmap[day_index][time_index][row - 1][col - 1] == 'x')
    {
        sprintf(response_out, "Failed: Seat (%d,%d) is already booked!", row, col);
        return;
    }

    // Đặt ghế
    movie_cache[movie_index].seatmap[day_index][time_index][row - 1][col - 1] = 'x';

    // Lưu lại file
    save_bookings();

    sprintf(response_out, "SUCCESS: Booked seat (%d,%d) for '%s' at %s %s",
            row, col, movie_cache[movie_index].title, day, time);
}

// --- CÁC HÀM ADMIN ---

void handle_add_movie(const char *payload, char *out)
{
    char title[100], genre[50];
    int duration;

    sscanf(payload, "ADD_MOVIE title=\"%[^\"]\" genre=\"%[^\"]\" duration=%d",
           title, genre, &duration);

    if (movie_count >= 100)
    {
        strcpy(out, "ERROR: Movie list full");
        return;
    }

    Movie *m = &movie_cache[movie_count++];
    if (movie_count == 0)
        m->id = 1;
    else
        m->id = movie_cache[movie_count - 1].id + 1;
    strcpy(m->title, title);
    strcpy(m->genre, genre);
    m->duration = duration;

    memset(m->schedule, 0, sizeof(m->schedule));
    memset(m->seatmap, 0, sizeof(m->seatmap));
    movie_count++;
    if (!save_all_movies("movies.txt"))
    {
        strcpy(out, "ERROR: Cannot save movie file");
        return;
    }

    sprintf(out, "SUCCESS: Movie added (ID: %d)", m->id);
}

void handle_delete_movie(const char *payload, char *out)
{
    char title[200];

    // 1. Lấy tên phim từ payload
    sscanf(payload, "DELETE_MOVIE title=\"%199[^\"]\"", title);

    // 2. Tìm vị trí phim (Giờ đây nó đã hỗ trợ không phân biệt hoa thường)
    int index = find_movie_by_title(title);

    if (index < 0)
    {
        strcpy(out, "ERROR: Movie not found");
        return;
    }

    // 3. Thực hiện xóa bằng cách dồn mảng
    // Ví dụ: Mảng có [A, B, C, D], xóa B (index 1)
    // Ta chép C đè lên B, D đè lên C -> [A, C, D]
    for (int i = index; i < movie_count - 1; i++)
    {
        movie_cache[i] = movie_cache[i + 1];
    }

    // 4. Giảm số lượng phim và làm sạch phần tử cuối (để tránh rác)
    movie_count--;
    memset(&movie_cache[movie_count], 0, sizeof(Movie));

    // 5. Lưu lại file
    save_all_movies("movies.txt");

    // 6. Thông báo thành công
    // Gửi lại tên phim GỐC (trong danh sách) để user biết chính xác phim nào đã xóa
    sprintf(out, "SUCCESS: Deleted movie");
}

void handle_update_movie(const char *payload, char *out)
{
    char title[100], new_genre[50];
    int new_duration;
    sscanf(payload,
           "UPDATE_MOVIE title=\"%[^\"]\" new_genre=\"%[^\"]\" new_duration=%d",
           title, new_genre, &new_duration);

    int i = find_movie_by_title(title);
    if (i < 0)
    {
        strcpy(out, "ERROR: Movie not found");
        return;
    }

    strcpy(movie_cache[i].genre, new_genre);
    movie_cache[i].duration = new_duration;
    save_all_movies("movies.txt");

    strcpy(out, "SUCCESS: Movie updated");
}

void handle_add_schedule(const char *payload, char *out)
{
    char title[100], day[20], time_list[200];

    // Parse input: time nằm trong ngoặc kép (time="%[^"]")
    sscanf(payload, "ADD_SCHEDULE title=\"%[^\"]\" day=\"%[^\"]\" time=\"%[^\"]\"",
           title, day, time_list);

    int movie_index = find_movie_by_title(title);
    if (movie_index < 0)
    {
        strcpy(out, "ERROR: Movie not found");
        return;
    }

    int di = map_day_to_index(day);
    if (di < 0)
    {
        strcpy(out, "ERROR: Invalid day");
        return;
    }

    Movie *m = &movie_cache[movie_index];
    int added_count = 0;
    int slots_full = 0;

    // Tách chuỗi bằng dấu phẩy (ví dụ "9h, 14h, 19h")
    char *token = strtok(time_list, ",");
    while (token != NULL)
    {
        trim(token); // Xóa khoảng trắng thừa (ví dụ " 14h" -> "14h")

        if (strlen(token) > 0)
        {
            int found_empty = 0;
            // Tìm slot trống để nhét giờ vào
            for (int i = 0; i < MAX_SLOTS; i++)
            {
                if (strlen(m->schedule[di][i]) == 0)
                {
                    strncpy(m->schedule[di][i], token, 9);
                    m->schedule[di][i][9] = '\0'; // Đảm bảo null-terminated

                    // Reset ghế cho suất chiếu mới này
                    memset(m->seatmap[di][i], ' ', sizeof(m->seatmap[di][i]));

                    found_empty = 1;
                    added_count++;
                    break;
                }
            }
            if (!found_empty)
                slots_full = 1;
        }
        token = strtok(NULL, ",");
    }

    if (added_count > 0)
    {
        save_all_movies("movies.txt");
        if (slots_full)
            sprintf(out, "SUCCESS: Added %d schedules (Warning: Some slots were full)", added_count);
        else
            sprintf(out, "SUCCESS: Added %d schedules", added_count);
    }
    else
    {
        strcpy(out, "ERROR: No schedules added (Slots full or invalid input)");
    }
}

void handle_reset_seatmap(const char *payload, char *out)
{
    char title[100], day[20], time[10];
    sscanf(payload, "RESET_SEATMAP title=\"%[^\"]\" day=\"%[^\"]\" time=%s",
           title, day, time);

    int mi = find_movie_by_title(title);
    int di = map_day_to_index(day);
    int ti = find_time_slot(mi, di, time);

    if (mi >= 0 && di >= 0 && ti >= 0)
    {
        memset(movie_cache[mi].seatmap[di][ti], ' ', sizeof(movie_cache[mi].seatmap[di][ti]));
        save_bookings();
        strcpy(out, "SUCCESS: Seatmap reset");
    }
    else
    {
        strcpy(out, "ERROR: Slot not found");
    }
}

void handle_delete_schedule(const char *payload, char *out)
{
    char title[200], day[200], time_raw[200];
    // time_raw sẽ chứa chuỗi gốc "9h, 14h"

    // 1. Đọc dữ liệu thô
    sscanf(payload, "DELETE_SCHEDULE title=\"%199[^\"]\" day=\"%199[^\"]\" time=\"%199[^\"]\"",
           title, day, time_raw);

    // 2. Kiểm tra Phim và Ngày (Chỉ cần kiểm tra 1 lần)
    int mi = find_movie_by_title(title);
    if (mi < 0)
    {
        strcpy(out, "ERROR: Movie not found");
        return;
    }

    int di = map_day_to_index(day);
    if (di < 0)
    {
        strcpy(out, "ERROR: Invalid day");
        return;
    }

    // 3. Xử lý cắt chuỗi thời gian bằng dấu phẩy
    int deleted_count = 0;               // Đếm số suất đã xóa được
    char *token = strtok(time_raw, ","); // Lấy giờ đầu tiên

    while (token != NULL)
    {
        // --- BƯỚC QUAN TRỌNG: Xóa khoảng trắng thừa ở đầu ---
        // Ví dụ: " 14h" (dư dấu cách) -> "14h"
        while (*token == ' ')
            token++;

        // Tìm vị trí suất chiếu này
        int ti = find_time_slot(mi, di, token);

        if (ti >= 0)
        {
            // Nếu tìm thấy -> Xóa
            movie_cache[mi].schedule[di][ti][0] = '\0';
            memset(movie_cache[mi].seatmap[di][ti], ' ', sizeof(movie_cache[mi].seatmap[di][ti]));
            deleted_count++;
            printf("DEBUG: Deleted time slot [%s]\n", token);
        }
        else
        {
            printf("DEBUG: Time slot [%s] not found to delete\n", token);
        }

        // Lấy giờ tiếp theo
        token = strtok(NULL, ",");
    }

    // 4. Lưu và phản hồi
    if (deleted_count > 0)
    {
        save_all_movies("movies.txt");
        // Thông báo đã xóa bao nhiêu suất
        sprintf(out, "SUCCESS: Deleted %d schedule(s)", deleted_count);
    }
    else
    {
        // Không xóa được cái nào (do nhập sai hết hoặc không tìm thấy)
        strcpy(out, "ERROR: No matching time slots found to delete");
    }
}