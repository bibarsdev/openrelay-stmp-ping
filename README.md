# Summary
A program that pings OpenRelay STMP servers.

# How to build
**Note:** Currently, This project only supports Windows. Any commit that contributes to adding multi-platfrom support would be appreciated.

1.Check out the source code:
```sh
git clone https://github.com/bibarsdev/openrelay-stmp-ping.git /path/to/source
cd /path/to/source
```
2.Create the output directory, then change into it:
```sh
mkdir build
cd build
```
3a.Run CMake with the desired generator. For this example we're generating Visual Studio 16 2019 project files:
```sh
cmake -G"Visual Studio 16 2019" /path/to/source
```
3b.If you want to build with Ninja:
```sh
cmake -GNinja /path/to/source
ninja
```

# How to use
Run "ping-smtp" with a list of subdomains or ip addresses as arguments:
```sh
ping-stmp smtp.gmail.com www.youtube.com ...
```
Specify 'print' to print sent and received packets:
```sh
ping-stmp print smtp.gmail.com www.youtube.com ...
```

# How does it work
First it sends a 'HELO' packet to the server, if it replies that means it is an SMTP server. Then it tries to send an email, if the server doesn't proceed then it is not openrelay. However, If it does proceed, that means it is an openrelay SMTP server.

# TODO
- Add multi-platform support.
