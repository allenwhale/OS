#include <stdio.h>
#include <time.h>
#include <bits/stdc++.h>
#include <unistd.h>
int main(){
    int cnt=0;
    while(1){
        printf("%d\n", cnt++);
        sleep(1);
        if(cnt==5)break;
    }
    return 0;
}
