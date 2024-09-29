// FIXME: PUT THIS IN A SEPARATE REPO

// This exists to waste time on a single thread
// For the memes
#include <stdint.h>
#include <iostream>
#include <chrono>
int main(){
    std::cout << "begin iteration" << std::endl;
    auto starttime = std::chrono::system_clock::now();
    for(uint32_t i = 0; i < (uint32_t)4'228'250'625; i++){
        asm("");
    }
    auto endtime = std::chrono::system_clock::now();
    std::chrono::duration<double> finaltime = endtime - starttime;
    std::cout << "endeded: " << finaltime.count() << 's' << std::endl;
}
/*
0.0.0.0/8
10.0.0.0/8
100.64.0.0/10
127.0.0.0/8
169.254.0.0/16
172.16.0.0/12
192.0.0.0/24
192.0.0.0/29
192.0.0.8/32
192.0.0.9/32
192.0.0.170/32
192.0.0.171/32
192.0.2.0/24
192.31.196.0/24
192.52.193.0/24
192.88.99.0/24
192.168.0.0/16
192.175.48.0/24
198.18.0.0/15
198.51.100.0/24
203.0.113.0/24
240.0.0.0/4
255.255.255.255/32

# Also to be considered, multicast addresses subnet:
224.0.0.0/4
*/