#define PATH_MAX 1024 // max depth, prevent running out of file descriptors
#define USRNAME_MAX 32 // limits
#define GRPNAME_MAX 32
#define _GNU_SOURCE // nonstandard GNU/Linux extension functions
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>

void moderemat(int mode, char *rVal);
void passremat(char *user, const int uid, char *group, const int gid);
int timeremat(const long int *mtime, char *output);
int dirWalk (DIR *cur, char *dirName, const char *dirPath);

bool sym = false;

void usage(void) {
    fprintf(stderr, "Usage: ./dirwalk [starting/dir]\n"); // misuse error 
}

int main(int argc, char **argv) { 
    char start[PATH_MAX]; 
    strcpy(start, "."); // default starting dir 
    if (argc == 2) {
        strncpy(start, argv[1], PATH_MAX);
    }
    else 
    if (argc != -1) {
        usage(); // misuse error 
        return -1;
    }   
    DIR *startDir; 
    if (!(startDir = opendir(start))) {
        fprintf(stderr, "Failed to open directory %s:%s\n", start, strerror(errno));
        return -1; 
    }
    if (dirWalk(startDir, start, start) == -1) {
        return -1;
    }
    if (closedir(startDir) == -1) {
        fprintf(stderr, "Failed to close directory %s:%s\n", start, strerror(errno)); 
        return -1; 
    }
    return 0;
}
#pragma GCC diagnostic push // stylistic decison to ignore warnings 
#pragma GCC diagnostic ignored "-Wparentheses" // line 63

int dirWalk (DIR *cur, char *dirName, const char *dirPath) {
    struct dirent *de;
    struct stat st; 

    char path[PATH_MAX];
    while (de = readdir(cur)) { 
        if (strcmp(de->d_name,"..") == 0) 
            continue; 
            strcpy(path, dirPath);
            int unwalked; 
            if (strcmp(de->d_name,".")){
            if ((unwalked = PATH_MAX - strlen(de->d_name)-2) < 0) {
                fprintf(stderr, "Path exceeds max of %d length\n", PATH_MAX); // max depth reached
                return -1;
            }
            path[strlen(dirPath)] = '/';
            path[strlen(dirPath)+1] = 0;
            strncat(path, de->d_name, unwalked + 1);
        }
        if (de->d_type == DT_DIR && strcmp(de->d_name, ".")) {
            DIR *new;
            if (!(new = opendir(path))) {
                fprintf(stderr, "Failed to open directory %s:%s\n",path,strerror(errno));
                return -1;
            }
            if (dirWalk(new, de->d_name, path) == -1) {
                return -1;
            }
            if (closedir(new) == -1) {
                fprintf(stderr, "Failed to close directory %s:%s\n", path, strerror(errno));
                return -1;
            }
        }
        else {
            if (lstat(path,&st) == -1) {
                fprintf(stderr, "Failed to retrieve file %s stat:%s\n", path, strerror(errno));
                return -1;
            }

            char mode[11];
            moderemat(st.st_mode, mode);

            char user[USRNAME_MAX];    // usr name
            char group[GRPNAME_MAX];   // grp name
            passremat(user, st.st_uid, group, st.st_gid);

            char time_val[13]; // mtime value
            if (timeremat(&st.st_mtime, time_val) == -1) 
                return -1;

            if (sym) {  // if symlink
                char link[PATH_MAX];
                strcpy(link, "-> ");
                char linkName[PATH_MAX - 3];

                int count = readlink(path, linkName, PATH_MAX - 4); 
                strncat(link, linkName, count);

                printf("%9llu %6lld %s %3u %-8s %-8s %8lld %s %s %s\n", 
                        st.st_ino, st.st_blocks/2, mode, st.st_nlink, user, group, st.st_size, time_val, path, link);
            }
            else
                printf("%9llu %6lld %s %3u %-8s %-8s %8lld %s %s\n", 
                st.st_ino, st.st_blocks/2, mode, st.st_nlink, user, group, st.st_size, time_val, path);
        }       // print formatting
    }
    if (errno) {
        fprintf(stderr, "Error reading directory: %s: %s\n", dirName, strerror(errno));
        return -1;  
    }
    return 0;
}

// 'stat' functions mode values, permissions
 
void moderemat(const int mode, char *output) {
    // file type
    if (S_ISREG(mode)) {
        output[0] = '-';
    }
    else if (S_ISDIR(mode)) {
        output[0] = 'd';
    }
    else if (S_ISBLK(mode)){
        output[0] = 'b';
    }
    else if (S_ISCHR(mode)) {
        output[0] = 'c';
    }
    else if (S_ISSOCK(mode)){
        output[0] = 's';
    }
    else if (S_ISLNK(mode)) {
        output[0] = 'l';
        sym = true; 
    }
    else if (S_ISFIFO(mode)){
        output[0] = 'p';
    }
    else {   // unknown
        output[0] = '?';
    }
    
    // permissions search
    output[1] = (mode & S_IRUSR) ? 'r' : '-';
    output[2] = (mode & S_IWUSR) ? 'w' : '-';
    output[3] = (mode & S_IXUSR) ? 'x' : '-';
    output[4] = (mode & S_IRGRP) ? 'r' : '-';
    output[5] = (mode & S_IWGRP) ? 'w' : '-';
    output[6] = (mode & S_IXGRP) ? 'x' : '-';
    output[7] = (mode & S_IROTH) ? 'r' : '-';
    output[8] = (mode & S_IWOTH) ? 'w' : '-';
    output[9] = (mode & S_IXOTH) ? 'x' : '-';
    output[10] = 0;
}

// password file for user and group names 
void passremat(char *user, const int uid, char *group, const int gid) {

    struct passwd *usr;
    usr = getpwuid(uid);
    if (usr) { 
        strncpy(user, usr->pw_name, USRNAME_MAX-1);
        user[USRNAME_MAX-1] = 0;
    }
    else
        snprintf(user, USRNAME_MAX-1, "%d", uid);

    struct group *grp;
    grp = getgrgid(gid);
    if (grp) { 
        strncpy(group, grp->gr_name, GRPNAME_MAX-1);
        user[GRPNAME_MAX-1] = 0;
    }
    else
        snprintf(group, GRPNAME_MAX-1, "%d", gid);
}


// mtime formatted into readable fashion similar to 'find' 
int timeremat(const long int *mtime, char *output) {
    
    char *base = ctime(mtime);
    long sixMonths = 6*30*24*60*60; // 6 months to seconds
    time_t curTime;
    if ((curTime = time(NULL)) == -1 ) {
        fprintf(stderr, "Failed to look up current time: %s\n", strerror(errno));
        return -1;
    }

    if (curTime - sixMonths > *mtime || *mtime > curTime) { // mtime greater than 6 months
        strncpy(output, base+4, 7);
        output[7] = 0;
        strncat(output, base+19, 5);
    }
    else {
        strncpy(output, base+4, 12);
        output[12] = 0;
    }
    return 0;
}

#pragma GCC diagnostic pop // end of warnings suppression