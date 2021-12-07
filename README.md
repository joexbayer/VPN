# VPN

### VPN that tunnels IP (default = all traffic) to specified VPN server.

## Usage:

### Client (Currently only MacOS support)
```bash
make client
./client.out <route> <ip of vpn server>
```

### Server Currently only Linux support
```bash
make server
./server.out
```

## Needs OPENSSL for encryption.
### Linux
```bash
apt-get install libssl-dev
```
### MacOS (Brew)
```bash
brew update
brew install openssl
```