# MiniRedis

MiniRedis is a Redis-style key-value server that accepts TCP connections, parses a subset of the Redis Serialization Protocol (RESP), and executes commands against a thread-safe key-value store.

## Supported Commands

| Command  | Description                | Example          |
| -------- | -------------------------- | ---------------- |
| `SET`    | Store a value              | `SET name alice` |
| `GET`    | Retrieve a value           | `GET name`       |
| `REMOVE` | Delete a key               | `REMOVE name`    |
| `EXISTS` | Check whether a key exists | `EXISTS name`    |

## Prerequisites

* Linux (can use WSL if on Windows)
* C++23 compiler (GCC 14+ or Clang 17+)
* CMake 3.20+

## Quick Start

Build and start the server:

```bash
git clone https://github.com/sameersaeed/MiniRedis.git
cd MiniRedis

cmake -B build
cmake --build build -j$(nproc)

./build/miniredis
```

Sample output:
```
[server] listening on port 9080
[client] setup successful

[request] *2\r\n$3\r\nGET\r\n$4\r\ntest\r\n'
[response] 404 Not Found


[request] *3\r\n$3\r\nSET\r\n$4\r\ntest\r\n$3\r\n321\r\n'
[response] 200 OK
321


[request] *2\r\n$3\r\nGET\r\n$4\r\ntest\r\n'
[response] 200 OK
321
```

## RESP

MiniRedis uses the Redis Serialization Protocol (RESP) for client communication.

For example, a `SET test 321` request is encoded as:

```bash
*3\r\n
$3\r\n
SET\r\n
$4\r\n
test\r\n
$3\r\n
321\r\n
```

The parser handles incomplete messages across multiple TCP reads rather than assuming one `recv()` call contains one complete command.

## Testing

Tests are written with GoogleTest and cover the storage, RESP parser, command executor, and end-to-end server behavior.

The test suite covers:

* Key insertion, lookup, deletion, and existence checks
* Missing keys and overwriting existing values
* Valid and malformed RESP commands
* Partial RESP messages across multiple reads
* Command execution and response generation
* End-to-end TCP client/server communication

You can run the tests with:

```bash
cmake --build build -j$(nproc) --target miniredis_tests
cd build
ctest --output-on-failure
```

Sample output:
```
[==========] Running 25 tests from 4 test suites.
[----------] 7 tests from StorageTest
[----------] 9 tests from ParserTest
[----------] 7 tests from ExecutorTest
[----------] 2 tests from IntegrationTest
[  PASSED  ] 25 tests.

100% tests passed, 0 tests failed out of 25
Total Test time (real) =   0.26 sec
```