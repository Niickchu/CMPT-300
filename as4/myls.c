#include <stdio.h>
#include <unistd.h>
#include <getopt.h>  //not actually needed because we are using unistd.h
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>

bool iFlag = 0;
bool lFlag = 0;
bool rFlag = 0;
int num_files = 0;
int num_directories = 0;

void printDirectory(char *inputDirectory);
void printFileInfo(struct stat *entry);
void printLongInfo(struct stat *file_stat);
void printFileName(char *filename,char* root,struct stat *file_stat);
void printDirectoryRecursive(char *inputDirectory);
void print_permissions(unsigned int st_mode);
void print_file_type(unsigned int st_mode);

int alphabetical_sort(const struct dirent **dirent_1, const struct dirent **dirent_2) {
    return strcasecmp((*dirent_1)->d_name, (*dirent_2)->d_name);
}

int alphabetical_string_sort(const void *string_1, const void *string_2) {
    return strcasecmp(*(char **)string_1, *(char **)string_2);
}

int main(int argc, char *argv[]) {

    int opt;
    opterr = 0; //override error message
    while((opt = getopt(argc, argv, "ilR")) != -1) { //lRi is 3 loops
        switch(opt) {
            case 'i':
                iFlag = 1;
                //printf("iFlag: %d\n", iFlag);
                break;
            case 'l':
                lFlag = 1;
                //printf("lFlag: %d\n", lFlag);
                break;
            case 'R':
                rFlag = 1;
                //printf("rFlag: %d\n", rFlag);
                break;
            default:
                printf("Error: Unsupported Option\n");
                exit(1);
        }
    }
    
    if (optind == argc){    //no arguments, list current directory
        if(rFlag)
            printDirectoryRecursive(".");
        else
            printDirectory(".");
        return 0;
    }

    struct stat current_stat;   //stat struct for current file

    //declare arrays
    char *files[argc];
    char *directories[argc];
    //if command line argument is a link, need to do directory listing on the link
    for(; optind < argc; optind++) {
        lstat(argv[optind], &current_stat); //maybe use lstat for symbolic links
        if(S_ISDIR(current_stat.st_mode)) { //if it is a directory
            directories[num_directories] = argv[optind];
            num_directories++;
        }
        else if(S_ISREG(current_stat.st_mode)) { //if it is a regular file
            files[num_files] = argv[optind];
            num_files++;
        }
        else if(S_ISLNK(current_stat.st_mode)){
            //determine if it is a directory or a file
            //use readlink to get the path of the link
            //use lstat to get the stat of the linked
            char linked_path[1024];
            ssize_t read_bytes;

            read_bytes = readlink(argv[optind], linked_path, sizeof(linked_path) - 1);

            linked_path[read_bytes] = '\0';
            struct stat linked_stat;
            if (lstat(linked_path, &linked_stat) == -1) {
                perror("lstat");
                exit(1);
            }

            // Determine if the target is a file or a directory
            if (S_ISDIR(linked_stat.st_mode)) {
                if(lFlag){ //treat it as a file, because ls -l causes this to be a file
                    files[num_files] = argv[optind];
                    num_files++;
                }
                else{
                    directories[num_directories] = argv[optind];
                    num_directories++;
                }
            }
            else{
                files[num_files] = argv[optind];
                num_files++;
            }
        }
        else{
            printf("Error : Nonexistent files or directories\n");
            exit(1);
        }
    }

    //sort the arrays
    qsort(files, num_files, sizeof(char *), alphabetical_string_sort);
    qsort(directories, num_directories, sizeof(char *), alphabetical_string_sort);

    //print the files
    for(int i = 0; i < num_files; i++) {
        lstat(files[i], &current_stat); //maybe use lstat for symbolic links
        printFileInfo(&current_stat);
        printFileName(files[i], ".", &current_stat);
    }
    if(num_files > 0 && num_directories > 0)
        printf("\n");

    //print the directories
    for(int i = 0; i < num_directories; i++) {
        if(rFlag){
            printDirectoryRecursive(directories[i]);
            printf("\n");
            continue;
        }
        printDirectory(directories[i]);
    }
}


void printDirectoryRecursive(char *inputDirectory) {
    DIR *dir = opendir(inputDirectory);

    if (dir == NULL) {
        printf("Error: Cannot open directory %s\n", inputDirectory);
        return;
    }

    struct dirent **items;
    int num_items = scandir(inputDirectory, &items, NULL, alphabetical_sort);

    printf("%s:\n", inputDirectory);

    struct stat current_stat;   //stat struct for current file
    char* directories[num_items];
    int num_recursive_directories = 0;

    for (int i = 0; i < num_items; i++) {
        // Skip . and ..
        if (strcmp(items[i]->d_name, ".") == 0 || strcmp(items[i]->d_name, "..") == 0) {
            free(items[i]);
            continue;
        }

        // Skip hidden files
        if (items[i]->d_name[0] == '.') {
            free(items[i]);
            continue;
        }

        // Create the full path
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", inputDirectory, items[i]->d_name);

        // Get file information using lstat
        if (lstat(full_path, &current_stat) == -1) {
            perror("lstat");
            free(items[i]);
            continue;
        }

        printFileInfo(&current_stat);
        printFileName(items[i]->d_name, inputDirectory, &current_stat);

        if (S_ISDIR(current_stat.st_mode)) {
            directories[num_recursive_directories] = strdup(full_path);
            num_recursive_directories++;
        }

        free(items[i]);
    }

    free(items);

    for(int i = 0; i < num_recursive_directories; i++) {
        printf("\n");
        printDirectoryRecursive(directories[i]);
        free(directories[i]);
    }

    closedir(dir);
}

void printDirectory(char *inputDirectory) {
    DIR *dir = opendir(inputDirectory);

    if(dir == NULL) {
        printf("Error : Nonexistent files or directories\n");
        exit(1);
    }

    struct dirent **items;
    int num_items = scandir(inputDirectory, &items, NULL, alphabetical_sort);

    if(num_directories > 1)
        printf("%s:\n", inputDirectory);
    struct stat current_stat;   //stat struct for current file
    for (int i = 0; i < num_items; i++) {

        //skip . and ..
        if(strcmp(items[i]->d_name, ".") == 0 || strcmp(items[i]->d_name, "..") == 0) {
            free(items[i]);
            continue;
        }

        //skip hidden files
        if(items[i]->d_name[0] == '.') {
            free(items[i]);
            continue;
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", inputDirectory, items[i]->d_name);


        lstat(full_path, &current_stat); //maybe use lstat for symbolic links

        printFileInfo(&current_stat);
        printFileName(items[i]->d_name, inputDirectory, &current_stat);

        free(items[i]);
    }

    if(num_directories > 1)
        printf("\n");

    // Free the allocated memory and close the directory
    free(items);
    closedir(dir);
}

void printFileInfo(struct stat *entry) {
    if(iFlag) {
        printf("%ld ", entry->st_ino);
    }
    if(lFlag) {
        printLongInfo(entry);
    }
}

void printFileName(char *filename, char* root,struct stat *file_stat) {

    if(S_ISLNK(file_stat->st_mode) && lFlag) {

        char *linkname = malloc(file_stat->st_size + 1);    //+1 for null terminator
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", root, filename);
        readlink(full_path, linkname, file_stat->st_size + 1);  //+1 for null terminator

        linkname[file_stat->st_size] = '\0';
        printf("%s -> %s\n", filename, linkname);
        free(linkname);
    }
    else {
        printf("%s\n", filename);
    }
}

void print_file_type(unsigned int st_mode) {
    if (S_ISREG(st_mode))      printf("-");
    else if (S_ISDIR(st_mode)) printf("d");
    else if (S_ISLNK(st_mode)) printf("l");
    else if (S_ISFIFO(st_mode)) printf("p");
    else if (S_ISBLK(st_mode)) printf("b");
    else if (S_ISSOCK(st_mode)) printf("s");
    else if (S_ISCHR(st_mode)) printf("c");

}

void print_permissions(unsigned int st_mode) {
    printf((st_mode & S_IRUSR) ? "r" : "-");
    printf((st_mode & S_IWUSR) ? "w" : "-");
    printf((st_mode & S_IXUSR) ? "x" : "-");

    printf((st_mode & S_IRGRP) ? "r" : "-");
    printf((st_mode & S_IWGRP) ? "w" : "-");
    printf((st_mode & S_IXGRP) ? "x" : "-");

    printf((st_mode & S_IROTH) ? "r" : "-");
    printf((st_mode & S_IWOTH) ? "w" : "-");
    printf((st_mode & S_IXOTH) ? "x" : "-");
}

void printLongInfo(struct stat *file_stat) {
    //file type
    print_file_type(file_stat->st_mode);

    //permissions
    print_permissions(file_stat->st_mode);

    // number of links
    printf(" %ld", file_stat->st_nlink);

    // owner name
    printf(" %s", getpwuid(file_stat->st_uid)->pw_name);

    //  group name
    printf(" %s", getgrgid(file_stat->st_gid)->gr_name);

    //  size, right-aligned with 6 spaces
    printf(" %6ld", file_stat->st_size);

    // time as month day year hour:minute
    char time[100];
    strftime(time, sizeof(time), "%b %e %Y %H:%M", localtime(&(file_stat->st_mtime)));
    printf(" %s ", time);
}