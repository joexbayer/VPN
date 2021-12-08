# VPN

### VPN that tunnels IP (default = all traffic) to specified VPN server.

## Usage:

### Client (Only MacOS / Linux support)

For forwarding all traffic to VPN
```bash
make client
```
or choose route yourself
```bash
make build-client
./client.out <route> <ip of vpn server>
```

### Server (Only Linux support)
```bash
make build-server
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