#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp;
    int i = 0;
    char hostnames[10][20];

    fp = fopen("NodeList", "r");

    for (i = 0; i < 10; i++) {
        fscanf(fp, "%s ", hostnames[i]);

        printf("%s\n", hostnames[i]);
    }

    fclose(fp);
    return 0;
}
