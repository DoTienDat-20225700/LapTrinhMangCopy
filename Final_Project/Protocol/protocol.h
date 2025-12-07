#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAXLINE 4096
#define MAX_PAYLOAD 4096

typedef enum
{
    CREATE_ACCOUNT,
    LOGIN,
    CHECK_USERNAME,
    CHECK_PASSWORD,
    SEARCH,
    LIST_MOVIES,
    LIST_GENRES,
    FILTER_GENRE,
    FILTER_TIME,
    GET_SEATMAP,
    BOOK_SEAT,

    ADD_MOVIE,
    DELETE_MOVIE,
    UPDATE_MOVIE,
    ADD_SCHEDULE,
    DELETE_SCHEDULE,
    RESET_SEATMAP,
    LIST_USERS,
    DELETE_USER,
    SET_ROLE,

    UNKNOWN
} CommandType;

typedef struct
{
    CommandType type;
    char payload[MAX_PAYLOAD];
} Message;

CommandType parse_command_type(const char *payload);
void parse_message(const char *raw, Message *msg);
void format_message(CommandType type, const char *payload, char *out);

#endif
