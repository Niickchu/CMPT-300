#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USERNAME_LENGTH 33  //max username length is 32

int getUserId(const char* username);

int main() {
    printf("Enter username: ");

    char username[USERNAME_LENGTH + 1];
    fgets(username, sizeof(username), stdin);   //fgets is safer than scanf
    username[strcspn(username, "\n")] = 0;  //remove trailing newline

    int userId = getUserId(username);

    if (userId == -1) {
        printf("Username not found.\n");
    } else {
        printf("%d\n", userId);
    }

    return 0;
}

int getUserId(const char* username) {
    FILE* file = fopen("/etc/passwd", "r");

    if (file == NULL) {
        printf("Failed to open /etc/passwd file.\n");
        exit(1);
    }

    char line[1024];
    while (fgets(line, 1024, file) != NULL) {
        char* token = strtok(line, ":");
        if(strcmp(token, username) != 0){   //find username line
            continue;
        }

        for(int i =0; i < 2; i++){  //skip to userID
            token = strtok(NULL, ":");
        }

        unsigned int userID = atoi(token);  //unsigned int is used because userID is always positive
        fclose(file);
        return userID;
    }

    fclose(file);
    return -1;
}
