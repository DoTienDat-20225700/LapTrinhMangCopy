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

// Hàm chuyển chuỗi giờ (vd: "17h30", "17", "17h") thành tổng số phút
int time_str_to_minutes(const char *time_str)
{
    int h = 0, m = 0;

    // Tạo bản sao để xử lý
    char temp[20];
    strncpy(temp, time_str, 19);

    char *sep = strpbrk(temp, "h:");
    if (sep)
    {
        *sep = '\0';
        h = atoi(temp);
        m = atoi(sep + 1);
    }
    else
    {
        h = atoi(temp);
        m = 0;
    }
    return h * 60 + m;
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

// Trả về số lượng thể loại tìm được. Danh sách lưu vào mảng `out_list`
int get_unique_genres(char out_list[50][MAX_GENRE])
{
    int count = 0;

    for (int i = 0; i < movie_count; i++)
    {
        // Tạo bản sao để cắt chuỗi (vì strtok làm hỏng chuỗi gốc)
        char temp_genre[MAX_GENRE];
        strcpy(temp_genre, movie_cache[i].genre);

        char *token = strtok(temp_genre, ",");
        while (token != NULL)
        {
            trim(token); // Xóa khoảng trắng thừa (ví dụ " Kinh Dị" -> "Kinh Dị")

            // Kiểm tra xem thể loại này đã có trong danh sách chưa
            int exists = 0;
            for (int j = 0; j < count; j++)
            {
                if (string_equals_ignore_case(out_list[j], token))
                {
                    exists = 1;
                    break;
                }
            }

            // Nếu chưa có thì thêm vào
            if (!exists && count < 50)
            {
                strcpy(out_list[count], token);
                count++;
            }

            token = strtok(NULL, ",");
        }
    }
    return count;
}

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

// ... (Các hàm khác giữ nguyên)

int load_movies(const char *filename, Movie *movies, int max_movies)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Server: Movie file not found (starting fresh).\n");
        return 0;
    }

    char line[4096];
    int count = 0;

    printf("Server: Loading database from %s...\n", filename);

    while (fgets(line, sizeof(line), fp) && count < max_movies)
    {
        // Xóa ký tự xuống dòng
        line[strcspn(line, "\r\n")] = 0;

        // Bỏ qua dòng quá ngắn hoặc dòng trống
        if (strlen(line) < 2)
            continue;

        char *saveptr;
        // Tách các trường
        char *id_str = strtok_r(line, "|", &saveptr);
        char *title = strtok_r(NULL, "|", &saveptr);
        char *genre = strtok_r(NULL, "|", &saveptr);
        char *dur_str = strtok_r(NULL, "|", &saveptr);

        // Phần lịch chiếu là phần còn lại
        char *sched_str = saveptr;

        // Nếu thiếu thông tin quan trọng thì âm thầm bỏ qua (không in lỗi nữa)
        if (!id_str || !title || !genre || !dur_str)
            continue;

        // --- GÁN DỮ LIỆU ---
        movies[count].id = atoi(id_str);
        strncpy(movies[count].title, title, MAX_TITLE - 1);
        strncpy(movies[count].genre, genre, MAX_GENRE - 1);
        movies[count].duration = atoi(dur_str);

        // Reset dữ liệu cũ
        for (int d = 0; d < MAX_DAYS; d++)
            for (int s = 0; s < MAX_SLOTS; s++)
            {
                movies[count].schedule[d][s][0] = 0;
                memset(movies[count].seatmap[d][s], ' ', 15);
            }

        // Xử lý lịch chiếu (Parsing Schedule)
        if (sched_str && strlen(sched_str) > 0)
        {
            char sched_copy[2048];
            strncpy(sched_copy, sched_str, 2047);

            char *sp2;
            char *day_block = strtok_r(sched_copy, ";", &sp2);
            while (day_block)
            {
                char *colon = strchr(day_block, ':');
                if (colon)
                {
                    *colon = 0;
                    int d_idx = map_day_to_index(day_block);
                    if (d_idx >= 0)
                    {
                        char *sp3;
                        char *t = strtok_r(colon + 1, ",", &sp3);
                        int s_idx = 0;
                        while (t && s_idx < MAX_SLOTS)
                        {
                            while (*t == ' ')
                                t++;
                            if (strlen(t) > 0)
                                strcpy(movies[count].schedule[d_idx][s_idx++], t);
                            t = strtok_r(NULL, ",", &sp3);
                        }
                    }
                }
                day_block = strtok_r(NULL, ";", &sp2);
            }
        }
        count++;
    }
    fclose(fp);
    printf("Server: Database ready. Total %d movies loaded successfully.\n", count);
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
    char unique_genres[50][MAX_GENRE];
    int count = get_unique_genres(unique_genres);

    if (count == 0)
    {
        strcpy(response_out, "No genres available.");
        return;
    }

    strcpy(response_out, "Available genres:\n");
    for (int i = 0; i < count; i++)
    {
        char line[100];
        // Format: "1. Kinh Dị\n"
        sprintf(line, "%d. %s\n", i + 1, unique_genres[i]);

        if (strlen(response_out) + strlen(line) < MAXLINE)
        {
            strcat(response_out, line);
        }
    }
}

void handle_filter_genre(const char *payload, char *response_out)
{
    int genre_id;
    // Nhận ID từ client (Format: FILTER_GENRE id=1)
    sscanf(payload, "FILTER_GENRE id=%d", &genre_id);

    // 1. Tái tạo lại danh sách thể loại để tìm xem ID đó ứng với tên gì
    char unique_genres[50][MAX_GENRE];
    int total_genres = get_unique_genres(unique_genres);

    // 2. Validate ID
    if (genre_id < 1 || genre_id > total_genres)
    {
        sprintf(response_out, "Invalid Genre ID: %d", genre_id);
        return;
    }

    // 3. Lấy tên thể loại (ID 1 -> index 0)
    char *target_genre = unique_genres[genre_id - 1];

    // 4. Tìm phim theo tên thể loại vừa lấy được
    strcpy(response_out, "");
    int found_count = 0;

    // Thêm dòng thông báo đang lọc theo gì
    char header[100];
    sprintf(header, "--- Movies with genre: %s ---\n", target_genre);
    strcat(response_out, header);

    for (int i = 0; i < movie_count; i++)
    {
        // Dùng stristr để tìm (ví dụ phim "Hài, Tình Cảm" chứa "Hài")
        if (stristr(movie_cache[i].genre, target_genre) != NULL)
        {
            char line[MAXLINE];
            sprintf(line, "[ID: %d] %s\nGenre: %s\nDuration: %d mins\n\n",
                    movie_cache[i].id, movie_cache[i].title, movie_cache[i].genre, movie_cache[i].duration);

            if (strlen(response_out) + strlen(line) < MAXLINE)
            {
                strcat(response_out, line);
                found_count++;
            }
        }
    }

    if (found_count == 0)
    {
        strcat(response_out, "No movies found.");
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

    // 1. Chuyển thời gian nhập của User sang PHÚT
    int u_begin_mins = time_str_to_minutes(begin_str);
    int u_end_mins = time_str_to_minutes(end_str);

    // Xử lý logic 0h (nửa đêm)
    if (u_end_mins == 0)
        u_end_mins = 24 * 60; // 0h -> 24h (1440 phút)

    // Xử lý tìm kiếm xuyên đêm (ví dụ: 22h -> 2h sáng hôm sau)
    if (u_end_mins < u_begin_mins)
    {
        u_end_mins += 24 * 60; // Cộng thêm 24h vào giờ kết thúc
    }

    printf("Filtering time (mins): Day=%s, Range: %d - %d\n", day, u_begin_mins, u_end_mins);

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

            // 2. Chuyển giờ chiếu của Phim sang PHÚT
            int m_start_mins = time_str_to_minutes(time_str);
            int duration = movie_cache[i].duration;
            int m_end_mins = m_start_mins + duration; // Giờ kết thúc của phim
            // Xử lý trường hợp phim chiếu sau nửa đêm trong logic xuyên đêm
            // Nếu User tìm từ 22h (1320p) đến 26h (1560p), mà phim chiếu lúc 1h (60p)
            // Ta cần hiểu 1h đó là 25h (1500p) để so sánh
            int compare_start = m_start_mins;
            int compare_end = m_end_mins;

            if (u_end_mins > 24 * 60 && m_start_mins < u_begin_mins)
            {
                compare_start += 24 * 60;
                compare_end += 24 * 60;
            }

            // 3. So sánh chính xác theo phút
            if (compare_start >= u_begin_mins && compare_end <= u_end_mins)
            {
                // Tính toán để hiển thị
                int m_show_start_h = (m_start_mins / 60) % 24;
                int m_show_start_m = m_start_mins % 60;

                int m_show_end_h = (m_end_mins / 60) % 24;
                int m_show_end_m = m_end_mins % 60;

                char slot_info[50];
                sprintf(slot_info, "%02dh%02d - %02dh%02d", m_show_start_h, m_show_start_m, m_show_end_h, m_show_end_m);

                if (found_slots > 0)
                    strcat(time_list, ", ");
                strcat(time_list, slot_info);
                found_slots++;
            }
        }
        if (found_slots > 0)
        {
            char movie_entry[MAXLINE];
            sprintf(movie_entry, "[ID: %d] Title: %s\nGenre: %s\nDuration: %d mins\nSchedule: %s\n\n",
                    movie_cache[i].id,
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

    // 1. Parse input
    sscanf(payload, "BOOK_SEAT id=%d day=\"%[^\"]\" time=%s row=%d col=%d", &movie_id, day, time, &row, &col);

    // 2. Validate
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

    // 3. Đặt ghế & Lưu
    movie_cache[movie_index].seatmap[day_index][time_index][row - 1][col - 1] = 'x';
    save_bookings();

    int h = 0, m = 0;
    char *sep = strpbrk(time, "h:");

    if (sep != NULL)
    {
        *sep = '\0';
        h = atoi(time);
        m = atoi(sep + 1);
        *sep = 'h';
    }
    else
    {
        h = atoi(time);
        m = 0;
    }

    char display_time[20];
    sprintf(display_time, "%02dh%02d", h, m);

    // 4. In vé
    sprintf(response_out,
            "SUCCESS\n"
            "***************************************\n"
            "* MOVIE TICKET              *\n"
            "***************************************\n"
            "  Movie : %s\n"
            "  Day : %s\n"
            "  Time  : %s\n"
            "  Seat  : Row %d - Col %d\n"
            "***************************************",
            movie_cache[movie_index].title, day, display_time, row, col);
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

    int new_id;
    if (movie_count == 0)
    {
        new_id = 1;
    }
    else
    {
        new_id = movie_cache[movie_count - 1].id + 1;
    }

    Movie *m = &movie_cache[movie_count];

    m->id = new_id;
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
    int id;
    sscanf(payload, "DELETE_MOVIE id=%d", &id); // Nhận ID thay vì Title

    int index = find_movie_by_id(id);
    if (index < 0)
    {
        strcpy(out, "ERROR: Movie ID not found");
        return;
    }

    // Dồn mảng để xóa
    for (int i = index; i < movie_count - 1; i++)
    {
        movie_cache[i] = movie_cache[i + 1];
    }

    movie_count--;
    memset(&movie_cache[movie_count], 0, sizeof(Movie)); // Xóa rác ở cuối
    save_all_movies("movies.txt");

    sprintf(out, "SUCCESS: Deleted movie ID %d", id);
}

void handle_update_movie(const char *payload, char *out)
{
    int id, new_duration;
    char new_genre[50];

    sscanf(payload, "UPDATE_MOVIE id=%d new_genre=\"%[^\"]\" new_duration=%d",
           &id, new_genre, &new_duration);

    int index = find_movie_by_id(id);
    if (index < 0)
    {
        strcpy(out, "ERROR: Movie ID not found");
        return;
    }

    strcpy(movie_cache[index].genre, new_genre);
    movie_cache[index].duration = new_duration;
    save_all_movies("movies.txt");

    strcpy(out, "SUCCESS: Movie updated");
}

void handle_add_schedule(const char *payload, char *out)
{
    int id;
    char day[20], time_list[200];

    sscanf(payload, "ADD_SCHEDULE id=%d day=\"%[^\"]\" time=\"%[^\"]\"",
           &id, day, time_list);

    int mi = find_movie_by_id(id);
    if (mi < 0)
    {
        strcpy(out, "ERROR: Movie ID not found");
        return;
    }

    int di = map_day_to_index(day);
    if (di < 0)
    {
        strcpy(out, "ERROR: Invalid day");
        return;
    }

    Movie *m = &movie_cache[mi];
    int count = 0;

    // Tách chuỗi giờ (ví dụ: "9h, 14h")
    char *token = strtok(time_list, ",");
    while (token != NULL)
    {
        trim(token);
        if (strlen(token) > 0)
        {
            for (int s = 0; s < MAX_SLOTS; s++)
            {
                if (strlen(m->schedule[di][s]) == 0)
                { // Tìm slot trống
                    strcpy(m->schedule[di][s], token);
                    memset(m->seatmap[di][s], ' ', 15); // Reset ghế
                    count++;
                    break;
                }
            }
        }
        token = strtok(NULL, ",");
    }

    save_all_movies("movies.txt");
    sprintf(out, "SUCCESS: Added %d schedules for ID %d", count, id);
}

void handle_reset_seatmap(const char *payload, char *out)
{
    int id;
    char day[20], time[20];
    sscanf(payload, "RESET_SEATMAP id=%d day=\"%[^\"]\" time=%s", &id, day, time);

    int mi = find_movie_by_id(id);
    if (mi < 0)
    {
        strcpy(out, "ERROR: Movie ID not found");
        return;
    }

    int di = map_day_to_index(day);
    int ti = find_time_slot(mi, di, time);

    if (di >= 0 && ti >= 0)
    {
        memset(movie_cache[mi].seatmap[di][ti], ' ', 15);
        save_bookings(); // Lưu lại trạng thái trống
        strcpy(out, "SUCCESS: Seatmap reset");
    }
    else
    {
        strcpy(out, "ERROR: Schedule not found");
    }
}

void handle_delete_schedule(const char *payload, char *out)
{
    int id;
    char day[20], time_raw[200];

    sscanf(payload, "DELETE_SCHEDULE id=%d day=\"%[^\"]\" time=\"%[^\"]\"",
           &id, day, time_raw);

    int mi = find_movie_by_id(id);
    if (mi < 0)
    {
        strcpy(out, "ERROR: Movie ID not found");
        return;
    }

    int di = map_day_to_index(day);
    if (di < 0)
    {
        strcpy(out, "ERROR: Invalid day");
        return;
    }

    int del_count = 0;
    char *token = strtok(time_raw, ",");
    while (token)
    {
        trim(token);
        int ti = find_time_slot(mi, di, token);
        if (ti >= 0)
        {
            movie_cache[mi].schedule[di][ti][0] = 0;          // Xóa giờ
            memset(movie_cache[mi].seatmap[di][ti], ' ', 15); // Xóa ghế
            del_count++;
        }
        token = strtok(NULL, ",");
    }

    if (del_count > 0)
    {
        save_all_movies("movies.txt");
        save_bookings(); // Cập nhật cả file booking nếu cần
        sprintf(out, "SUCCESS: Deleted %d schedule(s)", del_count);
    }
    else
    {
        strcpy(out, "ERROR: Time slot not found");
    }
}