#include "protocol.h"
#include <string.h>
#include <stdio.h>

CommandType parse_command_type(const char *payload)
{
    if (strncmp(payload, "CREATE_ACCOUNT", 14) == 0)
        return CREATE_ACCOUNT;
    if (strncmp(payload, "LOGIN", 5) == 0)
        return LOGIN;
    if (strncmp(payload, "CHECK_USERNAME", 14) == 0)
        return CHECK_USERNAME;
    if (strncmp(payload, "CHECK_PASSWORD", 14) == 0)
        return CHECK_PASSWORD;
    if (strncmp(payload, "SEARCH", 6) == 0)
        return SEARCH;
    if (strncmp(payload, "LIST_MOVIES", 11) == 0)
        return LIST_MOVIES;
    if (strncmp(payload, "LIST_GENRES", 11) == 0)
        return LIST_GENRES;
    if (strncmp(payload, "FILTER_GENRE", 12) == 0)
        return FILTER_GENRE;
    if (strncmp(payload, "FILTER_TIME", 11) == 0)
        return FILTER_TIME;
    if (strncmp(payload, "GET_SEATMAP", 11) == 0)
        return GET_SEATMAP;
    if (strncmp(payload, "BOOK_SEAT", 9) == 0)
        return BOOK_SEAT;
    if (strncmp(payload, "ADD_MOVIE", 9) == 0)
        return ADD_MOVIE;
    if (strncmp(payload, "DELETE_MOVIE", 12) == 0)
        return DELETE_MOVIE;
    if (strncmp(payload, "UPDATE_MOVIE", 12) == 0)
        return UPDATE_MOVIE;
    if (strncmp(payload, "ADD_SCHEDULE", 12) == 0)
        return ADD_SCHEDULE;
    if (strncmp(payload, "DELETE_SCHEDULE", 15) == 0)
        return DELETE_SCHEDULE;
    if (strncmp(payload, "RESET_SEATMAP", 13) == 0)
        return RESET_SEATMAP;
    if (strncmp(payload, "LIST_USERS", 10) == 0)
        return LIST_USERS;
    if (strncmp(payload, "DELETE_USER", 11) == 0)
        return DELETE_USER;
    if (strncmp(payload, "SET_ROLE", 8) == 0)
        return SET_ROLE;
    if (strncmp(payload, "EXIT", 3) == 0)
        return EXIT;
    return UNKNOWN;
}

void parse_message(const char *raw, Message *msg)
{
    msg->type = parse_command_type(raw);
    strncpy(msg->payload, raw, MAX_PAYLOAD - 1);
    msg->payload[MAX_PAYLOAD - 1] = '\0';
}

void format_message(CommandType type, const char *payload, char *out)
{
    const char *prefix;
    switch (type)
    {
    case CREATE_ACCOUNT:
        prefix = "CREATE_ACCOUNT";
        break;
    case LOGIN:
        prefix = "LOGIN";
        break;
    case CHECK_USERNAME:
        prefix = "CHECK_USERNAME";
        break;
    case CHECK_PASSWORD:
        prefix = "CHECK_PASSWORD";
        break;
    case SEARCH:
        prefix = "SEARCH";
        break;
    case LIST_MOVIES:
        prefix = "LIST_MOVIES";
        break;
    case LIST_GENRES:
        prefix = "LIST_GENRES";
        break;
    case FILTER_GENRE:
        prefix = "FILTER_GENRE";
        break;
    case FILTER_TIME:
        prefix = "FILTER_TIME";
        break;
    case GET_SEATMAP:
        prefix = "GET_SEATMAP";
        break;
    case BOOK_SEAT:
        prefix = "BOOK_SEAT";
        break;
    case EXIT:
        prefix = "EXIT";
        break;
    default:
        prefix = "UNKNOWN";
        break;
    }
    snprintf(out, MAXLINE, "%s %s", prefix, payload);
}
