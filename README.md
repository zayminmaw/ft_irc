_This project has been created as part of the 42 curriculum by zmin, wmin-kha._

# Description

`ft_irc` is a from-scratch implementation of an Internet Relay Chat (IRC) server written in C++98. The goal is to build a server that real IRC clients (e.g. `irssi`, `HexChat`, or even plain `nc`) can connect to and use to chat in real time, following the IRC protocol defined in RFC 1459.

The server handles multiple clients simultaneously over TCP using non-blocking I/O multiplexed through a single `poll()` loop — no forking, no threading, and no busy-waiting. It supports user authentication, nickname and user registration, channel creation and management, private and channel messaging, and the standard operator-only channel controls.

Supported features include:

- **Connection registration:** `PASS`, `NICK`, `USER`
- **Messaging:** `PRIVMSG` (to users and channels)
- **Channels:** `JOIN`, `PART`, `TOPIC`, `NAMES`, `QUIT`
- **Operator commands:** `KICK`, `INVITE`, `TOPIC`, `MODE`
- **Channel modes:**
  - `i` — invite-only
  - `t` — topic locked to operators
  - `k` — channel key (password)
  - `l` — user limit
  - `o` — operator privilege
- **Server housekeeping:** `PING`/`PONG`, graceful and abrupt disconnects, broadcast of `QUIT` to all shared channels.

# Instructions

### Requirements

- A C++98-compatible compiler (`c++` / `clang++` / `g++`)
- POSIX environment (tested on macOS and Linux)

### Compilation

```sh
make           # builds ./ircserv
make clean     # removes object files
make fclean    # removes object files and the binary
make re        # fclean + all
```

### Running the server

```sh
./ircserv <port> <password>
```

- `<port>` — TCP port to listen on (e.g. `6667`)
- `<password>` — connection password clients must send via `PASS`

Example:

```sh
./ircserv 6667 testpass
```

### Connecting with `nc`

```sh
nc 127.0.0.1 6667
```

Then register and join a channel:

```
PASS testpass
NICK alice
USER alice 0 * :Alice
JOIN #chat
PRIVMSG #chat :hello world
```

### Connecting with an IRC client

Point any IRC client at `127.0.0.1`, port `6667`, server password `testpass`. The server identifies itself as `ircserv`.

# Command Reference & Usage Examples

All examples below assume the server is running on `127.0.0.1:6667` with password `testpass`, and that you are already connected via `nc 127.0.0.1 6667`.

### Registration

Every client must register before sending any other command.

```
PASS testpass
NICK alice
USER alice 0 * :Alice Liddell
```

On success the server replies with the welcome burst:

```
:ircserv 001 alice :Welcome to the Internet Relay Network alice
:ircserv 002 alice :Your host is ircserv, running version 1.0
:ircserv 003 alice :This server was created May 2026
:ircserv 004 alice ircserv 1.0 o itkol
```

### PING / PONG

Used by clients to keep the connection alive.

```
PING :hello
```
Reply:
```
:ircserv PONG ircserv :hello
```

### JOIN — enter a channel

```
JOIN #chat
JOIN #secret mypass        # join a +k channel using its key
```

The first user to join a channel automatically becomes its operator (`@`).

### PART — leave a channel

```
PART #chat
PART #chat :see you later
```

### PRIVMSG — send a message

To a channel:
```
PRIVMSG #chat :hello everyone
```
To a user:
```
PRIVMSG bob :hi bob
```

### TOPIC — view or set a channel topic

```
TOPIC #chat                       # query current topic
TOPIC #chat :Welcome to #chat     # set topic (op-only if +t is set)
```

### NAMES — list members of a channel

```
NAMES #chat
```
Reply:
```
:ircserv 353 alice = #chat :@alice bob
:ircserv 366 alice #chat :End of /NAMES list
```

### KICK — remove a user from a channel (operator-only)

```
KICK #chat bob
KICK #chat bob :stop spamming
```

### INVITE — invite a user to a channel (operator-only when channel is +i)

```
INVITE bob #chat
```

### MODE — view or change channel modes (operator-only to change)

Query current modes:
```
MODE #chat
```
Reply:
```
:ircserv 324 alice #chat +itk secret
```

Set / unset modes:

| Mode | Set example                  | Unset example          | Effect                                |
|------|------------------------------|------------------------|---------------------------------------|
| `i`  | `MODE #chat +i`              | `MODE #chat -i`        | Invite-only channel                   |
| `t`  | `MODE #chat +t`              | `MODE #chat -t`        | Only operators may change the topic   |
| `k`  | `MODE #chat +k secret`       | `MODE #chat -k`        | Set/remove a channel key (password)   |
| `l`  | `MODE #chat +l 10`           | `MODE #chat -l`        | Set/remove a user-count limit         |
| `o`  | `MODE #chat +o bob`          | `MODE #chat -o bob`    | Grant/revoke operator status          |

Combined modes also work:
```
MODE #chat +it
MODE #chat -i-t
```

### QUIT — disconnect from the server

```
QUIT :gotta go
```

The server broadcasts `:alice!alice@127.0.0.1 QUIT :Quit: gotta go` to every channel the user was in, then closes the connection.

# Resources

### Reference material

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459) — the canonical IRC specification; the source of truth for command syntax and numeric replies.
- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812) — clarifies a few behaviors left ambiguous in 1459.
- [Modern IRC Documentation (ircdocs.horse)](https://modern.ircdocs.horse/) — a community-maintained, more readable rendering of the protocol.
- [Small IRC Server (ft_irc 42 Network) — A. Fatir](https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9) — overview article useful for project orientation.
- `man 2 poll`, `man 2 socket`, `man 2 accept`, `man 2 send`, `man 2 recv` — POSIX socket and multiplexing primitives.

### Use of AI

AI assistance was used for two purposes:

- **RFC interpretation** — to clarify the expected behavior and numeric reply codes (e.g. `324`, `353`, `366`, `433`, `461`, `464`, `473`, `475`, `482`) when reading RFC 1459/2812 was ambiguous, and to confirm the precise wire format of certain server-to-client messages such as `JOIN`, `PART`, `QUIT`, and `MODE` echoes.
- **Project planning and task division** — to help break the project down into a networking layer (socket setup, `poll()` loop, accept/recv/send, client lifecycle) and a protocol layer (command parsing, channel logic, IRC replies), and to split the work cleanly between the two teammates so that the two layers could be developed in parallel against a stable interface.
