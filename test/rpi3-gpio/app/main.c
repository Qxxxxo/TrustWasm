
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

__attribute__((import_name("gpio_init"))) int
gpio_init(int pin, int func_code);

__attribute__((import_name("gpio_set"))) int
gpio_set(int pin, int val);

__attribute__((import_name("gpio_get"))) int
gpio_get(int pin);

__attribute__((import_name("test_gpio_overhead"))) int
test_gpio_overhead();

__attribute__((import_name("native_sleep"))) int 
native_sleep(int ms);


int main(int argc, char **argv)
{
    int res = 0;
    int x = 1, y = 2;
    printf("Hello World!\n");

    test_gpio_overhead();

    // set pin 22 output
    res = gpio_init(3, 1);
    // if (res)
    // {
    //     printf("Error set gpio output\n");
    //     return -1;
    // }
    int level=0;
    while (level<11)
    {
        res = gpio_set(3, level%2);
        if (res)
        {
            printf("Error set gpio\n");
            return -1;
        }
        native_sleep(1000);
        level+=1;
    }

    return 0;
}
