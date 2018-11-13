#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 4096



int main(){
    int arr_size = 101*PAGE_SIZE;
    int offset = 0;
    int arr[arr_size];
    int count = 0;
    while(count < 10000){
        arr[offset] = count;
        count += 1;
        offset = (offset+PAGE_SIZE)%arr_size;
    }
}
