#include <iostream>
#include "mmwall_ctl.h"

int main(){

    mmwall_ctl rpi1("10.37.23.1", 1234);
    rpi1.send_angle("exit");
        
}
