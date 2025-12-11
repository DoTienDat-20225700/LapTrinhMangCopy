#ifndef MOVIE_H
#define MOVIE_H

#define MAX_TITLE 100
#define MAX_GENRE 100
#define MAX_DAYS 7
#define MAX_SLOTS 10

typedef struct
{
    int id;
    char title[MAX_TITLE];
    char genre[MAX_GENRE];
    int duration;
    char schedule[MAX_DAYS][MAX_SLOTS][10];  // day x time slots
    char seatmap[MAX_DAYS][MAX_SLOTS][3][5]; // 3 rows × 5 cols per time slot
} Movie;

// Core movie database functions
int find_movie_by_id(int id);
int map_day_to_index(const char *day);
int load_movies(const char *filename, Movie *movies, int max_movies);
int save_movie(const char *filename, const Movie *movie);
void save_bookings();
void load_bookings();

// Server-side handlers
void handle_list_movies(char *response_out);
void handle_search(const char *payload, char *response_out);
void handle_list_genres(char *response_out);
void handle_filter_genre(const char *payload, char *response_out);
void handle_filter_time(const char *payload, char *response_out);
void handle_get_seatmap(const char *payload, char *response_out);
void handle_book_seat(const char *payload, char *response_out);

void handle_add_movie(const char *payload, char *out);
void handle_delete_movie(const char *payload, char *out);
void handle_update_movie(const char *payload, char *out);
void handle_add_schedule(const char *payload, char *out);
void handle_reset_seatmap(const char *payload, char *out);
void handle_delete_schedule(const char *payload, char *out);

#endif
