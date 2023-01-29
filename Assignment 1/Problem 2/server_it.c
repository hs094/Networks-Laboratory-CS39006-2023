#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
void update(double *ans, char op, double val)
{
    if (op == '+') *ans += val;
    if (op == '-') *ans -= val;
    if (op == '*') *ans *= val;
    if (op == '/') *ans /= val;
}
void clearBuffer(char *temp, int n)
{
    for (int i = 0; i < n; i++)
        temp[i] = '\0';
}
bool isOp(char st)
{
    return (st == '+' || st == '-' || st == '*' || st == '/');
}
// function to solve an expression entered as a string
double solve(char *str, int n)
{
    char *temp1 = (char *)malloc(100 * sizeof(char));
    char *temp3 = (char *)malloc(100 * sizeof(char));
    char *temp2 = (char *)malloc((n) * sizeof(char));
    char op = '*', op2 = '*';
    double ans = 1.0, inside = 1.0;
    int l = 0;
    for (int i = 0; i < n; i++)
    {
        if (isOp(str[i]))
        {
            if (temp1[0] != '\0')
                update(&ans, op, atof(temp1));
            clearBuffer(temp1, 25);
            l = 0;
            op = str[i];
        }
        else if (str[i] == '(')
        {
            int j = i;
            while (str[j] != ')') j++;
            for (int k = i + 1; k < j; k++) temp2[k - i - 1] = str[k];
            for (int k = 0; k < strlen(temp2); k++)
            {
                if(isOp(temp2[k]))
                {
                    double val = atof(temp3);
                    clearBuffer(temp3, 25);
                    update(&ans, op, val);
                    l = 0;
                    op2 = temp2[k];
                }
                else temp3[l++] = temp2[k];
            }
            double val = atof(temp3);
            update(&inside, op2, val);
            update(&ans, op, val);
            clearBuffer(temp2, n);
            inside = 1.0;
            op2 = '*';
            i = j;
        }
        else if (str[i] == ' ' || str[i] == ')') continue;
        else temp1[l++] = str[i];
    }
    if (temp1[0] != '\0')
    {
        double val = atof(temp1);
        update(&ans, op, val);
    }
    return ans;
}
int main()
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr, serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket\n");
        exit(0);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        perror("Unable to bind local address\n");
        exit(0);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                       &clilen);
    if (newsockfd < 0)
    {
        perror("Accept error\n");
        exit(0);
    }
    char *str = (char *)malloc(10 * sizeof(char));
    char *buf = (char *)malloc(10 * sizeof(char));
    while (1)
    {
        for (int i = 0; i < 10; i++)
            str[i] = '\0';
        for (int i = 0; i < 10; i++)
            buf[i] = '\0';
        int j = 0;
        int present_buffer_size = 10;
        int entered_size = 0;
        int f = 0;
        int cnt = 0;
        while (1)
        {
            int sz = recv(newsockfd, buf, 100, 0);
            entered_size += sz;
            int k = strlen(str);
            cnt++;
            if (entered_size > k)
                str = (char *)realloc(str, entered_size + k);
            strcpy(str + k, buf);
            int i;
            for (i = 0; i < sz; i++)
            {
                if (buf[i] == '\n')
                {
                    f = 1;
                    break;
                }
            }
            if (f == 1) break;
        }
        printf("[*] Recived Expression:- %s\n", str);
        printf("** Solving:- %s\n", str);
        double ans = solve(str, strlen(str));
        char *ans1 = (char *)malloc(100 * sizeof(char));
        gcvt(ans, 100, ans1);
        send(newsockfd, ans1, 100, 0);
        free(ans1);
    }
    close(newsockfd);
    return 0;
}
