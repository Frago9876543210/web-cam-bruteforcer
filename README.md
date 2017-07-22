# web-cam-bruteforcer

Simple and convenient software for hacking other people's cameras.

[Rewritten to C# for Windows](https://github.com/Frago9876543210/hikvision-bruteforcer)

### Warning
***So...***
***You will bear all responsibility. I do not call for these actions.***

### Installing
* Download image of kali linux and install.
* Write `git clone https://github.com/Frago9876543210/web-cam-bruteforcer.git` in terminal.
* Use `make` to build an application.

### Instructions for hacking cameras
* Install masscan if you do not have it. Use `sudo apt-get install masscan`.
* Get the ranges of your desired city or country.
* Use masscan to find open ports. Usually cameras from hikvision use `port 8000`.
* For example: `masscan -p8000 -iL ranges.txt --rate=1000 -oX scan.xml`.
* Next, using regular expressions, remove the excess from the file.
* Now paste these addresses into the `ips.txt` file.
* Also you can expand my dictionary, which is in the `dictionary.txt` file
* And now, use `./web-cam-bruteforcer` to run this application.
* Enter the port on which you were searching for the camera, and I recommend 300-450 threads.
