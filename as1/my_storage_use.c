#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    if(system("df -h > storage_output.txt") == -1){    //this will output the file system usage to a file called output.txt
        printf("Command Failed\n");
        return 1;
    }

    FILE* pipe = popen("df -h | awk '{sum+=$5} END {if (NR>1) print sum/(NR-1); else print 0}'", "r");  //use awk to find the average use %
    if (pipe == NULL) {
        printf("Command Failed\n");
        return 1;
    }

    char output[32];
    if (fgets(output, sizeof(output), pipe) == NULL) {  //read the output of the pipe
        printf("Failed to read output.\n");
        pclose(pipe);
        return 1;
    }

    if (pclose(pipe) != 0) {
        printf("Failed to close the pipe.\n");
        return 1;
    }   

    double averageUsePercent = atof(output);    //convert the output to a double and print
    printf("%.2f\n", averageUsePercent);

    return 0;

}