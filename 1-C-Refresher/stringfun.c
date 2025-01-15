#include <stdio.h>
#include <string.h>   // for memcpy
#include <stdlib.h>

#define BUFFER_SZ 50

// Function prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);
int  count_words(char *, int, int);
int  reverse_string(char *, int, int);
int  print_words(char *, int, int);

// Extra credit prototypes
int  my_strlen(char *);
int  do_replace(char *buff, int *p_str_len, int buff_len, char *search, char *replacement);

// ---------------------------------------------------------------------------
// setup_buff: copies user_str into buff, collapsing multiple spaces, ignoring
// leading spaces, and padding with '.' up to BUFFER_SZ. Returns the # of
// real characters in the user string (not counting dots).
// ---------------------------------------------------------------------------
int setup_buff(char *buff, char *user_str, int buff_len)
{
    char *src = user_str;
    char *dst = buff;
    int user_count = 0;
    int in_space = 1;

    while (*src != '\0') {
        if (*src == ' ' || *src == '\t') {
            if (!in_space && user_count > 0) {
                if (user_count >= buff_len) {
                    return -1;
                }
                *dst++ = ' ';
                user_count++;
            }
            in_space = 1;
        } else {
            if (user_count >= buff_len) {
                return -1;
            }
            *dst++ = *src;
            user_count++;
            in_space = 0;
        }
        src++;
    }

    // fill remainder with '.'
    for (int i = user_count; i < buff_len; i++) {
        *dst++ = '.';
    }

    return user_count;
}

// ---------------------------------------------------------------------------
// print_buff: prints the entire buffer (including dots).
// ---------------------------------------------------------------------------
void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i = 0; i < len; i++){
        putchar(*(buff + i));
    }
    putchar('\n');
}

// ---------------------------------------------------------------------------
// usage: prints how to run the program.
// ---------------------------------------------------------------------------
void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

// ---------------------------------------------------------------------------
// count_words: returns number of words in buff (up to str_len). A word is
// separated by spaces.
// ---------------------------------------------------------------------------
int count_words(char *buff, int len, int str_len){
    if (str_len > len) return -1;
    if (str_len == 0) return 0;

    int word_count = 0;
    int in_space = 1;

    for (int i = 0; i < str_len; i++){
        if (buff[i] == ' ') {
            in_space = 1;
        } else {
            if (in_space) {
                word_count++;
                in_space = 0;
            }
        }
    }
    return word_count;
}

// ---------------------------------------------------------------------------
// reverse_string: reverses only the user portion (str_len chars).
// ---------------------------------------------------------------------------
int reverse_string(char *buff, int buff_len, int str_len){
    if (str_len > buff_len) return -1;
    if (str_len <= 1) return 0;

    int left = 0;
    int right = str_len - 1;
    while (left < right) {
        char temp = buff[left];
        buff[left] = buff[right];
        buff[right] = temp;
        left++;
        right--;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// print_words: prints each word and its length. Ignores trailing dots.
// ---------------------------------------------------------------------------
int print_words(char *buff, int buff_len, int str_len){
    if (str_len > buff_len) return -1;
    if (str_len == 0) return 0;

    printf("Word Print\n");
    printf("----------\n");

    int word_count = 0;
    int in_space = 1;
    int start_i = 0;
    int chars_in_word = 0;

    for (int i = 0; i < str_len; i++){
        if (buff[i] == ' ') {
            if (!in_space) {
                word_count++;
                printf("%d. ", word_count);
                for (int j = start_i; j < i; j++){
                    putchar(buff[j]);
                }
                printf(" (%d)\n", chars_in_word);
                chars_in_word = 0;
            }
            in_space = 1;
        } else {
            if (in_space) {
                start_i = i;
                chars_in_word = 1;
                in_space = 0;
            } else {
                chars_in_word++;
            }
        }
    }

    // if buffer ended while still in a word
    if (!in_space) {
        word_count++;
        printf("%d. ", word_count);
        for (int j = start_i; j < str_len; j++){
            putchar(buff[j]);
        }
        printf(" (%d)\n", chars_in_word);
    }

    return word_count;
}

// ---------------------------------------------------------------------------
// my_strlen: small helper to measure string length (no standard library).
// ---------------------------------------------------------------------------
int my_strlen(char *s) {
    int n = 0;
    while (*s != '\0') {
        s++;
        n++;
    }
    return n;
}

// ---------------------------------------------------------------------------
// do_replace: replaces the FIRST occurrence of `search` with `replacement`.
// Returns 0 if success, 1 if no match, -1 if error (e.g. not enough space).
// ---------------------------------------------------------------------------
int do_replace(char *buff, int *p_str_len, int buff_len, char *search, char *replacement)
{
    int search_len = my_strlen(search);
    int repl_len   = my_strlen(replacement);
    int str_len    = *p_str_len;

    if (search_len == 0) {
        return -1;  // can't search for empty
    }
    if (search_len > str_len) {
        return 1;   // search bigger than user string => no match
    }

    int match_index = -1;
    for (int i = 0; i <= (str_len - search_len); i++) {
        int match = 1;
        for (int j = 0; j < search_len; j++) {
            if (buff[i + j] != search[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            match_index = i;
            break;
        }
    }
    if (match_index < 0) {
        // no match found
        return 1;
    }

    // new length after replacement
    int new_len = str_len - search_len + repl_len;
    if (new_len > buff_len) {
        return -1; // overflow
    }

    // shift tail
    int tail_len = str_len - (match_index + search_len);
    if (tail_len > 0) {
        memcpy(buff + match_index + repl_len,
               buff + match_index + search_len,
               tail_len);
    }

    // insert replacement
    for (int k = 0; k < repl_len; k++) {
        buff[match_index + k] = replacement[k];
    }

    *p_str_len = new_len;

    // fill leftover with '.'
    for (int x = new_len; x < buff_len; x++) {
        buff[x] = '.';
    }

    return 0;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char *argv[]){

    char *buff;
    char *input_string;
    char opt;
    int  rc;
    int  user_str_len;

    // #1: This is safe because we check (argc < 2) first
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1] + 1);

    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    // #2: We need at least 3 arguments: program name, option, user string
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2];

    // #3: Allocate buffer with malloc, exit(99) on failure
    buff = (char *)malloc(BUFFER_SZ);
    if (!buff) {
        fprintf(stderr, "malloc failed\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d\n", user_str_len);
        free(buff);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error counting words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        case 'r':
            rc = reverse_string(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error reversing string, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            printf("Reversed String: ");
            for (int i = 0; i < user_str_len; i++){
                putchar(buff[i]);
            }
            putchar('\n');
            break;

        case 'w':
            rc = print_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error printing words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            break;

        case 'x': {
            // Extra credit: need 2 more args => search, replacement
            if (argc < 5) {
                fprintf(stderr, "error: -x requires 3 arguments.\n");
                free(buff);
                exit(1);
            }
            rc = do_replace(buff, &user_str_len, BUFFER_SZ, argv[3], argv[4]);
            if (rc < 0) {
                printf("Error replacing \"%s\" with \"%s\"\n", argv[3], argv[4]);
                free(buff);
                exit(3);
            } else if (rc > 0) {
                printf("No match for \"%s\" found.\n", argv[3]);
            } else {
                printf("Modified String: ");
                for (int i = 0; i < user_str_len; i++){
                    putchar(buff[i]);
                }
                putchar('\n');
            }
            break;
        }

        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    // #6: Free buffer before exiting
    print_buff(buff, BUFFER_SZ);
    free(buff);

    exit(0);
}

// #7: Providing both pointer and length is good for preventing overflow,
//     reusability, and more clarity.
