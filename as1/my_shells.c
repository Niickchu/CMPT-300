#include <stdlib.h>

int main() {
    system("grep -v '^#' /etc/shells | grep -oE '[^/]+$' | sort -u");
    //here were just using grep to find the shells in /etc/shells while ignoring the comments
    //then we pipe the output to grep again to read the non "/" characters at the end of the line
    //then we pipe the output to sort -u to sort the output and remove duplicates
    return 0;
}